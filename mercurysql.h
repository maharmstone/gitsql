#pragma once

#include <string>
#include <vector>
#include <Windows.h>
#include <sqlext.h>

using namespace std;

class SQLHStmt {
public:
	SQLHStmt(HDBC hdbc);
	~SQLHStmt();
	void SQLPrepare(const string& s);
	SQLRETURN SQLExecute();
	void SQLBindParameter(SQLUSMALLINT ipar, SQLSMALLINT fParamType, SQLSMALLINT fCType, SQLSMALLINT fSqlType, SQLULEN cbColDef,
		SQLSMALLINT ibScale, SQLPOINTER rgbValue, SQLLEN cbValueMax, SQLLEN * pcbValue);
	SQLRETURN SQLPutData(SQLPOINTER DataPtr, SQLLEN StrLen_or_Ind);
	SQLRETURN SQLExecDirect(const string& s);
	SQLRETURN SQLParamData(SQLPOINTER* ValuePtrPtr);
	bool SQLFetch();
	SQLRETURN SQLGetData(SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType, SQLPOINTER TargetValuePtr, SQLLEN BufferLength, SQLLEN* StrLen_or_IndPtr);
	SQLSMALLINT SQLNumParams();
	SQLSMALLINT SQLNumResultCols();
	SQLRETURN SQLDescribeParam(SQLUSMALLINT ParameterNumber, SQLSMALLINT* DataTypePtr, SQLULEN* ParameterSizePtr, SQLSMALLINT* DecimalDigitsPtr, SQLSMALLINT* NullablePtr);
	SQLRETURN SQLDescribeCol(SQLUSMALLINT ColumnNumber, SQLCHAR* ColumnName, SQLSMALLINT BufferLength, SQLSMALLINT* NameLengthPtr, SQLSMALLINT* DataTypePtr, SQLULEN* ColumnSizePtr, SQLSMALLINT* DecimalDigitsPtr, SQLSMALLINT* NullablePtr);

private:
	HSTMT hstmt = NULL;
};

class NullableString;

class SQLField {
public:
	SQLField(SQLHStmt& hstmt, unsigned int i);
	operator string() const;
	explicit operator signed long long() const;
	explicit operator unsigned int() const;
	operator NullableString() const;
	explicit operator double() const;
	bool operator==(const string& s) const;

	// FIXME - other operators (timestamp, ...?)

	SQLSMALLINT datatype, digits, nullable;
	string name;
	string str;
	signed long long val;
	double d;
	SQL_TIMESTAMP_STRUCT ts;
	bool null = false;

private:
	unsigned int colnum;
	unsigned short reallen;
};

class SQLQuery {
private:
	void add_params2(unsigned int i, signed long long t) {
		signed long long* buf;

		buf = (signed long long*)malloc(sizeof(t));
		*buf = t;

		bufs.push_back(buf);

		hstmt.SQLBindParameter(i + 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_VARCHAR, sizeof(t), 0, (SQLPOINTER)buf, sizeof(t), NULL);
	}

	void add_params2(unsigned int i, const string& t) {
		char* buf;

		buf = (char*)malloc(t.size() + 1);
		memcpy(buf, t.c_str(), t.size());
		buf[t.size()] = 0;

		bufs.push_back(buf);

		hstmt.SQLBindParameter(i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, t.size(), 0, (SQLPOINTER)buf, t.size(), NULL);
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
	SQLQuery(const string& q);

	template<typename... Args>
	SQLQuery(const string& q, Args... args) : hstmt(hdbc) {
		hstmt.SQLPrepare(q);

		add_params(0, args...);

		hstmt.SQLExecute();
	}

	~SQLQuery();
	bool fetch_row();
	unsigned int num_cols();
	string col_name(unsigned int i);
	SQLSMALLINT col_type(unsigned int i);

	SQLHStmt hstmt;
	vector<void*> bufs;
	vector<SQLField> cols;
};

template<class T>
class nullable {
public:
	nullable(const SQLField& f) {
		if (f.null)
			null = true;
		else
			t = (T)f;
	}

	nullable(const T& s) {
		t = s;
		null = false;
	}

	nullable(nullptr_t) {
		null = true;
	}

	nullable() {
		null = true;
	}

	operator T() const {
		if (null)
			return T();
		else
			return t;
	}

	bool is_null() const {
		return null;
	}

private:
	T t;
	bool null = false;
};

class sql_transaction {
public:
	sql_transaction();
	~sql_transaction();
	void commit();

private:
	bool committed = false;
};

class mercury_exception : public runtime_error {
public:
	mercury_exception(const string& arg, const char* file, int line) : runtime_error(arg) {
		msg = string(file) + ":" + to_string(line) + ": " + arg;
	}

	const char* what() const throw() {
		return msg.c_str();
	}

private:
	string msg;
};

class NullableString {
public:
	NullableString(string t) {
		s = t;
		null = false;
	}

	NullableString(char* t) {
		s = string(t);
		null = false;
	}

	NullableString(nullptr_t) {
		null = true;
	}

	operator string() const {
		if (null)
			return "";
		else
			return s;
	}

	bool is_null() const {
		return null;
	}

private:
	string s;
	bool null = false;
};

#define throw_exception(s) throw mercury_exception(s, __FILE__, __LINE__)

void _throw_sql_error(const string& funcname, SQLSMALLINT handle_type, SQLHANDLE handle, const char* filename, unsigned int line);
#define throw_sql_error(funcname, handle_type, handle) _throw_sql_error(funcname, handle_type, handle, __FILE__, __LINE__)

#define run_sql(s, ...) { SQLQuery sq(s, ##__VA_ARGS__); }
void SQLInsert(const string& tablename, const vector<string>& np, const vector<vector<NullableString>>& vp);
