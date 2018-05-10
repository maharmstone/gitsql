#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlwapi.h>
#include <sqlext.h>
#include <git2.h>
#include <string>
#include <vector>
#include "mercurysql.h"

using namespace std;

static const char* connexion_strings[] = {
	"DRIVER=SQL Server Native Client 11.0;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n;MARS_Connection=Yes",
	"DRIVER=SQL Server Native Client 10.0;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n;MARS_Connection=Yes",
	"DRIVER=SQL Server;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n"
};

#define REPO_DIR "\\\\sys122-n\\Manual Data Files\\devel\\gitrepo\\" // FIXME - move to DB

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

static void clear_dir(const string& dir) {
	HANDLE h;
	WIN32_FIND_DATAA fff;

	h = FindFirstFileA((dir + "*").c_str(), &fff);

	if (h == INVALID_HANDLE_VALUE)
		return;

	do {
		if (fff.cFileName[0] != '.') {
			if (fff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				clear_dir(dir + fff.cFileName + "\\");
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

static void dump_sql2(SQLQuery& sq) {
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

static void dump_sql(const string& schema, const string& obj, const string& type, string& unixpath, string& def, bool& deleted) {
	if (obj == "") {
		string s;

		SQLQuery sq("SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type), extended_properties.value FROM sys.objects LEFT JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id LEFT JOIN sys.extended_properties ON extended_properties.major_id=objects.object_id AND extended_properties.minor_id=0 AND extended_properties.class=1 AND extended_properties.name='fulldump' WHERE objects.type='V' OR objects.type='P' OR objects.type='FN' OR objects.type='U' ORDER BY schemas.name, objects.name");

		clear_dir(REPO_DIR);

		dump_sql2(sq);
	} else {
		string subdir;
		bool fulldump;
		
		SQLQuery sq("SELECT sql_modules.definition, extended_properties.value FROM sys.objects LEFT JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id LEFT JOIN sys.extended_properties ON extended_properties.major_id=objects.object_id AND extended_properties.minor_id=0 AND extended_properties.class=1 AND extended_properties.name='fulldump' WHERE (objects.type='V' OR objects.type='P' OR objects.type='FN' OR objects.type='U') AND schemas.name=? AND objects.name=?", schema, obj);

		if (sq.fetch_row()) {
			def = sq.cols[0];
			fulldump = sq.cols[1] == "true";
			deleted = false;
		} else
			deleted = true;

		unixpath = sanitize_fn(schema);

		if (type == "V")
			subdir = "views";
		else if (type == "P")
			subdir = "procedures";
		else if (type == "FN")
			subdir = "functions";
		else if (type == "U")
			subdir = "tables";

		unixpath += "/" + subdir;
		unixpath += "/" + sanitize_fn(obj) + ".sql";

		if (type == "U") {
			SQLQuery sq2("EXEC dbo.sp_GetDDL ?", "[" + schema + "].[" + obj + "]");

			if (sq2.fetch_row())
				def = sq2.cols[0];

			if (fulldump)
				def += "\n" + dump_table(schema, obj);
		}
	}
}

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw runtime_error(string(func) + " failed (" + lg2err->message + ").");

	throw runtime_error(string(func) + " failed.");
}

static void git_add_dir(git_index* index, const string& dir, const string& unixpath) {
	HANDLE h;
	WIN32_FIND_DATAA fff;

	h = FindFirstFileA((REPO_DIR + dir + "*").c_str(), &fff);

	if (h == INVALID_HANDLE_VALUE)
		return;

	do {
		if (fff.cFileName[0] != '.') {
			if (fff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				git_add_dir(index, dir + fff.cFileName + "\\", unixpath + fff.cFileName + "/");
			else {
				unsigned int ret;

				if ((ret = git_index_add_bypath(index, (unixpath + fff.cFileName).c_str())))
					throw_git_error(ret, "git_index_add_bypath");
			}
		}
	} while (FindNextFileA(h, &fff));

	FindClose(h);
}


static void git_remove_dir(git_repository* repo, git_tree* tree, const string& dir, const string& unixdir, vector<string>& deleted) {
	size_t c = git_tree_entrycount(tree);

	for (size_t i = 0; i < c; i++) {
		const git_tree_entry* gte = git_tree_entry_byindex(tree, i);

		if (gte) {
			string name = git_tree_entry_name(gte);

			if (!PathFileExistsA((REPO_DIR + dir + name).c_str()))
				deleted.push_back(unixdir + name);
			else if (git_tree_entry_type(gte) == GIT_OBJ_TREE) {
				git_tree* subtree;
				unsigned int ret;

				if ((ret = git_tree_entry_to_object((git_object**)&subtree, repo, gte)))
					throw_git_error(ret, "git_tree_entry_to_object");

				git_remove_dir(repo, subtree, dir + name + "\\", unixdir + name + "/", deleted);
			}
		}
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

static void update_git(const string& user, const string& schema, const string& obj, const string& unixpath, string def, bool filedeleted) {
	git_repository* repo = NULL;
	unsigned int ret;

	replace_all(def, "\r\n", "\n"); // fix line endings

	git_libgit2_init();

	if ((ret = git_repository_open(&repo, REPO_DIR)))
		throw_git_error(ret, "git_repository_open");

	try {
		git_signature* sig;
		git_index* index;
		git_oid commit_id, tree_id;
		git_tree* tree;
		string gituser = "Mark Harmstone", gitemail = "mark.harmstone@boltonft.nhs.uk";

		if (upper(user.substr(0, 5)) == "XRBH\\") { // FIXME - fetch this directly from AD without relying on DB
			SQLQuery sq("SELECT givenName+' '+sn, mail FROM AD.ldap WHERE sAMAccountName=?", user.substr(5));

			if (sq.fetch_row()) {
				gituser = sq.cols[0];
				gitemail = sq.cols[1];
			}
		}

		if ((ret = git_signature_now(&sig, gituser.c_str(), gitemail.c_str())))
			throw_git_error(ret, "git_signature_now");

		try {
			git_commit* parent;
			git_oid parent_id;
			git_tree* parent_tree;
			vector<string> deleted;

			if ((ret = git_reference_name_to_id(&parent_id, repo, "HEAD")))
				throw_git_error(ret, "git_reference_name_to_id");

			if ((ret = git_commit_lookup(&parent, repo, &parent_id)))
				throw_git_error(ret, "git_commit_lookup");

			if ((ret = git_commit_tree(&parent_tree, parent)))
				throw_git_error(ret, "git_commit_tree");

			if ((ret = git_repository_index(&index, repo)))
				throw_git_error(ret, "git_repository_index");

			if (obj == "") {
				// loop through saved files and add
				git_add_dir(index, "", "");

				if ((ret = git_index_write_tree(&tree_id, index)))
					throw_git_error(ret, "git_index_write_tree");

				git_index_free(index);

				if ((ret = git_tree_lookup(&tree, repo, &tree_id)))
					throw_git_error(ret, "git_tree_lookup");

				// loop through repo and remove anything that's been deleted
				git_remove_dir(repo, parent_tree, "", "", deleted);

				if (deleted.size() > 0) {
					if ((ret = git_repository_index(&index, repo)))
						throw_git_error(ret, "git_repository_index");

					for (unsigned int i = 0; i < deleted.size(); i++) {
						if ((ret = git_index_remove_bypath(index, deleted[i].c_str())))
							throw_git_error(ret, "git_index_remove_bypath");
					}

					if ((ret = git_index_write_tree(&tree_id, index)))
						throw_git_error(ret, "git_index_write_tree");

					git_index_free(index);

					if ((ret = git_tree_lookup(&tree, repo, &tree_id)))
						throw_git_error(ret, "git_tree_lookup");
				}
			} else {
				git_oid oid, blob;
				git_tree_update upd;

				if (!filedeleted) {
					if ((ret = git_blob_create_frombuffer(&blob, repo, def.c_str(), def.length())))
						throw_git_error(ret, "git_blob_create_frombuffer");
				} else {
					git_tree_entry* gte;
					unsigned int ret;

					// don't delete file if not there to begin with
					ret = git_tree_entry_bypath(&gte, parent_tree, unixpath.c_str());

					if (ret == GIT_ENOTFOUND)
						return;
					else if (ret)
						throw_git_error(ret, "git_tree_entry_bypath");

					git_tree_entry_free(gte);
				}

				upd.action = filedeleted ? GIT_TREE_UPDATE_REMOVE : GIT_TREE_UPDATE_UPSERT;

				if (!filedeleted) {
					upd.id = blob;
					upd.filemode = GIT_FILEMODE_BLOB;
				}

				upd.path = unixpath.c_str();

				if ((ret = git_tree_create_updated(&oid, repo, parent_tree, 1, &upd)))
					throw_git_error(ret, "git_tree_create_updated");

				if ((ret = git_tree_lookup(&tree, repo, &oid)))
					throw_git_error(ret, "git_tree_lookup");
			}

			{
				git_diff* diff;
				size_t nd;

				if ((ret = git_diff_tree_to_tree(&diff, repo, parent_tree, tree, NULL)))
					throw_git_error(ret, "git_diff_tree_to_tree");

				nd = git_diff_num_deltas(diff);

				git_diff_free(diff);

				if (nd == 0) { // no changes - avoid doing empty commit
					git_signature_free(sig);
					git_repository_free(repo);
					return;
				}
			}

			try {
				if ((ret = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig, NULL, unixpath == "" ? "Update" : unixpath.c_str(), tree, 1, parent)))
					throw_git_error(ret, "git_commit_create_v");
			} catch (...) {
				git_tree_free(tree);
				throw;
			}

			git_tree_free(tree);

			// FIXME - push to remote server?
		} catch (...) {
			git_signature_free(sig);
			throw;
		}

		git_signature_free(sig);
	} catch (...) {
		git_repository_free(repo);
		throw;
	}

	git_repository_free(repo);
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
			UnlockFileEx(h, 0, 1, 0, NULL);
			CloseHandle(h);
		}
	}

private:
	HANDLE h = INVALID_HANDLE_VALUE;
};

int main(int argc, char** argv) {  
	HENV henv;
	string schema, user, type, obj;

	if (argc >= 5) {
		user = argv[1];
		type = argv[2];
		schema = argv[3];
		obj = argv[4];
	}

	SQLAllocEnv(&henv);
	SQLAllocConnect(henv, &hdbc);


	try {
		try {
			SQLRETURN rc;
			string unixpath, def;
			bool deleted, success = false;

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

				dump_sql(schema, obj, type, unixpath, def, deleted);
				update_git(user, schema, obj, unixpath, def, deleted);
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
