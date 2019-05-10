#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>
#include <shlwapi.h>
#include <sqlext.h>
#include <git2.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "tds.h"
#include "git.h"

using namespace std;

const string db_app = "GitSQL";

static void replace_all(string& source, const string& from, const string& to);

// strip out characters that NTFS doesn't like
static string sanitize_fn(const string& fn) {
	string s;

	for (auto c : fn) {
		if (c != '<' && c != '>' && c != ':' && c != '"' && c != '/' && c != '\\' && c != '|' && c != '?' && c != '*')
			s += c;
	}

	return s;
}

static void clear_dir(const string& dir, bool top) {
	HANDLE h;
	WIN32_FIND_DATAA fff;

	h = FindFirstFileA((dir + "*").c_str(), &fff);

	if (h == INVALID_HANDLE_VALUE)
		return;

	do {
		if (!strcmp(fff.cFileName, ".") || !strcmp(fff.cFileName, ".."))
			continue;

		if (!top || fff.cFileName[0] != '.') {
			if (fff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				clear_dir(dir + fff.cFileName + "\\", false);
				RemoveDirectoryA((dir + fff.cFileName).c_str());
			} else
				DeleteFileA((dir + fff.cFileName).c_str());
		}
	} while (FindNextFileA(h, &fff));

	FindClose(h);
}

static string sql_escape(string s) {
	replace_all(s, "'", "''");

	return s;
}

static string dump_table(const TDSConn& tds, const string& schema, const string& table) {
	string obj = "[" + schema + "].[" + table + "]"; // FIXME - do we need to escape?
	string cols, s;

	TDSQuery sq(tds, "SELECT * FROM " + obj);

	while (sq.fetch_row()) {
		string vals;
		auto column_count = sq.column_count();

		if (cols == "") {
			for (unsigned int i = 0; i < column_count; i++) {
				const auto& col = sq[i];

				if (cols != "")
					cols += ", ";

				cols += "[" + col.name + "]";
			}
		}

		s += "INSERT INTO " + obj + "(" + cols + ") VALUES(";

		for (unsigned int i = 0; i < column_count; i++) {
			const auto& col = sq[i];

			if (vals != "")
				vals += ", ";

			if (col.is_null())
				vals += "NULL";
			else {
				switch (col.type) {
					case SYBINTN:
					case SYBINT1:
					case SYBINT2:
					case SYBINT4:
					case SYBINT8:
					case SYBBITN:
					case SYBBIT:
						vals += to_string((signed long long)col);
					break;

					case SYBFLT8:
					case SYBFLTN:
						vals += to_string((double)col);
					break;

					default:
						vals += "'" + sql_escape(string(col)) + "'";
					break;
				}
			}
		}

		s += vals;

		s += ");\n";
	}

	return s;
}

struct sql_obj {
	sql_obj(const string& schema, const string& name, const string& def, const string& type, bool fulldump) : schema(schema), name(name), def(def), type(type), fulldump(fulldump) { }

	string schema, name, def, type;
	bool fulldump;
};

static void dump_sql(const TDSConn& tds, const string& repo_dir, const string& db) {
	string s, dbs;

	if (db != "")
		dbs = db + ".";

	clear_dir(repo_dir, true);

	vector<sql_obj> objs;

	{
		TDSQuery sq(tds, "SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type), extended_properties.value FROM " + dbs + "sys.objects LEFT JOIN " + dbs + "sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN " + dbs + "sys.schemas ON schemas.schema_id=objects.schema_id LEFT JOIN " + dbs + "sys.extended_properties ON extended_properties.major_id=objects.object_id AND extended_properties.minor_id=0 AND extended_properties.class=1 AND extended_properties.name='fulldump' WHERE objects.type='V' OR objects.type='P' OR objects.type='FN' OR objects.type='U' ORDER BY schemas.name, objects.name");

		while (sq.fetch_row()) {
			objs.emplace_back(sq[0], sq[1], sq[2], sq[3], (string)sq[4] == "true");
		}
	}

	for (auto& obj : objs) {
		string dir = repo_dir + sanitize_fn(obj.schema);
		string subdir;
		string fn;
		HANDLE h;
		DWORD written;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw runtime_error("CreateDirectory for " + dir + " returned error " + to_string(last_error));
		}

		if (obj.type == "V")
			subdir = "views";
		else if (obj.type == "P")
			subdir = "procedures";
		else if (obj.type == "FN")
			subdir = "functions";
		else if (obj.type == "U")
			subdir = "tables";

		if (obj.type == "U") {
			{
				TDSQuery sq2(tds, "SELECT " + dbs + "dbo.func_GetDDL(?)",  obj.schema + "." + obj.name);

				if (!sq2.fetch_row())
					throw runtime_error("Error calling dbo.func_GetDDL for " + dbs + obj.schema + "." + obj.name + ".");

				obj.def = sq2[0];
			}

			if (obj.fulldump)
				obj.def += "\n" + dump_table(tds, obj.schema, obj.name);
		}

		replace_all(obj.def, "\r\n", "\n");

		obj.def += "\n";

		dir += "\\" + subdir;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw runtime_error("CreateDirectory for " + dir + " returned error " + to_string(last_error));
		}

		fn = dir + "\\" + sanitize_fn(obj.name) + ".sql";

		h = CreateFileA(fn.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (h == INVALID_HANDLE_VALUE)
			throw runtime_error("CreateFile returned error " + to_string(GetLastError()));

		if (!WriteFile(h, obj.def.c_str(), static_cast<DWORD>(obj.def.size()), &written, NULL))
			throw runtime_error("WriteFile returned error " + to_string(GetLastError()));

		SetEndOfFile(h);

		CloseHandle(h);
	}
}

static void git_add_dir(GitIndex& index, const string& dir, const string& unixpath) {
	HANDLE h;
	WIN32_FIND_DATAA fff;

	h = FindFirstFileA((dir + "*").c_str(), &fff);

	if (h == INVALID_HANDLE_VALUE)
		return;

	do {
		if (fff.cFileName[0] != '.') {
			if (fff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				git_add_dir(index, dir + fff.cFileName + "\\", unixpath + fff.cFileName + "/");
			else
				index.add_bypath(unixpath + fff.cFileName);
		}
	} while (FindNextFileA(h, &fff));

	FindClose(h);
}

static void git_remove_dir(GitRepo& repo, GitTree& tree, const string& dir, const string& unixdir, vector<string>& deleted) {
	size_t c = tree.entrycount();

	for (size_t i = 0; i < c; i++) {
		GitTreeEntry gte(tree, i);
		string name = gte.name();

		if (gte.type() == GIT_OBJ_TREE) {
			GitTree subtree(repo, gte);

			git_remove_dir(repo, subtree, dir + name + "\\", unixdir + name + "/", deleted);
		}

		if (!PathFileExistsA((dir + name).c_str()))
			deleted.push_back(unixdir + name);
	}
}

// taken from Stack Overflow: https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
static void replace_all(string& source, const string& from, const string& to) {
	string new_string;
	new_string.reserve(source.length());

	string::size_type last_pos = 0;
	string::size_type find_pos;

	while((find_pos = source.find(from, last_pos)) != string::npos) {
		new_string.append(source, last_pos, find_pos - last_pos);
		new_string += to;
		last_pos = find_pos + from.length();
	}

	new_string += source.substr(last_pos);

	source.swap(new_string);
}

static void do_update_git(const string& repo_dir) {
	git_oid tree_id;
	git_commit* parent;
	git_oid parent_id;
	vector<string> deleted;

	git_libgit2_init();

	GitRepo repo(repo_dir);

	GitSignature sig("System", "business.intelligence@boltonft.nhs.uk");

	repo.reference_name_to_id(&parent_id, "HEAD");

	repo.commit_lookup(&parent, &parent_id);

	GitTree parent_tree(parent);

	{
		GitIndex index(repo);

		index.clear();

		// loop through saved files and add
		git_add_dir(index, repo_dir, "");

		// loop through repo and remove anything that's been deleted
		git_remove_dir(repo, parent_tree, repo_dir, "", deleted);

		for (const auto& d : deleted) {
			index.remove_bypath(d);
		}

		index.write_tree(&tree_id);
	}

	GitTree tree(repo, tree_id);

	{
		GitDiff diff(repo, parent_tree, tree, nullptr);

		if (diff.num_deltas() == 0) // no changes - avoid doing empty commit
			return;
	}

	repo.commit_create("HEAD", sig, sig, "Update", tree, parent);

	// FIXME - push to remote server?
}

static void flush_git(const TDSConn& tds) {
	map<unsigned int, string> repos;

	git_libgit2_init();

	{
		TDSQuery sq(tds, "SELECT Git.repo, GitRepo.dir FROM (SELECT repo FROM Restricted.Git GROUP BY repo) Git JOIN Restricted.GitRepo ON GitRepo.id=Git.repo");

		while (sq.fetch_row()) {
			repos[(unsigned int)sq[0]] = sq[1];
		}

		if (repos.size() == 0)
			return;
	}

	for (const auto& r : repos) {
		GitRepo repo(r.second);

		while (true) {
			TDSTrans trans(tds);
			unsigned int commit, dt;
			int tz_offset;
			string name, email, description;
			nullable<unsigned long long> transaction;

			{
				TDSQuery sq(tds, "SELECT TOP 1 Git.id, COALESCE(LDAP.givenName+' '+LDAP.sn, LDAP.name, Git.username), COALESCE(LDAP.mail,'business.intelligence@boltonft.nhs.uk'), Git.description, DATEDIFF(SECOND,'19700101',Git.dt), Git.offset, Git.tran_id FROM Restricted.Git LEFT JOIN AD.ldap ON LEFT(Git.username,5)='XRBH\\' AND ldap.sAMAccountName=RIGHT(Git.username,CASE WHEN LEN(Git.username)>=5 THEN LEN(Git.username)-5 END) WHERE Git.repo=? ORDER BY Git.id", r.first);

				if (!sq.fetch_row())
					break;

				commit = (unsigned int)sq[0];
				name = sq[1];
				email = sq[2];
				description = sq[3];
				dt = (unsigned int)sq[4];
				tz_offset = (int)sq[5];
				transaction = sq[6];
			}

			list<git_file> files;

			{
				TDSQuery sq2(tds, "SELECT filename, data FROM Restricted.GitFiles WHERE id=?", commit);

				while (sq2.fetch_row()) {
					files.emplace_back(sq2[0], sq2[1]);
				}
			}

			// merge together entries with the same transaction ID
			if (!transaction.is_null()) {
				bool first = true;

				while (true) {
					unsigned int commit2;

					{
						TDSQuery sq(tds, "SELECT TOP 1 Git.id, Git.tran_id FROM Restricted.Git WHERE repo=? AND id!=? ORDER BY id", r.first, commit);

						if (!sq.fetch_row())
							break;

						if (sq[1].is_null())
							break;

						if ((signed long long)sq[1] != (signed long long)transaction)
							break;

						commit2 = (unsigned int)sq[0];
					}

					if (first)
						description += " (transaction)";

					{
						TDSQuery sq2(tds, "SELECT filename, data FROM Restricted.GitFiles WHERE id=?", commit2);

						while (sq2.fetch_row()) {
							string fn = sq2[0];
							
							for (auto it = files.begin(); it != files.end(); it++) {
								if (it->filename == fn) {
									files.erase(it);
									break;
								}
							}

							files.emplace_back(fn, sq2[1]);
						}
					}

					{ TDSQuery(tds, "UPDATE Restricted.GitFiles SET id=? WHERE id=?", commit, commit2); }
					{ TDSQuery(tds, "DELETE FROM Restricted.Git WHERE id=?", commit2); }

					first = false;
				}
			}

			if (files.size() > 0)
				update_git(repo, name, email, description, dt, tz_offset, files);

			{ TDSQuery(tds, "DELETE FROM Restricted.Git WHERE id=?", commit); }
			{ TDSQuery(tds, "DELETE FROM Restricted.GitFiles WHERE id=?", commit); }

			trans.commit();
		}
	}
}

class lockfile {
public:
	lockfile() {
		OVERLAPPED ol;

		h = CreateFileA("lockfile", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (h == INVALID_HANDLE_VALUE)
			throw runtime_error("Could not create or open lockfile.");

		if (GetLastError() == 0) { // file created
			DWORD written;

			WriteFile(h, "lockfile", 8, &written, nullptr);
		}

		memset(&ol, 0, sizeof(ol));

		LockFileEx(h, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ol);
	}

	~lockfile() {
		if (h != INVALID_HANDLE_VALUE) {
			OVERLAPPED ol;

			memset(&ol, 0, sizeof(ol));
			UnlockFileEx(h, 0, 1, 0, &ol);

			CloseHandle(h);
		}
	}

private:
	HANDLE h = INVALID_HANDLE_VALUE;
};

static int tds_message_handler(const TDSCONTEXT*, TDSSOCKET*, TDSMESSAGE* msg) {
	if (msg->severity > 10)
		throw runtime_error(msg->message);

	return 0;
}

int main(int argc, char** argv) {  
	string cmd;

	if (argc < 4)
		return 1;

	string db_server = argv[1];
	string db_username = argv[2];
	string db_password = argv[3];

	if (argc > 4)
		cmd = argv[4];

	if (cmd != "flush" && argc < 6)
		return 1;

	TDSConn tds(db_server, db_username, db_password, db_app, tds_message_handler, tds_message_handler);

	try {
		string unixpath, def;

		{
			lockfile lf;

			if (cmd == "flush")
				flush_git(tds);
			else {
				string repo_dir = cmd;
				string db = argv[5];

				dump_sql(tds, repo_dir, db);
				do_update_git(repo_dir);
			}
		}
	} catch (const exception& e) {
		TDSQuery sq(tds, "INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", e.what());
		//MessageBoxA(0, s, "Error", MB_ICONERROR);
		throw;
	} catch (...) {
		TDSQuery sq(tds, "INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", "Unrecognized exception.");
		//MessageBoxA(0, "Unrecognized exception.", "Error", MB_ICONERROR);
		throw;
	}

	return 0;
}
