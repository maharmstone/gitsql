#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlwapi.h>
#include <sqlext.h>
#include <git2.h>
#include <string>
#include <vector>
#include "mercurysql.h"
#include "git.h"

using namespace std;

static const char* connexion_strings[] = {
	"DRIVER=SQL Server Native Client 11.0;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n;MARS_Connection=Yes",
	"DRIVER=SQL Server Native Client 10.0;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n;MARS_Connection=Yes",
	"DRIVER=SQL Server;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n"
};

//#define REPO_DIR "\\\\sys122-n\\Manual Data Files\\devel\\gitrepo\\" // FIXME - move to DB
#define REPO_DIR "\\\\sys283\\gitrepo\\" // FIXME - move to DB

HDBC hdbc;

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

static string dump_table(const string& schema, const string& table) {
	string obj = "[" + schema + "].[" + table + "]"; // FIXME - do we need to escape?
	string cols, s;

	SQLQuery sq("SELECT * FROM " + obj);

	while (sq.fetch_row()) {
		string vals;

		if (cols == "") {
			for (const auto& col : sq.cols) {
				if (cols != "")
					cols += ", ";

				cols += "[" + col.name + "]";
			}
		}

		s += "INSERT INTO " + obj + "(" + cols + ") VALUES(";

		// FIXME - "Unicode" columns might break here: MS stores them as UTF-16, and we use UTF-8

		for (const auto& col : sq.cols) {
			if (vals != "")
				vals += ", ";

			if (col.null)
				vals += "NULL";
			else if (col.datatype == SQL_INTEGER || col.datatype == SQL_BIT || col.datatype == SQL_NUMERIC || col.datatype == SQL_BIGINT)
				vals += to_string((signed long long)col);
			else if (col.datatype == SQL_FLOAT || col.datatype == SQL_DOUBLE)
				vals += to_string((double)col);
			else
				vals += "'" + sql_escape(string(col)) + "'";
		}

		s += vals;

		s += ");\n";
	}

	return s;
}

static void dump_sql() {
	string s;

	SQLQuery sq("SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type), extended_properties.value FROM sys.objects LEFT JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id LEFT JOIN sys.extended_properties ON extended_properties.major_id=objects.object_id AND extended_properties.minor_id=0 AND extended_properties.class=1 AND extended_properties.name='fulldump' WHERE objects.type='V' OR objects.type='P' OR objects.type='FN' OR objects.type='U' ORDER BY schemas.name, objects.name");

	clear_dir(REPO_DIR, true);

	while (sq.fetch_row()) {
		string schema = sq.cols[0];
		string dir = string(REPO_DIR) + sanitize_fn(schema);
		string subdir;
		string fn;
		string name = sq.cols[1];
		string def = sq.cols[2];
		string type = sq.cols[3];
		bool fulldump = sq.cols[4] == "true";
		HANDLE h;
		DWORD written;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw runtime_error("CreateDirectory returned error " + to_string(last_error));
		}

		if (type == "V")
			subdir = "views";
		else if (type == "P")
			subdir = "procedures";
		else if (type == "FN")
			subdir = "functions";
		else if (type == "U")
			subdir = "tables";

		if (type == "U") {
			SQLQuery sq2("EXEC dbo.sp_GetDDL ?", schema + "." + name);

			if (!sq2.fetch_row())
				throw runtime_error("Error calling dbo.sp_GetDDL for " + schema + "." + name + ".");

			def = sq2.cols[0];

			if (fulldump)
				def += "\n" + dump_table(schema, name);
		}

		replace_all(def, "\r\n", "\n");

		dir += "\\" + subdir;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw runtime_error("CreateDirectory returned error " + to_string(last_error));
		}

		fn = dir + "\\" + sanitize_fn(name) + ".sql";

		h = CreateFileA(fn.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (h == INVALID_HANDLE_VALUE)
			throw runtime_error("CreateFile returned error " + to_string(GetLastError()));

		if (!WriteFile(h, def.c_str(), def.size(), &written, NULL))
			throw runtime_error("WriteFile returned error " + to_string(GetLastError()));

		SetEndOfFile(h);

		CloseHandle(h);
	}
}

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw runtime_error(string(func) + " failed (" + lg2err->message + ").");

	throw runtime_error(string(func) + " failed.");
}

static void git_add_dir(GitIndex& index, const string& dir, const string& unixpath) {
	HANDLE h;
	WIN32_FIND_DATAA fff;

	h = FindFirstFileA((REPO_DIR + dir + "*").c_str(), &fff);

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

		if (!PathFileExistsA((REPO_DIR + dir + name).c_str()))
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

static string upper(string s) {
	for (auto& c : s) {
		if (c >= 'a' && c <= 'z')
			c -= 32;
	}

	return s;
}

static void do_update_git() {
	git_oid tree_id;
	git_commit* parent;
	git_oid parent_id;
	vector<string> deleted;

	git_libgit2_init();

	GitRepo repo(REPO_DIR);

	GitSignature sig("System", "business.intelligence@boltonft.nhs.uk");

	repo.reference_name_to_id(&parent_id, "HEAD");

	repo.commit_lookup(&parent, &parent_id);

	GitTree parent_tree(parent);

	{
		GitIndex index(repo);

		// loop through saved files and add
		git_add_dir(index, "", "");

		// loop through repo and remove anything that's been deleted
		git_remove_dir(repo, parent_tree, "", "", deleted);

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

static void flush_git() {
	git_libgit2_init();

	GitRepo repo(REPO_DIR);

	while (true) {
		sql_transaction trans;

		SQLQuery sq("SELECT TOP 1 Git.id, COALESCE(LDAP.givenName+' '+LDAP.sn, LDAP.name, Git.username), COALESCE(LDAP.mail,'business.intelligence@boltonft.nhs.uk'), Git.description, DATEDIFF(SECOND,'19700101',Git.dt), Git.offset FROM Restricted.Git LEFT JOIN AD.ldap ON LEFT(Git.username,5)='XRBH\\' AND ldap.sAMAccountName=RIGHT(Git.username,LEN(Git.username)-5) ORDER BY Git.id");

		if (sq.fetch_row()) {
			unsigned int commit = (signed long long)sq.cols[0];
			string name = sq.cols[1];
			string email = sq.cols[2];
			string description = sq.cols[3];
			unsigned int dt = (signed long long)sq.cols[4];
			int offset = (signed long long)sq.cols[5];
			vector<git_file> files;

			SQLQuery sq2("SELECT filename, data FROM Restricted.GitFiles WHERE id=?", commit);

			while (sq2.fetch_row()) {
				files.push_back({ sq2.cols[0], sq2.cols[1] });
			}

			if (files.size() > 0)
				update_git(repo, name, email, description, dt, offset, files);

			run_sql("DELETE FROM Restricted.Git WHERE id=?", (signed long long)sq.cols[0]);
			run_sql("DELETE FROM Restricted.GitFiles WHERE id=?", (signed long long)sq.cols[0]);
		} else
			break;

		trans.commit();
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

int main(int argc, char** argv) {  
	HENV henv;
	string cmd;

	if (argc > 1)
		cmd = argv[1];

	SQLAllocEnv(&henv);
	SQLAllocConnect(henv, &hdbc);

	try {
		try {
			SQLRETURN rc;
			string unixpath, def;
			bool success = false;

			for (const auto& cs : connexion_strings) {
				rc = SQLDriverConnectA(hdbc, NULL, (unsigned char*)cs, (SQLSMALLINT)strlen(cs), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

				if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
					success = true;
					break;
				}
			}

			if (!success)
				throw_sql_error("SQLDriverConnect", SQL_HANDLE_DBC, hdbc);

			{
				lockfile lf;

				if (cmd == "flush")
					flush_git();
				else {
					dump_sql();
					do_update_git();
				}
			}
		} catch (const exception& e) {
			SQLQuery sq("INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", e.what());
			//MessageBoxA(0, s, "Error", MB_ICONERROR);
			throw;
		} catch (...) {
			SQLQuery sq("INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", "Unrecognized exception.");
			//MessageBoxA(0, "Unrecognized exception.", "Error", MB_ICONERROR);
			throw;
		}
	} catch (...) {
		SQLFreeConnect(henv);
		SQLFreeEnv(henv);
		SQLFreeConnect(hdbc);

		return 0;
	}

	SQLFreeConnect(henv);
	SQLFreeEnv(henv);
	SQLFreeConnect(hdbc);

	return 0;
}
