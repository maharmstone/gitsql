#include <windows.h>
#include <sqlext.h>
#include <git2.h>
#include <string>
#include <vector>

using namespace std;

#define CONNEXION_STRING "DRIVER=SQL Server;SERVER=sys122-n;UID=Minerva_Apps;PWD=Inf0rmati0n"
#define REPO_DIR "W:\\devel\\gitrepo\\" // FIXME - move to DB

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
	void add_params(unsigned int i, Args... args) {
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

static void dump_sql() {
	// FIXME - also dump procedures etc.

	// FIXME - delete contents of directory first of all (apart from .git)

	SQLQuery sq("SELECT schemas.name, objects.name, sql_modules.definition FROM sys.objects JOIN sys.sql_modules ON sql_modules.object_id=objects.object_id JOIN sys.schemas ON schemas.schema_id=objects.schema_id WHERE objects.type='V' ORDER BY schemas.name, objects.name");

	while (sq.fetch_row()) {
		string dir = string(REPO_DIR) + sanitize_fn(sq.row(0));
		string fn;
		string name = sq.row(1);
		string def = sq.row(2);
		HANDLE h;
		DWORD written;

		if (!CreateDirectoryA(dir.c_str(), NULL)) {
			auto last_error = GetLastError();

			if (last_error != ERROR_ALREADY_EXISTS)
				throw "CreateDirectory returned error " + to_string(last_error);
		}

		dir += "\\views";

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

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw string(func) + " failed (" + lg2err->message + ").";

	throw string(func) + " failed.";
}

static void update_git() {
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

		if ((ret = git_signature_now(&sig, "Mark Harmstone", "mark.harmstone@boltonft.nhs.uk"))) // FIXME - don't hardcode
			throw_git_error(ret, "git_signature_now");

		try {
			if ((ret = git_repository_index(&index, repo)))
				throw_git_error(ret, "git_repository_index");

			// FIXME - loop through saved files and add
			// FIXME - remove from index any that have been deleted

			if ((ret = git_index_add_bypath(index, "test.txt")))
				throw_git_error(ret, "git_index_add_bypath");

			if ((ret = git_index_write_tree(&tree_id, index)))
				throw_git_error(ret, "git_index_write_tree");

			git_index_free(index);

			if ((ret = git_tree_lookup(&tree, repo, &tree_id)))
				throw_git_error(ret, "git_tree_lookup");

			try {
				git_commit* parent;
				git_oid parent_id;

				if ((ret = git_reference_name_to_id(&parent_id, repo, "HEAD")))
					throw_git_error(ret, "git_reference_name_to_id");

				if ((ret = git_commit_lookup(&parent, repo, &parent_id)))
					throw_git_error(ret, "git_commit_lookup");

				if ((ret = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig, NULL, "test commit", tree, 1, parent)))
					throw_git_error(ret, "git_commit_create_v");
			} catch (...) {
				git_tree_free(tree);
				throw;
			}

			git_tree_free(tree);

			// FIXME - push to remote server?*/
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	HENV henv;

	SQLAllocEnv(&henv);
	SQLAllocConnect(henv, &hdbc);

	try {
		try {
			SQLRETURN rc;

			rc = SQLDriverConnectA(hdbc, NULL, (unsigned char*)CONNEXION_STRING, (SQLSMALLINT)strlen(CONNEXION_STRING), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
				throw_sql_error("SQLDriverConnect", SQL_HANDLE_DBC, hdbc);

			//dump_sql();
			update_git();
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
