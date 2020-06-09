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
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tdscpp.h>
#include "git.h"

using namespace std;

const string db_app = "GitSQL";

void replace_all(string& source, const string& from, const string& to);

// table.cpp
string table_ddl(const tds::Conn& tds, const string_view& schema, const string_view& table);

// strip out characters that NTFS doesn't like
static string sanitize_fn(const string& fn) {
	string s;

	for (auto c : fn) {
		if (c != '<' && c != '>' && c != ':' && c != '"' && c != '/' && c != '\\' && c != '|' && c != '?' && c != '*')
			s += c;
	}

	return s;
}

static void clear_dir(const filesystem::path& dir) {
	vector<filesystem::path> children;

	for (const auto& p : filesystem::directory_iterator(dir)) {
		children.emplace_back(p);
	}

	for (const auto& p : children) {
		if (p.filename().u8string()[0] != '.')
			filesystem::remove_all(p);
	}
}

struct sql_obj {
	sql_obj(const string& schema, const string& name, const string& def, const string& type) : schema(schema), name(name), def(def), type(type) { }

	string schema, name, def, type;
};

static void dump_sql(const tds::Conn& tds, const filesystem::path& repo_dir, const string& db) {
	string s, dbs;

	if (db != "")
		dbs = db + ".";

	clear_dir(repo_dir);

	vector<sql_obj> objs;

	{
		tds::Query sq(tds, "SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type) FROM " + dbs + "sys.objects LEFT JOIN " + dbs + "sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN " + dbs + "sys.schemas ON schemas.schema_id=objects.schema_id WHERE objects.type IN ('V','P','FN','TF','IF','U') ORDER BY schemas.name, objects.name");

		while (sq.fetch_row()) {
			objs.emplace_back(sq[0], sq[1], sq[2], sq[3]);
		}
	}

	{
		tds::Query sq(tds, "SELECT triggers.name, sql_modules.definition FROM " + dbs + "sys.triggers LEFT JOIN " + dbs + "sys.sql_modules ON sql_modules.object_id=triggers.object_id WHERE triggers.parent_class_desc = 'DATABASE'");

		while (sq.fetch_row()) {
			objs.emplace_back("db_triggers", sq[0], sq[1], "");
		}
	}

	for (auto& obj : objs) {
		filesystem::path dir = repo_dir / sanitize_fn(obj.schema);

		if (!filesystem::exists(dir) && !filesystem::create_directory(dir))
			throw runtime_error("Error creating directory " + dir.u8string() + ".");

		string subdir;

		if (obj.type == "V")
			subdir = "views";
		else if (obj.type == "P")
			subdir = "procedures";
		else if (obj.type == "FN" || obj.type == "TF" || obj.type == "IF")
			subdir = "functions";
		else if (obj.type == "U")
			subdir = "tables";
		else
			subdir = "";

		if (obj.type == "U")
			obj.def = table_ddl(tds, obj.schema, obj.name);

		replace_all(obj.def, "\r\n", "\n");

		if (!obj.def.empty() && obj.def[0] == '\n') {
			auto pos = obj.def.find_first_not_of("\n");

			if (pos == string::npos)
				obj.def.clear();
			else
				obj.def = obj.def.substr(pos);
		}

		while (!obj.def.empty() && obj.def.back() == '\n') {
			obj.def.pop_back();
		}

		obj.def += "\n";

		if (subdir != "") {
			dir /= subdir;

			if (!filesystem::exists(dir) && !filesystem::create_directory(dir))
				throw runtime_error("Error creating directory " + dir.u8string() + ".");
		}

		filesystem::path fn = dir / (sanitize_fn(obj.name) + ".sql");

		ofstream f(fn, ios::binary);

		if (!f.good())
			throw runtime_error("Error creating file " + fn.u8string() + ".");

		f.write(obj.def.c_str(), obj.def.size());
	}
}

static void git_add_dir(list<git_file>& files, const filesystem::path& dir, const string& unixpath) {
	for (const auto& de : filesystem::directory_iterator(dir)) {
		const auto& p = de.path();
		auto name = p.filename().u8string();

		if (!name.empty() && name[0] != '.') {
			if (de.is_directory())
				git_add_dir(files, p, unixpath + name + "/");
			else {
				string data;

				{
					ifstream f(p, ios::binary);

					if (!f.good())
						throw runtime_error("Could not open " + p.u8string() + " for reading.");

					f.seekg(0, ios::end);
					size_t size = f.tellg();
					data = string(size, ' ');
					f.seekg(0, ios::beg);

					f.read(&data[0], size); 
				}

				files.emplace_back(unixpath + name, data);
			}
		}
	}
}

static void git_remove_dir(GitRepo& repo, GitTree& tree, const filesystem::path& dir, const string& unixdir, list<git_file>& files) {
	size_t c = tree.entrycount();

	for (size_t i = 0; i < c; i++) {
		GitTreeEntry gte(tree, i);
		string name = gte.name();

		if (gte.type() == GIT_OBJ_TREE) {
			GitTree subtree(repo, gte);

			git_remove_dir(repo, subtree, dir / name, unixdir + name + "/", files);
		} else if (!filesystem::exists(dir / name))
			files.emplace_back(unixdir + name, nullopt);
	}
}

// taken from Stack Overflow: https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
void replace_all(string& source, const string& from, const string& to) {
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
	list<git_file> files;
	git_oid parent_id;

	git_libgit2_init();

	GitRepo repo(repo_dir);

	git_add_dir(files, repo_dir, "");

	if (repo.reference_name_to_id(&parent_id, "HEAD")) {
		git_commit* parent;

		repo.commit_lookup(&parent, &parent_id);

		GitTree parent_tree(parent);

		// loop through repo and remove anything that's been deleted
		git_remove_dir(repo, parent_tree, repo_dir, "", files);
	}

	update_git(repo, "System", "business.intelligence@boltonft.nhs.uk", "Update", files);

	// FIXME - push to remote server?
}

static void flush_git(const tds::Conn& tds) {
	map<unsigned int, string> repos;

	git_libgit2_init();

	tds.run("DELETE FROM Restricted.Git WHERE (SELECT COUNT(*) FROM Restricted.GitFiles WHERE id = Git.id) = 0");

	{
		tds::Query sq(tds, "SET LOCK_TIMEOUT 0; SET XACT_ABORT ON; SELECT Git.repo, GitRepo.dir FROM (SELECT repo FROM Restricted.Git GROUP BY repo) Git JOIN Restricted.GitRepo ON GitRepo.id=Git.repo");

		while (sq.fetch_row()) {
			repos[(unsigned int)sq[0]] = sq[1];
		}

		if (repos.size() == 0)
			return;
	}

	for (const auto& r : repos) {
		GitRepo repo(r.second);

		while (true) {
			tds::Trans trans(tds);
			unsigned int commit, dt;
			int tz_offset;
			string name, email, description;
			list<git_file> files;
			bool clear_all = false;
			list<unsigned int> delete_commits;
			list<unsigned int> delete_files;
			bool merged_trans = false;

			{
				tds::Query sq(tds, R"(
SET LOCK_TIMEOUT 0;
SET XACT_ABORT ON;

SELECT
	Git.id,
	COALESCE([user].givenName+' '+[user].sn, [user].name, Git.username),
	COALESCE([user].mail,'business.intelligence@boltonft.nhs.uk'),
	Git.description,
	DATEDIFF(SECOND,'19700101',Git.dt),
	Git.offset,
	GitFiles.file_id,
	GitFiles.filename,
	GitFiles.data
FROM Restricted.Git
LEFT JOIN AD.[user] ON LEFT(Git.username,5) = 'XRBH\' AND [user].sAMAccountName = RIGHT(Git.username, CASE WHEN LEN(Git.username) >= 5 THEN LEN(Git.username) - 5 END)
JOIN Restricted.GitFiles ON GitFiles.id = Git.id
WHERE Git.id = (SELECT MIN(id) FROM Restricted.Git WHERE repo = ?) OR Git.tran_id = (SELECT tran_id FROM Restricted.Git WHERE id = (SELECT MIN(id) FROM Restricted.Git WHERE repo = ?))
ORDER BY Git.id
)", r.first, r.first);

				if (!sq.fetch_row())
					break;

				commit = (unsigned int)sq[0];
				name = sq[1];
				email = sq[2];
				description = sq[3];
				dt = (unsigned int)sq[4];
				tz_offset = (int)sq[5];

				delete_commits.push_back(commit);

				do {
					delete_files.push_back((unsigned int)sq[6]);

					if ((unsigned int)sq[0] != commit) {
						if (!merged_trans) {
							description += " (transaction)";
							merged_trans = true;
						}

						bool found = false;
						auto new_commit = (unsigned int)sq[0];

						for (auto dc : delete_commits) {
							if (dc == new_commit) {
								found = true;
								break;
							}
						}

						if (!found)
							delete_commits.push_back(new_commit);
					}

					if (sq[7].is_null())
						clear_all = true;
					else {
						if (merged_trans) {
							string fn = sq[7];

							for (auto it = files.begin(); it != files.end(); it++) {
								if (it->filename == fn) {
									files.erase(it);
									break;
								}
							}
						}

						files.emplace_back(sq[7], sq[8]);
					}

					if (!sq.fetch_row())
						break;
				} while (true);
			}

			if (files.size() > 0 || clear_all)
				update_git(repo, name, email, description, files, clear_all, dt, tz_offset);

			while (!delete_commits.empty()) {
				tds.run("DELETE FROM Restricted.Git WHERE id=?", delete_commits.front());
				delete_commits.pop_front();
			}

			while (!delete_files.empty()) {
				tds.run("DELETE FROM Restricted.GitFiles WHERE file_id=?", delete_files.front());
				delete_files.pop_front();
			}

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

static void write_table_ddl(tds::Conn& tds, const string_view& schema, const string_view& table, const string_view& bind_token,
							unsigned int commit_id, const string_view& filename) {
	tds::Trans trans(tds); // not a real transaction - just to stop MSSQL moaning

	trans.committed = true;

	tds.run("BEGIN TRANSACTION; DECLARE @token VARCHAR(255) = ?; EXEC sp_bindsession @token;", bind_token);

	auto ddl = table_ddl(tds, schema, table);

	tds.run("INSERT INTO Restricted.GitFiles(id, filename, data) VALUES(?, ?, ?)", commit_id, filename, tds::binary_string(ddl));
}

int main(int argc, char** argv) {  
	string cmd;

	if (argc < 2)
		return 1;

	string db_server = argv[1];

	if (argc > 2)
		cmd = argv[2];

	if (cmd != "flush" && cmd != "table" && argc < 4)
		return 1;

	tds::Conn tds(db_server, "", "", db_app);

	try {
		string unixpath, def;

		{
			lockfile lf;

			if (cmd == "flush")
				flush_git(tds);
			else if (cmd == "table") {
				if (argc < 8)
					throw runtime_error("Too few arguments.");

				// FIXME - also specify DB?
				string schema = argv[3];
				string table = argv[4];
				string bind_token = argv[5];
				auto commit_id = stoi(argv[6]);
				string filename = argv[7];

				write_table_ddl(tds, schema, table, bind_token, commit_id, filename);
			} else {
				string repo_dir = cmd;
				string db = argv[3], old_db;

				{
					tds::Query sq(tds, "SELECT DB_NAME()");

					if (!sq.fetch_row())
						throw runtime_error("Could not get current database name.");

					old_db = sq[0];
				}

				tds.run("USE [" + db + "]"); // FIXME - can we definitely not do this with parameters?
				dump_sql(tds, repo_dir, db);

				tds.run("USE [" + old_db + "]");
				do_update_git(repo_dir);
			}
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;
		tds.run("INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", string_view(e.what()));
		throw;
	}

	return 0;
}
