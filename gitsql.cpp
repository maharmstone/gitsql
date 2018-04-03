#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlwapi.h>
#include <sqlext.h>
#include <git2.h>
#include <string>
#include <vector>

using namespace std;

#define CONNEXION_STRING "DRIVER=SQL Server;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n"
//#define CONNEXION_STRING "DRIVER=SQL Server Native Client 11.0;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n"
#define REPO_DIR "\\\\sys122-n\\Manual Data Files\\devel\\gitrepo\\" // FIXME - move to DB

HDBC hdbc;

static void throw_sql_error(string funcname, SQLSMALLINT handle_type, SQLHANDLE handle) {
	char state[SQL_SQLSTATE_SIZE + 1];
	char msg[1000];
	SQLINTEGER error;

	SQLGetDiagRecA(handle_type, handle, 1, (SQLCHAR*)state, &error, (SQLCHAR*)msg, sizeof(msg), NULL);

	throw funcname + " failed: " + msg;
}

class SQLQuery {
private:
	void add_params2(unsigned int i, signed long long t) {
		signed long long* buf;
		SQLRETURN rc;

		buf = (signed long long*)malloc(sizeof(t));
		*buf = t;

		bufs.push_back(buf);

		rc = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_VARCHAR, sizeof(t), 0, (SQLPOINTER)buf, sizeof(t), NULL);

		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLBindParameter", SQL_HANDLE_STMT, hstmt);
	}

	void add_params2(unsigned int i, const string& t) {
		char* buf;
		SQLRETURN rc;

		buf = (char*)malloc(t.size() + 1);
		memcpy(buf, t.c_str(), t.size());
		buf[t.size()] = 0;

		bufs.push_back(buf);

		rc = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, t.size(), 0, (SQLPOINTER)buf, t.size(), NULL);

		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLBindParameter", SQL_HANDLE_STMT, hstmt);
	}

	template<typename T>
	void add_params(unsigned int i, T t) {
		add_params2(i, t);
	}

	template<typename T, typename... Args>
	void add_params(unsigned int i, T t, Args... args) {
		add_params(i, t);
		add_params(i + 1, args...);
	}

public:
	SQLQuery(string q) {
		SQLRETURN rc;

		rc = SQLAllocStmt(hdbc, &hstmt);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLAllocStmt", SQL_HANDLE_DBC, hdbc);

		// FIXME - can we simplify if no params?

		rc = SQLPrepare(hstmt, (unsigned char*)q.c_str(), q.length());
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLPrepare", SQL_HANDLE_STMT, hstmt);

		rc = SQLExecute(hstmt);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLExecute", SQL_HANDLE_STMT, hstmt);
	}

	template<typename... Args>
	SQLQuery(string q, Args... args) {
		SQLRETURN rc;

		rc = SQLAllocStmt(hdbc, &hstmt);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLAllocStmt", SQL_HANDLE_DBC, hdbc);

		rc = SQLPrepare(hstmt, (unsigned char*)q.c_str(), q.length());
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLPrepare", SQL_HANDLE_STMT, hstmt);

		add_params(0, args...);

		rc = SQLExecute(hstmt);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLExecute", SQL_HANDLE_STMT, hstmt);
	}

	~SQLQuery() {
		if (hstmt)
			SQLFreeStmt(hstmt, SQL_DROP);

		for (unsigned int i = 0; i < bufs.size(); i++) {
			free(bufs[i]);
		}
		bufs.clear();
	}

	bool fetch_row() {
		SQLRETURN rc;

		rc = SQLFetch(hstmt);

		if (rc == SQL_NO_DATA)
			return false;
		else if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			return true;

		throw_sql_error("SQLFetch", SQL_HANDLE_STMT, hstmt);

		return false;
	}

	string row(unsigned int i) {
		string s = "                ";
		SQLRETURN rc;
		SQLINTEGER len;
		unsigned int origsize;

		rc = SQLGetData(hstmt, i + 1, SQL_C_CHAR, (SQLPOINTER)s.c_str(), s.size(), &len);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLGetData", SQL_HANDLE_STMT, hstmt);

		if (len == -1) // NULL
			return "";

		if ((unsigned int)len < s.size()) {
			s = s.substr(0, len);
			return s;
		}

		origsize = strlen(s.c_str()); // FIXME - what if first 0 is after end of string?
		len -= origsize;

		s = s.substr(0, origsize) + string((unsigned int)len, ' ');

		rc = SQLGetData(hstmt, i + 1, SQL_C_CHAR, (SQLPOINTER)&s[origsize], len + 1, &len);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			throw_sql_error("SQLGetData", SQL_HANDLE_STMT, hstmt);

		return s.substr(0, origsize + len);
	}

	HSTMT hstmt = NULL;
	vector<void*> bufs;
};

static string sanitize_fn(const string& fn) {
	// FIXME - strip out characters that NTFS doesn't like

	return fn;
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

static void dump_sql2(SQLQuery& sq) {
	while (sq.fetch_row()) {
		string schema = sanitize_fn(sq.row(0));
		string dir = string(REPO_DIR) + schema;
		string subdir;
		string fn;
		string name = sq.row(1);
		string def = sq.row(2);
		string type = sq.row(3);
		HANDLE h;
		DWORD written;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw "CreateDirectory returned error " + to_string(last_error);
		}

		if (type == "V")
			subdir = "views";
		else if (type == "P")
			subdir = "procedures";
		else if (type == "FN")
			subdir = "functions";

		dir += "\\" + subdir;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw "CreateDirectory returned error " + to_string(last_error);
		}

		fn = dir + "\\" + sanitize_fn(name) + ".sql";

		h = CreateFileA(fn.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (h == INVALID_HANDLE_VALUE)
			throw "CreateFile returned error " + to_string(GetLastError());

		if (!WriteFile(h, def.c_str(), def.size(), &written, NULL))
			throw "WriteFile returned error " + to_string(GetLastError());

		SetEndOfFile(h);

		CloseHandle(h);
	}
}

static void dump_sql3(SQLQuery& sq, string& unixpath, string& def) {
	if (sq.fetch_row()) {
		string schema = sanitize_fn(sq.row(0));
		string dir = string(REPO_DIR) + schema;
		string subdir;
		string fn;
		string name = sq.row(1);
		string value = sq.row(2);
		string type = sq.row(3);

		def = value;
		unixpath = schema;

		if (type == "V")
			subdir = "views";
		else if (type == "P")
			subdir = "procedures";
		else if (type == "FN")
			subdir = "functions";

		unixpath += "/" + subdir;

		unixpath += "/" + sanitize_fn(name) + ".sql";
	}
}

static void dump_sql(const string& schema, const string& obj, string& unixpath, string& def) {
	if (obj == "") {
		string s;

		SQLQuery sq("SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type) FROM sys.objects JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id WHERE objects.type='V' OR objects.type='P' OR objects.type='FN' ORDER BY schemas.name, objects.name");

		clear_dir(REPO_DIR);

		dump_sql2(sq);
	} else {
		SQLQuery sq("SELECT schemas.name, objects.name, sql_modules.definition, RTRIM(objects.type) FROM sys.objects JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id WHERE (objects.type='V' OR objects.type='P' OR objects.type='FN') AND schemas.name=? AND objects.name=?", schema, obj);

		dump_sql3(sq, unixpath, def);
	}
}

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw string(func) + " failed (" + lg2err->message + ").";

	throw string(func) + " failed.";
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

static void update_git(const string& user, const string& schema, const string& obj, const string& unixpath, const string& def) {
	git_repository* repo = NULL;
	unsigned int ret;

	git_libgit2_init();

	if ((ret = git_repository_open(&repo, REPO_DIR)))
		throw_git_error(ret, "git_repository_open");

	try {
		git_signature* sig;
		git_index* index;
		git_oid commit_id, tree_id;
		git_tree* tree;

		if ((ret = git_signature_now(&sig, user != "" ? user.c_str() : "Mark Harmstone", "mark.harmstone@boltonft.nhs.uk"))) // FIXME - don't hardcode, and use actual details
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

				if ((ret = git_blob_create_frombuffer(&blob, repo, def.c_str(), def.length())))
					throw_git_error(ret, "git_blob_create_frombuffer");

				upd.action = GIT_TREE_UPDATE_UPSERT; // FIXME - deleting
				upd.id = blob;
				upd.filemode = GIT_FILEMODE_BLOB;
				upd.path = unixpath.c_str();

				git_tree_create_updated(&oid, repo, parent_tree, 1, &upd);

				git_tree_lookup(&tree, repo, &oid);
			}

			try {
				if ((ret = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig, NULL, "Update", tree, 1, parent)))
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

int main(int argc, char** argv) {  
	HENV henv;
	string schema, user, obj;

	if (argc >= 4) {
		user = argv[1];
		schema = argv[2];
		obj = argv[3];
	}

	SQLAllocEnv(&henv);
	SQLAllocConnect(henv, &hdbc);

	try {
		try {
			SQLRETURN rc;
			string unixpath, def;

			rc = SQLDriverConnectA(hdbc, NULL, (unsigned char*)CONNEXION_STRING, (SQLSMALLINT)strlen(CONNEXION_STRING), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
				throw_sql_error("SQLDriverConnect", SQL_HANDLE_DBC, hdbc);

			// FIXME - handle deletions
			dump_sql(schema, obj, unixpath, def);
			update_git(user, schema, obj, unixpath, def);
		} catch (const char* s) {
			MessageBoxA(0, s, "Error", MB_ICONERROR);
			throw;
		} catch (string s) {
			MessageBoxA(0, s.c_str(), "Error", MB_ICONERROR);
			throw;
		} catch (...) {
			MessageBoxA(0, "Unrecognized exception.", "Error", MB_ICONERROR);
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
