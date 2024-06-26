#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <shlwapi.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <pwd.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <map>
#include <stdexcept>
#include <filesystem>
#include <span>
#include <charconv>
#include <iostream>
#include <tdscpp.h>
#include <nlohmann/json.hpp>
#include "git.h"
#include "gitsql.h"
#include "outptr.h"

using namespace std;
using json = nlohmann::json;

string db_username, db_password;

#ifndef _WIN32
errno_error::errno_error(string_view function, int en) : msg(function) {
	msg += " failed (";

	switch (en) {
		case EPERM:
			msg += "EPERM";
			break;
		case ENOENT:
			msg += "ENOENT";
			break;
		case ESRCH:
			msg += "ESRCH";
			break;
		case EINTR:
			msg += "EINTR";
			break;
		case EIO:
			msg += "EIO";
			break;
		case ENXIO:
			msg += "ENXIO";
			break;
		case E2BIG:
			msg += "E2BIG";
			break;
		case ENOEXEC:
			msg += "ENOEXEC";
			break;
		case EBADF:
			msg += "EBADF";
			break;
		case ECHILD:
			msg += "ECHILD";
			break;
		case EAGAIN:
			msg += "EAGAIN";
			break;
		case ENOMEM:
			msg += "ENOMEM";
			break;
		case EACCES:
			msg += "EACCES";
			break;
		case EFAULT:
			msg += "EFAULT";
			break;
		case ENOTBLK:
			msg += "ENOTBLK";
			break;
		case EBUSY:
			msg += "EBUSY";
			break;
		case EEXIST:
			msg += "EEXIST";
			break;
		case EXDEV:
			msg += "EXDEV";
			break;
		case ENODEV:
			msg += "ENODEV";
			break;
		case ENOTDIR:
			msg += "ENOTDIR";
			break;
		case EISDIR:
			msg += "EISDIR";
			break;
		case EINVAL:
			msg += "EINVAL";
			break;
		case ENFILE:
			msg += "ENFILE";
			break;
		case EMFILE:
			msg += "EMFILE";
			break;
		case ENOTTY:
			msg += "ENOTTY";
			break;
		case ETXTBSY:
			msg += "ETXTBSY";
			break;
		case EFBIG:
			msg += "EFBIG";
			break;
		case ENOSPC:
			msg += "ENOSPC";
			break;
		case ESPIPE:
			msg += "ESPIPE";
			break;
		case EROFS:
			msg += "EROFS";
			break;
		case EMLINK:
			msg += "EMLINK";
			break;
		case EPIPE:
			msg += "EPIPE";
			break;
		case EDOM:
			msg += "EDOM";
			break;
		case ERANGE:
			msg += "ERANGE";
			break;
		case ENAMETOOLONG:
			msg += "ENAMETOOLONG";
			break;
		case ENOLCK:
			msg += "ENOLCK";
			break;
		case ENOSYS:
			msg += "ENOSYS";
			break;
		case ENOTEMPTY:
			msg += "ENOTEMPTY";
			break;
		case ELOOP:
			msg += "ELOOP";
			break;
		case ENOMSG:
			msg += "ENOMSG";
			break;
		case EIDRM:
			msg += "EIDRM";
			break;
		case ECHRNG:
			msg += "ECHRNG";
			break;
		case EL2NSYNC:
			msg += "EL2NSYNC";
			break;
		case EL3HLT:
			msg += "EL3HLT";
			break;
		case EL3RST:
			msg += "EL3RST";
			break;
		case ELNRNG:
			msg += "ELNRNG";
			break;
		case EUNATCH:
			msg += "EUNATCH";
			break;
		case ENOCSI:
			msg += "ENOCSI";
			break;
		case EL2HLT:
			msg += "EL2HLT";
			break;
		case EBADE:
			msg += "EBADE";
			break;
		case EBADR:
			msg += "EBADR";
			break;
		case EXFULL:
			msg += "EXFULL";
			break;
		case ENOANO:
			msg += "ENOANO";
			break;
		case EBADRQC:
			msg += "EBADRQC";
			break;
		case EBADSLT:
			msg += "EBADSLT";
			break;
		case EDEADLOCK:
			msg += "EDEADLOCK";
			break;
		case EBFONT:
			msg += "EBFONT";
			break;
		case ENOSTR:
			msg += "ENOSTR";
			break;
		case ENODATA:
			msg += "ENODATA";
			break;
		case ETIME:
			msg += "ETIME";
			break;
		case ENOSR:
			msg += "ENOSR";
			break;
		case ENONET:
			msg += "ENONET";
			break;
		case ENOPKG:
			msg += "ENOPKG";
			break;
		case EREMOTE:
			msg += "EREMOTE";
			break;
		case ENOLINK:
			msg += "ENOLINK";
			break;
		case EADV:
			msg += "EADV";
			break;
		case ESRMNT:
			msg += "ESRMNT";
			break;
		case ECOMM:
			msg += "ECOMM";
			break;
		case EPROTO:
			msg += "EPROTO";
			break;
		case EMULTIHOP:
			msg += "EMULTIHOP";
			break;
		case EDOTDOT:
			msg += "EDOTDOT";
			break;
		case EBADMSG:
			msg += "EBADMSG";
			break;
		case EOVERFLOW:
			msg += "EOVERFLOW";
			break;
		case ENOTUNIQ:
			msg += "ENOTUNIQ";
			break;
		case EBADFD:
			msg += "EBADFD";
			break;
		case EREMCHG:
			msg += "EREMCHG";
			break;
		case ELIBACC:
			msg += "ELIBACC";
			break;
		case ELIBBAD:
			msg += "ELIBBAD";
			break;
		case ELIBSCN:
			msg += "ELIBSCN";
			break;
		case ELIBMAX:
			msg += "ELIBMAX";
			break;
		case ELIBEXEC:
			msg += "ELIBEXEC";
			break;
		case EILSEQ:
			msg += "EILSEQ";
			break;
		case ERESTART:
			msg += "ERESTART";
			break;
		case ESTRPIPE:
			msg += "ESTRPIPE";
			break;
		case EUSERS:
			msg += "EUSERS";
			break;
		case ENOTSOCK:
			msg += "ENOTSOCK";
			break;
		case EDESTADDRREQ:
			msg += "EDESTADDRREQ";
			break;
		case EMSGSIZE:
			msg += "EMSGSIZE";
			break;
		case EPROTOTYPE:
			msg += "EPROTOTYPE";
			break;
		case ENOPROTOOPT:
			msg += "ENOPROTOOPT";
			break;
		case EPROTONOSUPPORT:
			msg += "EPROTONOSUPPORT";
			break;
		case ESOCKTNOSUPPORT:
			msg += "ESOCKTNOSUPPORT";
			break;
		case EOPNOTSUPP:
			msg += "EOPNOTSUPP";
			break;
		case EPFNOSUPPORT:
			msg += "EPFNOSUPPORT";
			break;
		case EAFNOSUPPORT:
			msg += "EAFNOSUPPORT";
			break;
		case EADDRINUSE:
			msg += "EADDRINUSE";
			break;
		case EADDRNOTAVAIL:
			msg += "EADDRNOTAVAIL";
			break;
		case ENETDOWN:
			msg += "ENETDOWN";
			break;
		case ENETUNREACH:
			msg += "ENETUNREACH";
			break;
		case ENETRESET:
			msg += "ENETRESET";
			break;
		case ECONNABORTED:
			msg += "ECONNABORTED";
			break;
		case ECONNRESET:
			msg += "ECONNRESET";
			break;
		case ENOBUFS:
			msg += "ENOBUFS";
			break;
		case EISCONN:
			msg += "EISCONN";
			break;
		case ENOTCONN:
			msg += "ENOTCONN";
			break;
		case ESHUTDOWN:
			msg += "ESHUTDOWN";
			break;
		case ETOOMANYREFS:
			msg += "ETOOMANYREFS";
			break;
		case ETIMEDOUT:
			msg += "ETIMEDOUT";
			break;
		case ECONNREFUSED:
			msg += "ECONNREFUSED";
			break;
		case EHOSTDOWN:
			msg += "EHOSTDOWN";
			break;
		case EHOSTUNREACH:
			msg += "EHOSTUNREACH";
			break;
		case EALREADY:
			msg += "EALREADY";
			break;
		case EINPROGRESS:
			msg += "EINPROGRESS";
			break;
		case ESTALE:
			msg += "ESTALE";
			break;
		case EUCLEAN:
			msg += "EUCLEAN";
			break;
		case ENOTNAM:
			msg += "ENOTNAM";
			break;
		case ENAVAIL:
			msg += "ENAVAIL";
			break;
		case EISNAM:
			msg += "EISNAM";
			break;
		case EREMOTEIO:
			msg += "EREMOTEIO";
			break;
		case EDQUOT:
			msg += "EDQUOT";
			break;
		case ENOMEDIUM:
			msg += "ENOMEDIUM";
			break;
		case EMEDIUMTYPE:
			msg += "EMEDIUMTYPE";
			break;
		case ECANCELED:
			msg += "ECANCELED";
			break;
		case ENOKEY:
			msg += "ENOKEY";
			break;
		case EKEYEXPIRED:
			msg += "EKEYEXPIRED";
			break;
		case EKEYREVOKED:
			msg += "EKEYREVOKED";
			break;
		case EKEYREJECTED:
			msg += "EKEYREJECTED";
			break;
		case EOWNERDEAD:
			msg += "EOWNERDEAD";
			break;
		case ENOTRECOVERABLE:
			msg += "ENOTRECOVERABLE";
			break;
		case ERFKILL:
			msg += "ERFKILL";
			break;
		case EHWPOISON:
			msg += "EHWPOISON";
			break;
		default:
			msg += format("{}", en);
			break;
	}

	msg += ")";
}
#endif

// strip out characters that NTFS doesn't like
static string sanitize_fn(string_view fn) {
	string s;

	for (auto c : fn) {
		if (c != '<' && c != '>' && c != ':' && c != '"' && c != '/' && c != '\\' && c != '|' && c != '?' && c != '*')
			s += c;
	}

	return s;
}

struct sql_obj {
	sql_obj(string_view schema, string_view name, string_view def, string_view type = "", int64_t id = 0, bool has_perms = false, bool quoted_identifier = false) :
		schema(schema), name(name), def(def), type(type), id(id), has_perms(has_perms), quoted_identifier(quoted_identifier) { }

	string schema, name, def, type;
	int64_t id;
	bool has_perms;
	bool quoted_identifier;
};

struct sql_perms {
	sql_perms(string_view user, string_view type, string_view perm) : user(user), type(type) {
		perms.emplace_back(perm);
	}

	string user;
	string type;
	vector<string> perms;
};

static string grant_string(const vector<sql_perms>& perms, string_view obj) {
	string ret;

	for (const auto& p : perms) {
		bool first = true;

		ret += p.type + " ";

		for (const auto& p2 : p.perms) {
			if (!first)
				ret += ", ";

			ret += p2;
			first = false;
		}

		ret += " ON " + string(obj);
		ret += " TO ";
		ret += brackets_escape(p.user);
		ret += ";\n";
	}

	return ret;
}

static string get_schema_definition(tds::tds& tds, string_view name) {
	vector<sql_perms> perms;
	string ret = "CREATE SCHEMA " + brackets_escape(name) + ";\n";

	{
		tds::query sq(tds, R"(SELECT database_permissions.state_desc,
	database_permissions.permission_name,
	USER_NAME(database_permissions.grantee_principal_id)
FROM sys.database_permissions
JOIN sys.database_principals ON database_principals.principal_id = database_permissions.grantee_principal_id
WHERE database_permissions.class_desc = 'SCHEMA' AND
	database_permissions.major_id = SCHEMA_ID(?)
ORDER BY USER_NAME(database_permissions.grantee_principal_id),
	database_permissions.state_desc,
	database_permissions.permission_name)", name);

		while (sq.fetch_row()) {
			bool found = false;
			auto type = (string)sq[0];
			auto user = (string)sq[2];

			for (auto& p : perms) {
				if (p.user == user && p.type == type) {
					p.perms.emplace_back((string)sq[1]);
					found = true;
					break;
				}
			}

			if (!found)
				perms.emplace_back(user, type, (string)sq[1]);
		}
	}

	if (perms.empty())
		return ret;

	ret += "GO\n\n";
	ret += grant_string(perms, "SCHEMA :: " + brackets_escape(name));

	return ret;
}

static string get_role_definition(tds::tds& tds, string_view name, int64_t id) {
	string ret;
	bool first = true;

	ret = "CREATE ROLE " + brackets_escape(name) + ";\n";

	tds::query sq(tds, R"(SELECT database_principals.name
FROM sys.database_role_members
JOIN sys.database_principals ON database_principals.principal_id = database_role_members.member_principal_id
WHERE database_role_members.role_principal_id = ?
ORDER BY database_principals.name)", id);

	while (sq.fetch_row()) {
		if (first)
			ret += "\n";

		ret += "ALTER ROLE " + brackets_escape(name) + " ADD MEMBER " + brackets_escape((string)sq[0]) + ";\n";

		first = false;
	}

	return ret;
}

static string get_type_definition(string_view name, string_view schema, int32_t system_type_id,
								  int16_t max_length, uint8_t precision, uint8_t scale,
								  bool is_nullable) {
	string ret;
	string escaped_name = brackets_escape(schema) + "." + brackets_escape(name);

	ret = "DROP TYPE IF EXISTS " + escaped_name + ";\n\n";

	ret += "CREATE TYPE " + escaped_name + " FROM ";


	switch (system_type_id) {
		case 34:
			ret += "IMAGE";
			break;

		case 35:
			ret += "TEXT";
			break;

		case 36:
			ret += "UNIQUEIDENTIFIER";
			break;

		case 40:
			ret += "DATE";
			break;

		case 41:
			ret += "TIME("s + to_string(scale) + ")"s;
			break;

		case 42:
			ret += "DATETIME2("s + to_string(scale) + ")"s;
			break;

		case 43:
			ret += "DATETIMEOFFSET("s + to_string(scale) + ")"s;
			break;

		case 48:
			ret += "TINYINT";
			break;

		case 52:
			ret += "SMALLINT";
			break;

		case 56:
			ret += "INT";
			break;

		case 58:
			ret += "SMALLDATETIME";
			break;

		case 59:
			ret += "REAL";
			break;

		case 60:
			ret += "MONEY";
			break;

		case 61:
			ret += "DATETIME";
			break;

		case 62:
			ret += precision <= 24 ? "REAL" : "FLOAT";
			break;

		case 98:
			ret += "SQL_VARIANT";
			break;

		case 99:
			ret += "NTEXT";
			break;

		case 104:
			ret += "BIT";
			break;

		case 106:
			ret += "DECIMAL("s + to_string(precision) + ","s + to_string(scale) + ")"s;
			break;

		case 108:
			ret += "NUMERIC("s + to_string(precision) + ","s + to_string(scale) + ")"s;
			break;

		case 122:
			ret += "SMALLMONEY";
			break;

		case 127:
			ret += "BIGINT";
			break;

		case 165:
			ret += "VARBINARY("s + (max_length == -1 ? "MAX"s : to_string(max_length)) + ")"s;
			break;

		case 167:
			ret += "VARCHAR("s + (max_length == -1 ? "MAX"s : to_string(max_length)) + ")"s;
			break;

		case 173:
			ret += "BINARY(" + to_string(max_length) + ")";
			break;

		case 175:
			ret += "CHAR(" + to_string(max_length) + ")";
			break;

		case 189:
			ret += "TIMESTAMP";
			break;

		case 231:
			ret += "NVARCHAR("s + (max_length == -1 ? "MAX"s : to_string(max_length / 2)) + ")"s;
			break;

		case 239:
			ret += "NCHAR(" + to_string(max_length / 2) + ")";
			break;

		case 241:
			ret += "XML";
			break;

		default:
			throw formatted_error("Unrecognized system type {}.", system_type_id);
	}

	ret += is_nullable ? " NULL" : " NOT NULL";

	ret += ";\n";


	return ret;
}

static string object_perms(tds::tds& tds, int64_t id, string_view name) {
	vector<sql_perms> perms;

	{
		tds::query sq(tds, R"(SELECT database_permissions.state_desc,
database_permissions.permission_name,
USER_NAME(database_permissions.grantee_principal_id)
FROM sys.database_permissions
JOIN sys.database_principals ON database_principals.principal_id = database_permissions.grantee_principal_id
WHERE database_permissions.class_desc = 'OBJECT_OR_COLUMN' AND
	database_permissions.major_id = ?
ORDER BY USER_NAME(database_permissions.grantee_principal_id),
	database_permissions.state_desc,
	database_permissions.permission_name)", id);

		while (sq.fetch_row()) {
			bool found = false;
			auto type = (string)sq[0];
			auto user = (string)sq[2];

			for (auto& p : perms) {
				if (p.user == user && p.type == type) {
					p.perms.emplace_back((string)sq[1]);
					found = true;
					break;
				}
			}

			if (!found)
				perms.emplace_back(user, type, (string)sq[1]);
		}
	}

	if (perms.empty())
		return "";

	return "GO\n\n" + grant_string(perms, name);
}

static string fix_whitespace(string_view sv) {
	string s;

	s.reserve(sv.length());

	do {
		auto nl = sv.find('\n');

		if (nl == string::npos) {
			s += sv;
			return s;
		}

		auto line = sv.substr(0, nl);

		while (!line.empty() && (line.back() == '\t' || line.back() == '\r' || line.back() == ' ')) {
			line.remove_suffix(1);
		}

		s += line;
		s += "\n";

		sv = sv.substr(nl + 1);
	} while (true);

	return s;
}

#ifdef _WIN32
static unique_handle open_process_token(HANDLE process_handle, DWORD desired_access) {
	HANDLE h;

	if (!OpenProcessToken(process_handle, desired_access, &h))
		throw last_error("OpenProcessToken", GetLastError());

	return unique_handle{h};
}

static vector<uint8_t> get_token_information(HANDLE token, TOKEN_INFORMATION_CLASS token_information_class) {
	DWORD retlen;

	if (!GetTokenInformation(token, token_information_class, nullptr, 0, &retlen) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		throw last_error("GetTokenInformation", GetLastError());

	vector<uint8_t> buf(retlen);

	if (!GetTokenInformation(token, token_information_class, buf.data(), (DWORD)buf.size(), &retlen))
		throw last_error("GetTokenInformation", GetLastError());

	return buf;
}
#endif

string get_current_username() {
#ifdef _WIN32
	auto token = open_process_token(GetCurrentProcess(), TOKEN_QUERY);

	auto buf = get_token_information(token.get(), TokenUser);
	auto& tu = *(TOKEN_USER*)buf.data();

	array<char16_t, 255> username, domain;
	auto usernamelen = (DWORD)username.size(), domainlen = (DWORD)domain.size();
	SID_NAME_USE use;

	if (!LookupAccountSidW(nullptr, tu.User.Sid, (WCHAR*)username.data(), &usernamelen, (WCHAR*)domain.data(),
						   &domainlen, &use)) {
		throw last_error("LookupAccountSid", GetLastError());
	}

	auto usernamesv = u16string_view(username.data(), usernamelen);
	auto domainsv = u16string_view(domain.data(), domainlen);

	return tds::utf16_to_utf8(domainsv) + "\\" + tds::utf16_to_utf8(usernamesv);
#else
	auto uid = getuid();

	passwd pwd;
	passwd* pwdp = nullptr;
	char buf[1024];

	// FIXME - ERANGE

	auto ret = getpwuid_r(uid, &pwd, buf, sizeof(buf), &pwdp);
	if (ret)
		throw errno_error("getpwduid_r", errno);

	if (!pwdp)
		throw formatted_error("Unable to get name for UID {}.", uid);

	return pwd.pw_name;
#endif
}

void get_current_user_details(string& name, string& email) {
#ifdef _WIN32
	auto token = open_process_token(GetCurrentProcess(), TOKEN_QUERY);

	auto buf = get_token_information(token.get(), TokenUser);
	auto& tu = *(TOKEN_USER*)buf.data();

	try {
		get_ldap_details_from_sid(tu.User.Sid, name, email);
	} catch (...) {
		name.clear();
		email.clear();
	}

	if (name.empty() || email.empty()) {
		array<char16_t, 255> username, domain;
		auto usernamelen = (DWORD)username.size(), domainlen = (DWORD)domain.size();
		SID_NAME_USE use;

		if (!LookupAccountSidW(nullptr, tu.User.Sid, (WCHAR*)username.data(), &usernamelen, (WCHAR*)domain.data(),
							   &domainlen, &use)) {
			throw last_error("LookupAccountSid", GetLastError());
		}

		auto usernamesv = u16string_view(username.data(), usernamelen);
		auto domainsv = u16string_view(domain.data(), domainlen);

		if (name.empty())
			name = tds::utf16_to_utf8(usernamesv);

		if (email.empty())
			email = tds::utf16_to_utf8(usernamesv) + "@"s + tds::utf16_to_utf8(domainsv);
	}
#else
	auto username = get_current_username();

	try {
		get_ldap_details_from_name(username, name, email);
	} catch (...) {
		name.clear();
		email.clear();
	}

	if (name.empty())
		name = username;

	if (email.empty())
		email = username + "@localhost"s; // FIXME - computer name?
#endif
}

static void dump_partition_functions(tds::tds& tds, git_update& gu) {
	struct partfunc {
		partfunc(int32_t id, string_view name, bool boundary_value_on_right, string_view type, int max_length,
				 int precision, int scale, string_view collation_name) :
				 id(id), name(name), boundary_value_on_right(boundary_value_on_right), type(type),
				 max_length(max_length), precision(precision), scale(scale), collation_name(collation_name) {
		}

		int32_t id;
		string name;
		bool boundary_value_on_right;
		string type;
		int max_length;
		int precision;
		int scale;
		string collation_name;
		vector<tds::value> values;
	};

	vector<partfunc> funcs;

	{
		tds::query sq(tds, R"(SELECT partition_functions.function_id,
	partition_functions.name,
	partition_functions.boundary_value_on_right,
	UPPER(types.name),
	partition_parameters.max_length,
	partition_parameters.precision,
	partition_parameters.scale,
	CASE WHEN partition_parameters.collation_name != CONVERT(VARCHAR(MAX),DATABASEPROPERTYEX(DB_NAME(), 'Collation')) THEN partition_parameters.collation_name END,
	partition_range_values.value
FROM sys.partition_functions
JOIN sys.partition_parameters ON partition_parameters.function_id = partition_functions.function_id AND partition_parameters.parameter_id = 1
JOIN sys.types ON types.user_type_id = partition_parameters.user_type_id
LEFT JOIN sys.partition_range_values ON partition_range_values.function_id = partition_functions.function_id AND partition_range_values.parameter_id = 1
ORDER BY partition_functions.function_id, partition_range_values.boundary_id)");

		while (sq.fetch_row()) {
			auto function_id = (int32_t)sq[0];

			if (funcs.empty() || funcs.back().id != function_id) {
				funcs.emplace_back(function_id, (string)sq[1], (unsigned int)sq[2] != 0, (string)sq[3], (int)sq[4],
								   (int)sq[5], (int)sq[6], (string)sq[7]);
			}

			if (!sq[8].is_null)
				funcs.back().values.emplace_back(sq[8]);
		}
	}

	for (const auto& f : funcs) {
		string sql;

		sql = "CREATE PARTITION FUNCTION ";
		sql += brackets_escape(f.name);
		sql += "(";

		sql += type_to_string(f.type, f.max_length, f.precision, f.scale);

		if ((f.type == "VARCHAR" || f.type == "CHAR" || f.type == "NVARCHAR" || f.type == "NCHAR") && !f.collation_name.empty())
			sql += " COLLATE " + f.collation_name;

		sql += ") AS RANGE ";
		sql += f.boundary_value_on_right ? "RIGHT" : "LEFT";
		sql += " FOR VALUES (";

		bool first = true;

		for (const auto& v : f.values) {
			if (!first)
				sql += ", ";

			sql += v.to_literal();
			first = false;
		}

		sql += ");\n";

		gu.add_file("partition_functions/" + f.name + ".sql", sql);
	}
}

static void dump_partition_schemes(tds::tds& tds, git_update& gu) {
	struct partscheme {
		partscheme(int32_t id, string_view scheme_name, string_view func_name) :
			id(id), scheme_name(scheme_name), func_name(func_name) {
		}

		int32_t id;
		string scheme_name;
		string func_name;
		vector<string> dests;
	};

	vector<partscheme> schemes;

	{
		tds::query sq(tds, R"(SELECT partition_schemes.data_space_id, partition_schemes.name, partition_functions.name, data_spaces.name
FROM sys.partition_schemes
JOIN sys.partition_functions ON partition_functions.function_id = partition_schemes.function_id
LEFT JOIN sys.destination_data_spaces ON destination_data_spaces.partition_scheme_id = partition_schemes.data_space_id
LEFT JOIN sys.data_spaces ON data_spaces.data_space_id = destination_data_spaces.data_space_id
ORDER BY partition_schemes.data_space_id, destination_data_spaces.destination_id)");

		while (sq.fetch_row()) {
			auto id = (int32_t)sq[0];

			if (schemes.empty() || schemes.back().id != id)
				schemes.emplace_back(id, (string)sq[1], (string)sq[2]);

			if (!sq[3].is_null)
				schemes.back().dests.emplace_back((string)sq[3]);
		}
	}

	for (const auto& s : schemes) {
		string sql;

		sql = "CREATE PARTITION SCHEME ";
		sql += brackets_escape(s.scheme_name);
		sql += " AS PARTITION ";
		sql += brackets_escape(s.func_name);

		bool all = false;
		if (!s.dests.empty()) {
			const auto& d = s.dests.front();

			all = ranges::all_of(s.dests, [&d](const auto& d2) {
				return d2 == d;
			});
		}

		if (all)
			sql += " ALL TO (" + brackets_escape(s.dests.front());
		else {
			sql += " TO (";

			bool first = true;
			for (const auto& d : s.dests) {
				if (!first)
					sql += ", ";

				sql += brackets_escape(d);

				first = false;
			}
		}

		sql += ");\n";

		gu.add_file("partition_schemes/" + s.scheme_name + ".sql", sql);
	}
}

static string pages_to_data_size(int32_t pages) {
	// 1 page = 8 KB

	if (pages >= 134217728 && (pages % 134217728) == 0)
		return to_string(pages / 134217728) + " TB";

	if (pages >= 131072 && (pages % 131072) == 0)
		return to_string(pages / 131072) + " GB";

	if (pages >= 128 && (pages % 128) == 0)
		return to_string(pages / 128) + " MB";

	return to_string(pages * 8) + " KB";
}

static void dump_filegroups(tds::tds& tds, git_update& gu) {
	struct fg_file {
		fg_file(string_view name, string_view physical_name, int32_t size, int32_t max_size, int32_t growth, bool is_percent_growth) :
			name(name), physical_name(physical_name), size(size), max_size(max_size), growth(growth),
			is_percent_growth(is_percent_growth) {
		}

		string name;
		string physical_name;
		int32_t size;
		int32_t max_size;
		int32_t growth;
		bool is_percent_growth;
	};

	struct fg {
		fg(int32_t id, string_view name) : id(id), name(name) { }

		int32_t id;
		string name;
		vector<fg_file> files;
	};

	vector<fg> filegroups;

	{
		tds::query sq(tds, R"(SELECT filegroups.data_space_id, filegroups.name, database_files.name, database_files.physical_name, database_files.size, database_files.max_size, database_files.growth, database_files.is_percent_growth
FROM sys.filegroups
JOIN sys.database_files ON database_files.data_space_id = filegroups.data_space_id
ORDER BY filegroups.data_space_id, database_files.file_id)");

		while (sq.fetch_row()) {
			auto id = (int32_t)sq[0];

			if (filegroups.empty() || filegroups.back().id != id)
				filegroups.emplace_back(id, (string)sq[1]);

			filegroups.back().files.emplace_back((string)sq[2], (string)sq[3], (int32_t)sq[4], (int32_t)sq[5], (int32_t)sq[6], (unsigned int)sq[7] != 0);
		}
	}

	for (const auto& f : filegroups) {
		string sql;

		sql = "ALTER DATABASE ";
		sql += brackets_escape(tds::utf16_to_utf8(tds.db_name()));
		sql += " ADD FILEGROUP ";
		sql += brackets_escape(f.name);
		sql += ";\n\n";

		sql += "ALTER DATABASE ";
		sql += brackets_escape(tds::utf16_to_utf8(tds.db_name()));
		sql += " ADD FILE\n";

		bool first = true;
		for (const auto& f2 : f.files) {
			if (!first)
				sql += ",\n";

			sql += "(\n";

			sql += "\tNAME = " + brackets_escape(f2.name) + ",\n";
			sql += "\tFILENAME = " + tds::value{f2.physical_name}.to_literal() + ",\n";
			sql += "\tSIZE = " + pages_to_data_size(f2.size) + ",\n";

			if (f2.max_size == -1)
				sql += "\tMAXSIZE = UNLIMITED,\n";
			else
				sql += "\tMAXSIZE = " + pages_to_data_size(f2.max_size) + ",\n";

			if (f2.is_percent_growth)
				sql += "\tFILEGROWTH = " + to_string(f2.growth) + "%\n";
			else
				sql += "\tFILEGROWTH = " + pages_to_data_size(f2.growth) + "\n";

			sql += ")";

			first = false;
		}

		sql += "\nTO FILEGROUP " + brackets_escape(f.name) + ";\n";

		gu.add_file("filegroups/" + f.name + ".sql", sql);
	}
}

static void dump_log_files(tds::tds& tds, git_update& gu) {
	struct fg_file {
		fg_file(string_view name, string_view physical_name, int32_t size, int32_t max_size, int32_t growth, bool is_percent_growth) :
			name(name), physical_name(physical_name), size(size), max_size(max_size), growth(growth),
			is_percent_growth(is_percent_growth) {
		}

		string name;
		string physical_name;
		int32_t size;
		int32_t max_size;
		int32_t growth;
		bool is_percent_growth;
	};

	vector<fg_file> log_files;

	{
		tds::query sq(tds, R"(SELECT name, physical_name, size, max_size, growth, is_percent_growth
FROM sys.database_files
WHERE data_space_id = 0)");

		while (sq.fetch_row()) {
			log_files.emplace_back((string)sq[0], (string)sq[1], (int32_t)sq[2], (int32_t)sq[3], (int32_t)sq[4], (unsigned int)sq[5] != 0);
		}
	}

	for (const auto& l : log_files) {
		string sql;

		sql = "ALTER DATABASE ";
		sql += brackets_escape(tds::utf16_to_utf8(tds.db_name()));
		sql += " ADD LOG FILE (\n";

		sql += "\tNAME = " + brackets_escape(l.name) + ",\n";
		sql += "\tFILENAME = " + tds::value{l.physical_name}.to_literal() + ",\n";
		sql += "\tSIZE = " + pages_to_data_size(l.size) + ",\n";

		if (l.max_size == -1)
			sql += "\tMAXSIZE = UNLIMITED,\n";
		else
			sql += "\tMAXSIZE = " + pages_to_data_size(l.max_size) + ",\n";

		if (l.is_percent_growth)
			sql += "\tFILEGROWTH = " + to_string(l.growth) + "%\n";
		else
			sql += "\tFILEGROWTH = " + pages_to_data_size(l.growth) + "\n";

		sql += ");\n";

		gu.add_file("log_files/" + l.name + ".sql", sql);
	}
}

static void dump_options(tds::tds& tds, git_update& gu) {
	auto j = json::object();

	{
		tds::batch sq(tds, "SELECT compatibility_level, collation_name, user_access, is_read_only, is_auto_close_on, is_auto_shrink_on, is_supplemental_logging_enabled, snapshot_isolation_state, is_read_committed_snapshot_on, recovery_model, page_verify_option, is_auto_create_stats_on, is_auto_create_stats_incremental_on, is_auto_update_stats_on, is_auto_update_stats_async_on, is_ansi_null_default_on, is_ansi_nulls_on, is_ansi_padding_on, is_ansi_warnings_on, is_arithabort_on, is_concat_null_yields_null_on, is_numeric_roundabort_on, is_quoted_identifier_on, is_recursive_triggers_on, is_cursor_close_on_commit_on, is_local_cursor_default, is_fulltext_enabled, is_trustworthy_on, is_db_chaining_on, is_parameterization_forced, is_master_key_encrypted_by_server, is_query_store_on, is_published, is_subscribed, is_merge_published, is_distributor, is_sync_with_backup, is_broker_enabled, is_date_correlation_on, is_cdc_enabled, is_encrypted, is_honor_broker_priority_on, containment, is_memory_optimized_elevate_to_snapshot_on FROM sys.databases WHERE database_id = DB_ID()");

		if (!sq.fetch_row())
			throw runtime_error("Unable to dump database options.");

		// FIXME - string descriptions

		j["compatibility_level"] = (unsigned int)sq[0];
		j["collation_name"] = (string)sq[1];
		j["user_access"] = (unsigned int)sq[2];
		j["read_only"] = (unsigned int)sq[3] != 0;
		j["auto_close"] = (unsigned int)sq[4] != 0;
		j["auto_shrink"] = (unsigned int)sq[5] != 0;
		j["supplemental_logging_enabled"] = (unsigned int)sq[6] != 0;
		j["snapshot_isolation_state"] = (unsigned int)sq[7];
		j["read_committed_snapshot"] = (unsigned int)sq[8] != 0;
		j["recovery_model"] = (unsigned int)sq[9];
		j["page_verify_option"] = (unsigned int)sq[10];
		j["auto_create_stats"] = (unsigned int)sq[11] != 0;
		j["auto_create_stats_incremental"] = (unsigned int)sq[12] != 0;
		j["auto_update_stats"] = (unsigned int)sq[13] != 0;
		j["auto_update_stats_async"] = (unsigned int)sq[14] != 0;
		j["ansi_null_default"] = (unsigned int)sq[15] != 0;
		j["ansi_nulls"] = (unsigned int)sq[16] != 0;
		j["ansi_padding"] = (unsigned int)sq[17] != 0;
		j["ansi_warnings"] = (unsigned int)sq[18] != 0;
		j["arithabort"] = (unsigned int)sq[19] != 0;
		j["concat_null_yield_null"] = (unsigned int)sq[20] != 0;
		j["numeric_roundabort"] = (unsigned int)sq[21] != 0;
		j["quoted_identifier"] = (unsigned int)sq[22] != 0;
		j["recursive_triggers"] = (unsigned int)sq[23] != 0;
		j["cursor_close_on_commit"] = (unsigned int)sq[24] != 0;
		j["local_cursor_default"] = (unsigned int)sq[25] != 0;
		j["fulltext_enabled"] = (unsigned int)sq[26] != 0;
		j["trustworthy"] = (unsigned int)sq[27] != 0;
		j["db_chaining"] = (unsigned int)sq[28] != 0;
		j["parameterization_forced"] = (unsigned int)sq[29] != 0;
		j["master_key_encrypted_by_server"] = (unsigned int)sq[30] != 0;
		j["query_store"] = (unsigned int)sq[31] != 0;
		j["published"] = (unsigned int)sq[32] != 0;
		j["subscribed"] = (unsigned int)sq[33] != 0;
		j["merge_published"] = (unsigned int)sq[34] != 0;
		j["distributor"] = (unsigned int)sq[35] != 0;
		j["sync_with_backup"] = (unsigned int)sq[36] != 0;
		j["broker_enabled"] = (unsigned int)sq[37] != 0;
		j["date_correlation"] = (unsigned int)sq[38] != 0;
		j["cdc_enabled"] = (unsigned int)sq[39] != 0;
		j["encrypted"] = (unsigned int)sq[40] != 0;
		j["honour_broker_priority"] = (unsigned int)sq[41] != 0;
		j["containment"] = (unsigned int)sq[42];
		j["memory_optimized_elevate_to_snapshot"] = (unsigned int)sq[43] != 0;
	}

	auto sc = json::object();

	{
		tds::batch sq(tds, "SELECT name, value FROM sys.database_scoped_configurations");

		while (sq.fetch_row()) {
			sc[(string)sq[0]] = sq[1];
		}
	}

	j["scoped_configurations"] = sc;

	gu.add_file("options.json", j.dump(3) + "\n");
}

static void debracket(string_view& sv) {
	if (sv.empty())
		return;

	if (sv.front() != '[' || sv.back() != ']')
		return;

	if (sv.find(']') != sv.size() - 1)
		return;

	sv = sv.substr(1, sv.size() - 2);
}

static string synonym_ddl(tds::tds& tds, int64_t object_id, string_view schema, string_view name) {
	string base_object_name;

	{
		tds::query sq(tds, "SELECT base_object_name FROM sys.synonyms WHERE object_id = ?", object_id);

		if (!sq.fetch_row())
			throw formatted_error("Could not find base object name for SYNONYM {}.", object_id);

		base_object_name = (string)sq[0];
	}

	auto onp = tds::parse_object_name(base_object_name);

	debracket(onp.server);
	debracket(onp.db);
	debracket(onp.schema);
	debracket(onp.name);

	base_object_name = brackets_escape(onp.schema) + "." + brackets_escape(onp.name);

	if (!onp.db.empty())
		base_object_name = brackets_escape(onp.db) + "." + base_object_name;
	else if (!onp.server.empty())
		base_object_name = "." + base_object_name;

	if (!onp.server.empty())
		base_object_name = brackets_escape(onp.server) + "." + base_object_name;

	return "CREATE SYNONYM " + brackets_escape(schema) + "." + brackets_escape(name) + " FOR " + base_object_name + ";";
}

void do_dump_sql(tds::tds& tds, git_update& gu) {
	vector<sql_obj> objs;

	{
		tds::query sq(tds, R"(SELECT schemas.name,
	COALESCE(table_types.name, objects.name),
	sql_modules.definition,
	RTRIM(objects.type),
	objects.object_id,
	CASE WHEN EXISTS (SELECT * FROM sys.database_permissions WHERE class_desc = 'OBJECT_OR_COLUMN' AND major_id = objects.object_id) THEN 1 ELSE 0 END,
	sql_modules.uses_quoted_identifier
FROM sys.objects
LEFT JOIN sys.sql_modules ON sql_modules.object_id = objects.object_id
LEFT JOIN sys.table_types ON objects.type = 'TT' AND table_types.type_table_object_id = objects.object_id
JOIN sys.schemas ON schemas.schema_id = COALESCE(table_types.schema_id, objects.schema_id)
LEFT JOIN sys.extended_properties ON extended_properties.major_id = objects.object_id AND extended_properties.minor_id = 0 AND extended_properties.name = 'microsoft_database_tools_support'
WHERE objects.type IN ('V','P','FN','TF','IF','U','TT','SN') AND
	(extended_properties.value IS NULL OR extended_properties.value != 1)
ORDER BY schemas.name, objects.name)");

		while (sq.fetch_row()) {
			objs.emplace_back((string)sq[0], (string)sq[1], (string)sq[2], (string)sq[3], (int64_t)sq[4],
							  (unsigned int)sq[5] != 0, (unsigned int)sq[6] != 0);
		}
	}

	{
		tds::query sq(tds, "SELECT triggers.name, sql_modules.definition FROM sys.triggers LEFT JOIN sys.sql_modules ON sql_modules.object_id=triggers.object_id WHERE triggers.parent_class_desc = 'DATABASE'");

		while (sq.fetch_row()) {
			objs.emplace_back("db_triggers", (string)sq[0], (string)sq[1]);
		}
	}

	{
		vector<string> schemas;

		{
			tds::query sq(tds, "SELECT name FROM sys.schemas WHERE name != 'sys' AND name != 'INFORMATION_SCHEMA'");

			while (sq.fetch_row()) {
				schemas.emplace_back(sq[0]);
			}
		}

		for (const auto& v : schemas) {
			objs.emplace_back("schemas", (string)v, get_schema_definition(tds, v));
		}
	}

	{
		vector<pair<string, int64_t>> roles;

		{
			tds::query sq(tds, "SELECT name, principal_id FROM sys.database_principals WHERE type = 'R'");

			while (sq.fetch_row()) {
				roles.emplace_back(sq[0], (int64_t)sq[1]);
			}
		}

		for (const auto& r : roles) {
			objs.emplace_back("principals", r.first, get_role_definition(tds, r.first, r.second));
		}
	}

	{
		tds::query sq(tds, R"(SELECT name,
	system_type_id,
	SCHEMA_NAME(schema_id),
	max_length,
	precision,
	scale,
	is_nullable
FROM sys.types
WHERE is_user_defined = 1 AND is_table_type = 0)");

		while (sq.fetch_row()) {
			objs.emplace_back((string)sq[2], (string)sq[0], get_type_definition((string)sq[0], (string)sq[2], (int32_t)sq[1], (int16_t)sq[3], (uint8_t)sq[4], (uint8_t)sq[5], (unsigned int)sq[6] != 0), "T");
		}
	}

	for (auto& obj : objs) {
		string filename = sanitize_fn(obj.schema) + "/";

		if (obj.type == "V")
			filename += "views/";
		else if (obj.type == "P")
			filename += "procedures/";
		else if (obj.type == "FN" || obj.type == "TF" || obj.type == "IF")
			filename += "functions/";
		else if (obj.type == "U")
			filename += "tables/";
		else if (obj.type == "TT" || obj.type == "T")
			filename += "types/";
		else if (obj.type == "SN")
			filename += "synonyms/";

		if (obj.type == "U" || obj.type == "TT")
			obj.def = table_ddl(tds, obj.id, false);
		else if (obj.type == "V")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::VIEW);
		else if (obj.type == "P")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::PROCEDURE);
		else if (obj.type == "FN" || obj.type == "TF" || obj.type == "IF")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::FUNCTION);
		else if (obj.type == "SN")
			obj.def = synonym_ddl(tds, obj.id, obj.schema, obj.name);

		obj.def = fix_whitespace(obj.def);

		if (!obj.def.empty() && obj.def[0] == '\n') {
			auto pos = obj.def.find_first_not_of("\n");

			if (pos == string::npos)
				obj.def.clear();
			else
				obj.def = obj.def.substr(pos);
		}

		while (!obj.def.empty() && (obj.def.back() == '\n' || obj.def.back() == ' ')) {
			obj.def.pop_back();
		}

		obj.def += "\n";

		if (obj.has_perms)
			obj.def += object_perms(tds, obj.id, brackets_escape(obj.schema) + "." + brackets_escape(obj.name));

		if (obj.type == "P" && !obj.quoted_identifier)
			obj.def = "SET QUOTED_IDENTIFIER OFF;\nGO\n\n" + obj.def;

		filename += sanitize_fn(obj.name) + ".sql";

		gu.add_file(filename, obj.def);
	}

	dump_partition_functions(tds, gu);
	dump_partition_schemes(tds, gu);
	dump_filegroups(tds, gu);
	dump_log_files(tds, gu);
	dump_options(tds, gu);
}

static void dump_sql(tds::tds& tds, const filesystem::path& repo_dir, const string& branch) {
	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);
	git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, 0);

	GitRepo repo(repo_dir.string());
	git_update gu(repo);
	gu.start();

	do_dump_sql(tds, gu);

	gu.stop();

	string name, email;

	get_current_user_details(name, email);

	update_git(repo, name, email, "Update", gu.files2, true, nullopt, branch);

	if (!repo.is_bare() && repo.branch_is_head(branch)) {
		git_checkout_options opts;

		if (git_checkout_options_init(&opts, GIT_CHECKOUT_OPTIONS_VERSION))
			throw runtime_error("git_checkout_options_init failed");

		opts.checkout_strategy = GIT_CHECKOUT_FORCE;

		repo.checkout_head(&opts);
	}

	try {
		repo.try_push("refs/heads/" + branch);
	} catch (const exception& e) {
		cerr << e.what() << endl;
	}
}

static void get_user_details(const u16string& username, string& name, string& email) {
#ifdef _WIN32
	array<std::byte, 68> sid;
	auto sidlen = (DWORD)sid.size();
	char16_t domain[100];
	DWORD domainlen = sizeof(domain) / sizeof(domain[0]);
	SID_NAME_USE use;
	auto u8username = tds::utf16_to_utf8(username);

	if (!LookupAccountNameW(nullptr, (WCHAR*)username.c_str(), sid.data(), &sidlen,
							(WCHAR*)domain, &domainlen, &use)) {
		name = u8username;
		email = u8username + "@localhost"s;
		return;
	}

	try {
		get_ldap_details_from_sid(sid.data(), name, email);

		if (name.empty())
			name = u8username;
	} catch (...) {
		name = u8username;
		email.clear();
	}

	if (email.empty()) {
		domain[domainlen] = 0;

		auto bs = u8username.find("\\");

		if (bs == string::npos)
			email = u8username + "@"s + tds::utf16_to_utf8(domain);
		else
			email = u8username.substr(bs + 1) + "@"s + tds::utf16_to_utf8(domain);
	}
#else
	auto u8username = tds::utf16_to_utf8(username);

	try {
		get_ldap_details_from_full_name(u8username, name, email);
	} catch (const exception& e) {
		cerr << e.what() << endl;

		name.clear();
		email.clear();
	} catch (...) {
		name.clear();
		email.clear();
	}

	if (name.empty())
		name = u8username;

	if (email.empty())
		email = u8username + "@localhost"; // FIXME - computer name?
#endif
}

static void flush_git(const string& db_server) {
	struct repo {
		repo(unsigned int id, string_view dir, string_view branch) :
			id(id), dir(dir), branch(branch) { }

		unsigned int id;
		string dir;
		string branch;
	};

	vector<repo> repos;

	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);
	git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, 0);

	{
		tds::tds tds(db_server, db_username, db_password, db_app);
		tds.run("SET LOCK_TIMEOUT 0; SET XACT_ABORT ON; DELETE FROM master.dbo.git WHERE (SELECT COUNT(*) FROM master.dbo.git_files WHERE id = Git.id) = 0");

		{
			tds::batch sq(tds, "SELECT git.repo, git_repo.dir, git_repo.branch FROM (SELECT repo FROM master.dbo.git GROUP BY repo) git JOIN master.dbo.git_repo ON git_repo.id = Git.repo");

			while (sq.fetch_row()) {
				repos.emplace_back((unsigned int)sq[0], (string)sq[1], (string)sq[2]);
			}

			if (repos.size() == 0)
				return;
		}
	}

	for (const auto& r : repos) {
		GitRepo repo(r.dir);

		while (true) {
			tds::datetimeoffset dto;
			string name, email, description;
			list<git_file2> files;
			bool clear_all = false;
			list<unsigned int> delete_commits;
			list<unsigned int> delete_files;

			tds::tds tds(db_server, db_username, db_password, db_app);

			tds.run("SET LOCK_TIMEOUT 0; SET XACT_ABORT ON;");

			// FIXME - get this working with git_update thread?

			{
				tds::trans trans(tds);
				tds::query sq(tds, R"(SELECT
	git.id,
	git.username,
	git.description,
	git.dto,
	git_files.file_id,
	git_files.filename,
	git_files.data
FROM master.dbo.git
JOIN master.dbo.git_files ON git_files.id = git.id
WHERE git.id = (SELECT MIN(id) FROM master.dbo.git WHERE repo = ?) OR (git.tran_id = (SELECT tran_id FROM master.dbo.git WHERE id = (SELECT MIN(id) FROM master.dbo.git WHERE repo = ?)) AND git.repo = ?)
ORDER BY git.id
)", r.id, r.id, r.id);

				if (!sq.fetch_row())
					break;

				auto commit = (unsigned int)sq[0];
				auto username = (u16string)sq[1];
				description = (string)sq[2];
				dto = (tds::datetimeoffset)sq[3];

				get_user_details(username, name, email);

				delete_commits.push_back(commit);

				bool merged_trans = false;

				do {
					delete_files.push_back((unsigned int)sq[4]);

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

					if (sq[5].is_null)
						clear_all = true;
					else {
						if (merged_trans) {
							auto fn = (string)sq[5];

							for (auto it = files.begin(); it != files.end(); it++) {
								if (it->filename == fn) {
									files.erase(it);
									break;
								}
							}
						}

						if (!sq[6].is_null)
							files.emplace_back((string)sq[5], repo.blob_create_from_buffer((string)sq[6]));
						else
							files.emplace_back((string)sq[5], nullopt);
					}

					if (!sq.fetch_row())
						break;
				} while (true);
			}

			if (!files.empty() || clear_all)
				update_git(repo, name, email, description, files, clear_all, dto, r.branch);

			if (!delete_commits.empty()) {
				tds::trans trans(tds);

				while (!delete_commits.empty()) {
					tds.run("DELETE FROM master.dbo.git WHERE id=?", delete_commits.front());
					tds.run("DELETE FROM master.dbo.git_files WHERE id=?", delete_commits.front());
					delete_commits.pop_front();
				}

				trans.commit();
			}
		}

		if (!repo.is_bare() && repo.branch_is_head(r.branch.empty() ? "master" : r.branch)) {
			git_checkout_options opts;

			if (git_checkout_options_init(&opts, GIT_CHECKOUT_OPTIONS_VERSION))
				throw runtime_error("git_checkout_options_init failed");

			opts.checkout_strategy = GIT_CHECKOUT_FORCE;

			try {
				repo.checkout_head(&opts);
			} catch (const exception& e) {
				cerr << e.what() << endl;
			}
		}

		try {
			repo.try_push("refs/heads/" + (r.branch.empty() ? "master" : r.branch));
		} catch (const exception& e) {
			cerr << e.what() << endl;
		}
	}
}

class lockfile {
public:
	lockfile() {
#ifdef _WIN32
        h.reset(CreateMutexW(nullptr, false, L"Global\\gitsql_mutant"));

        if (!h || h.get() == INVALID_HANDLE_VALUE)
            throw last_error("CreateMutex", GetLastError());

        auto res = WaitForSingleObject(h.get(), INFINITE);

		if (res == WAIT_FAILED)
			throw last_error("WaitForSingleObject", GetLastError());
        else if (res != WAIT_OBJECT_0 && res != WAIT_ABANDONED)
            throw formatted_error("WaitForSingleObject returned {}", res);
#else
		h.reset(open("/tmp/gitsql_lock", O_CREAT, 0666));

		if (h.get() == -1)
			throw errno_error("open", errno);

		do {
			auto ret = flock(h.get(), LOCK_EX);

			if (ret == -1) {
				if (errno == EINTR)
					continue;

				throw errno_error("flock", errno);
			}

			break;
		} while (true);
#endif
	}

	~lockfile() {
#ifdef _WIN32
        ReleaseMutex(h.get());
#else
		flock(h.get(), LOCK_UN);
#endif
	}

private:
	unique_handle h;
};

static string object_ddl2(tds::tds& tds, string_view type, string_view orig_ddl, int64_t id, u16string_view schema,
						  u16string_view object, bool has_perms, bool nolock) {
	string ddl;

	if (type == "U") // table
		ddl = table_ddl(tds, id, nolock);
	else if (type == "V")
		ddl = munge_definition(orig_ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::VIEW);
	else if (type == "P")
		ddl = munge_definition(orig_ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::PROCEDURE);
	else if (type == "FN" || type == "TF" || type == "IF")
		ddl = munge_definition(orig_ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::FUNCTION);

	ddl = fix_whitespace(ddl);

	if (!ddl.empty() && ddl.front() == '\n') {
		auto pos = ddl.find_first_not_of("\n");

		if (pos == string::npos)
			ddl.clear();
		else
			ddl = ddl.substr(pos);
	}

	while (!ddl.empty() && (ddl.back() == '\n' || ddl.back() == ' ')) {
		ddl.pop_back();
	}

	ddl += "\n";

	if (has_perms)
		ddl += object_perms(tds, id, brackets_escape(tds::utf16_to_utf8(schema)) + "." + brackets_escape(tds::utf16_to_utf8(object)));

	return ddl;
}

static string object_ddl(tds::tds& tds, u16string_view schema, u16string_view object) {
	int64_t id;
	string type, ddl;
	bool has_perms;

	if (!object.empty() && object.front() == u'#') {
		tds::query sq(tds, "SELECT OBJECT_ID(?)", u"tempdb.dbo." + u16string(object));

		if (!sq.fetch_row() || sq[0].is_null)
			throw formatted_error("Could not find ID for temporary table {}.", tds::utf16_to_utf8(object));

		id = (int64_t)sq[0];
		type = "U";
		has_perms = false;
		ddl = "";
	} else {
		tds::query sq(tds, R"(SELECT objects.object_id,
	RTRIM(objects.type),
	CASE WHEN EXISTS (SELECT * FROM sys.database_permissions WHERE class_desc = 'OBJECT_OR_COLUMN' AND major_id = objects.object_id) THEN 1 ELSE 0 END,
	sql_modules.definition
FROM sys.objects
LEFT JOIN sys.sql_modules ON sql_modules.object_id = objects.object_id
WHERE objects.name = ? AND objects.schema_id = SCHEMA_ID(?))", object, schema);

		if (!sq.fetch_row())
			throw formatted_error("Could not find ID for object {}.{}.", tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object));

		id = (int64_t)sq[0];
		type = (string)sq[1];
		has_perms = (unsigned int)sq[2] != 0;
		ddl = (string)sq[3];
	}

	return object_ddl2(tds, type, ddl, id, schema, object, has_perms, false);
}

static string object_ddl_id(tds::tds& tds, int64_t id) {
	string type, ddl;
	u16string schema, name;
	bool has_perms;

	{
		tds::query sq(tds, R"(SELECT objects.name,
	RTRIM(objects.type),
	CASE WHEN EXISTS (SELECT * FROM sys.database_permissions WITH (NOLOCK) WHERE class_desc = 'OBJECT_OR_COLUMN' AND major_id = objects.object_id) THEN 1 ELSE 0 END,
	sql_modules.definition,
	SCHEMA_NAME(objects.schema_id)
FROM sys.objects WITH (NOLOCK)
LEFT JOIN sys.sql_modules WITH (NOLOCK) ON sql_modules.object_id = objects.object_id
WHERE objects.object_id = ?)", id);

		if (!sq.fetch_row())
			throw formatted_error("Could not find ID object ID {}.", id);

		name = (u16string)sq[0];
		type = (string)sq[1];
		has_perms = (unsigned int)sq[2] != 0;
		ddl = (string)sq[3];
		schema = (u16string)sq[4];
	}

	return object_ddl2(tds, type, ddl, id, schema, name, has_perms, true);
}

static void write_object_ddl(tds::tds& tds, u16string_view schema, u16string_view object,
							 const optional<u16string>& bind_token, unsigned int commit_id,
							 u16string_view filename, u16string_view db) {
	u16string old_db;

	if (bind_token.has_value()) {
		tds::rpc r(tds, u"sp_bindsession", tds::utf16_to_utf8(bind_token.value()));

		while (r.fetch_row()) { } // wait for last packet
	}

	if (!db.empty()) {
		old_db = tds.db_name();

		if (db != old_db)
			tds.run(tds::no_check{u"USE " + brackets_escape(db)});
	}

	auto ddl = object_ddl(tds, schema, object);

	if (!db.empty() && db != old_db)
		tds.run(tds::no_check{u"USE " + brackets_escape(old_db)});

	tds.run("INSERT INTO master.dbo.git_files(id, filename, data) VALUES(?, ?, ?)", commit_id, filename, tds::to_bytes(ddl));
}

static void dump_sql2(tds::tds& tds, unsigned int repo_num) {
	string repo_dir, db, server, branch;

	{
		tds::query sq(tds, "SELECT dir, db, server, branch FROM master.dbo.git_repo WHERE id = ?", repo_num);

		if (!sq.fetch_row())
			throw formatted_error("Repo {} not found in master.dbo.git_repo.", repo_num);

		repo_dir = (string)sq[0];
		db = (string)sq[1];
		server = (string)sq[2];
		branch = (string)sq[3];
	}

	if (server.empty()) {
		auto old_db = tds::utf16_to_utf8(tds.db_name());

		if (db != old_db)
			tds.run(tds::no_check{"USE " + brackets_escape(db)});

		dump_sql(tds, repo_dir, branch.empty() ? "master" : branch);

		if (db != old_db)
			tds.run(tds::no_check{"USE " + brackets_escape(old_db)});
	} else {
		tds::tds tds2(server, db_username, db_password, db_app);

		tds2.run(tds::no_check{"USE " + brackets_escape(db)});
		dump_sql(tds2, repo_dir, branch.empty() ? "master" : branch);
	}
}

static bool object_exists(tds::tds& tds, string_view name) {
	tds::query sq(tds, "SELECT OBJECT_ID(?)", name);

	if (!sq.fetch_row())
		throw formatted_error("Could not check whether {} exists.", name);

	return !sq[0].is_null;
}

static string prompt_str(string_view msg) {
#ifdef _WIN32
	auto con = GetStdHandle(STD_INPUT_HANDLE);
	char16_t s[255];
	DWORD mode, read;
#else
	char* buf = nullptr;
	size_t n;
	string s;
#endif

	cout << msg << " ";

#ifdef _WIN32
	if (!GetConsoleMode(con, &mode))
		throw last_error("GetConsoleMode", GetLastError());

	if (!ReadConsoleW(con, s, sizeof(s) / sizeof(char16_t), &read, nullptr))
		throw last_error("ReadConsole", GetLastError());

	auto sv = u16string_view(s, read);

	while (!sv.empty() && (sv.back() == u'\r' || sv.back() == u'\n')) {
		sv.remove_suffix(1);
	}

	return tds::utf16_to_utf8(sv);
#else
	auto ret = getline(&buf, &n, stdin);

	if (ret == -1) {
		free(buf);
		throw errno_error("getline", errno);
	}

	s.assign(buf, ret);
	free(buf);

	while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) {
		s.pop_back();
	}

	return s;
#endif
}

static void install_trigger(tds::tds& tds, string_view db, const filesystem::path& exe,
							unsigned int repo_num) {
	auto escaped_exe = tds::value{exe.string()}.to_literal();

	tds.run(tds::no_check{"USE " + brackets_escape(db)});

	tds.run(tds::no_check{R"(
CREATE OR ALTER TRIGGER git_trigger ON DATABASE
AFTER CREATE_FUNCTION, CREATE_PROCEDURE, CREATE_TABLE, CREATE_VIEW, ALTER_FUNCTION, ALTER_PROCEDURE, ALTER_TABLE, ALTER_VIEW, DROP_FUNCTION, DROP_PROCEDURE, DROP_TABLE, DROP_VIEW, CREATE_INDEX, DROP_INDEX, CREATE_TRIGGER, ALTER_TRIGGER, DROP_TRIGGER, RENAME
AS
BEGIN

SET NOCOUNT ON;

DECLARE @repo INT = )"s + to_string(repo_num) + R"(;

DECLARE @type NVARCHAR(100), @tbl NVARCHAR(100), @schema NVARCHAR(100), @idx NVARCHAR(100), @login VARCHAR(255), @id INT;
DECLARE @args NVARCHAR(255), @dir NVARCHAR(20), @msg NVARCHAR(255), @dbname NVARCHAR(100), @objtype NVARCHAR(20), @oldname NVARCHAR(100);
DECLARE @ret INT;
DECLARE @idtbl TABLE(id INT);

SELECT @type = EVENTDATA().value('(/EVENT_INSTANCE/EventType)[1]','nvarchar(100)');
SELECT @tbl = EVENTDATA().value('(/EVENT_INSTANCE/ObjectName)[1]','nvarchar(100)');
SELECT @schema = EVENTDATA().value('(/EVENT_INSTANCE/SchemaName)[1]','nvarchar(100)');
SELECT @login = EVENTDATA().value('(/EVENT_INSTANCE/LoginName)[1]','nvarchar(100)');
SELECT @dbname = EVENTDATA().value('(/EVENT_INSTANCE/DatabaseName)[1]','nvarchar(100)');

IF @type = N'CREATE_INDEX' OR @type = N'DROP_INDEX' OR @type = N'CREATE_TRIGGER' OR @type = N'ALTER_TRIGGER' OR @type = N'DROP_TRIGGER'
BEGIN
	IF EVENTDATA().value('(/EVENT_INSTANCE/TargetObjectType)[1]','nvarchar(100)') != N'TABLE'
		RETURN;

	SET @idx = @tbl;
	SET @tbl = EVENTDATA().value('(/EVENT_INSTANCE/TargetObjectName)[1]','nvarchar(100)');
END;

IF @type = N'RENAME'
BEGIN
	SET @objtype = EVENTDATA().value('(/EVENT_INSTANCE/ObjectType)[1]','nvarchar(100)');

	IF @objtype != 'TABLE' AND @objtype != 'VIEW' AND @objtype != 'FUNCTION' AND @objtype != 'PROCEDURE' AND @objtype != 'INDEX'
		RETURN;

	IF @objtype = 'INDEX'
	BEGIN
		SET @oldname = @tbl;
		SET @idx = EVENTDATA().value('(/EVENT_INSTANCE/NewObjectName)[1]','nvarchar(100)');
		SET @tbl = EVENTDATA().value('(/EVENT_INSTANCE/TargetObjectName)[1]','nvarchar(100)');
	END
	ELSE
	BEGIN
		SET @oldname = @tbl;
		SET @tbl = EVENTDATA().value('(/EVENT_INSTANCE/NewObjectName)[1]','nvarchar(100)');
	END;
END;

IF @type = N'CREATE_FUNCTION'
BEGIN
	SET @dir = 'functions';
	SET @msg = N'CREATE FUNCTION ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'CREATE_PROCEDURE'
BEGIN
	SET @dir = 'procedures';
	SET @msg = N'CREATE PROCEDURE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'CREATE_TABLE'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'CREATE TABLE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'CREATE_VIEW'
BEGIN
	SET @dir = 'views';
	SET @msg = N'CREATE VIEW ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'ALTER_FUNCTION'
BEGIN
	SET @dir = 'functions';
	SET @msg = N'ALTER FUNCTION ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'ALTER_PROCEDURE'
BEGIN
	SET @dir = 'procedures';
	SET @msg = N'ALTER PROCEDURE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'ALTER_TABLE'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'ALTER TABLE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'ALTER_VIEW'
BEGIN
	SET @dir = 'views';
	SET @msg = N'ALTER VIEW ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_FUNCTION'
BEGIN
	SET @dir = 'functions';
	SET @msg = N'DROP FUNCTION ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_PROCEDURE'
BEGIN
	SET @dir = 'procedures';
	SET @msg = N'DROP PROCEDURE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_TABLE'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'DROP TABLE ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_VIEW'
BEGIN
	SET @dir = 'views';
	SET @msg = N'DROP VIEW ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'CREATE_INDEX'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'CREATE INDEX ' + @idx + N' ON ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_INDEX'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'DROP INDEX ' + @idx + N' ON ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'CREATE_TRIGGER'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'CREATE TRIGGER ' + @idx + N' ON ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'ALTER_TRIGGER'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'ALTER TRIGGER ' + @idx + N' ON ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'DROP_TRIGGER'
BEGIN
	SET @dir = 'tables';
	SET @msg = N'DROP TRIGGER ' + @idx + N' ON ' + @schema + N'.' + @tbl;
END
ELSE IF @type = N'RENAME'
BEGIN
	IF @objtype = 'TABLE'
	BEGIN
		SET @dir = 'tables';
		SET @msg = N'RENAME TABLE ' + @schema + N'.' + @oldname + N' TO ' + @tbl;
	END
	ELSE IF @objtype = 'VIEW'
	BEGIN
		SET @dir = 'views';
		SET @msg = N'RENAME VIEW ' + @schema + N'.' + @oldname + N' TO ' + @tbl;
	END
	ELSE IF @objtype = 'FUNCTION'
	BEGIN
		SET @dir = 'functions';
		SET @msg = N'RENAME FUNCTION ' + @schema + N'.' + @oldname + N' TO ' + @tbl;
	END
	ELSE IF @objtype = 'PROCEDURE'
	BEGIN
		SET @dir = 'procedures';
		SET @msg = N'RENAME PROCEDURE ' + @schema + N'.' + @oldname + N' TO ' + @tbl;
	END
	ELSE IF @objtype = 'INDEX'
	BEGIN
		SET @dir = 'tables';
		SET @msg = N'RENAME INDEX ' + @oldname + N' ON ' + @schema + N'.' + @tbl + N' TO ' + @idx;
	END;
END;

BEGIN TRANSACTION;

INSERT INTO master.dbo.git(repo, username, description, dto, tran_id) OUTPUT inserted.id INTO @idtbl VALUES(@repo, @login, @msg, SYSDATETIMEOFFSET(), CURRENT_TRANSACTION_ID());
SET @id = (SELECT id FROM @idtbl);

IF @type = N'DROP_FUNCTION' OR @type = N'DROP_PROCEDURE' OR @type = N'DROP_TABLE' OR @type = N'DROP_VIEW'
	INSERT INTO master.dbo.git_files(id, filename, data) VALUES(@id, @schema + N'/' + @dir + N'/' + @tbl + N'.sql', NULL);
ELSE
BEGIN
	SET @args = N'object "' + @schema + N'" "' + @tbl + N'" ' + CONVERT(NVARCHAR, @id) + N' "' + @schema + N'/' + @dir + N'/' + @tbl + N'.sql" ' + @dbname;
	EXEC @ret = master.dbo.xp_cmd )" + escaped_exe + R"(, @args;

	IF @ret != 0
		THROW 50000, 'GitSQL failed.', 1;

	IF @type = N'RENAME' AND @objtype != N'INDEX'
		INSERT INTO master.dbo.git_files(id, filename, data) VALUES(@id, @schema + N'/' + @dir + N'/' + @oldname + N'.sql', NULL);
END;

COMMIT;

END;)"s});

	tds.run("ENABLE TRIGGER git_trigger ON DATABASE");
}

static filesystem::path get_exe_path() {
#ifdef _WIN32
	WCHAR path[MAX_PATH];

	if (GetModuleFileNameW(nullptr, path, sizeof(path) / sizeof(WCHAR)) == 0)
		throw last_error("GetModuleFileName", GetLastError());

	return path;
#else
	char buf[PATH_MAX];

	auto ret = readlink("/proc/self/exe", buf, sizeof(buf));
	if (ret < 0)
		throw errno_error("readlink", errno);

	return string(buf, buf + ret);
#endif
}

static void install(tds::tds& tds) {
	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);
	git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, 0);

	tds.run("USE master");

	bool has_xp_cmd;

	{
		tds::query sq(tds, "SELECT object_id FROM master.sys.extended_procedures WHERE name = ?", "xp_cmd");

		has_xp_cmd = sq.fetch_row();
	}

	if (has_xp_cmd)
		cout << "Extended stored procedure xp_cmd already installed.\n";
	else if (!filesystem::exists("xp_cmd.dll"))
		cerr << "Not installing extended stored procedure as xp_cmd.dll not found.\n";
	else {
		auto dll = filesystem::absolute(filesystem::path("xp_cmd.dll"));

		cout << "Installing extended stored procedure xp_cmd.\n";

		tds::rpc r(tds, "sp_addextendedproc", "xp_cmd", dll.string());

		while (r.fetch_row()) {
		}
	}

	if (!object_exists(tds, "dbo.git_repo")) {
		cout << "Creating table master.dbo.git_repo.\n";

		tds.run(R"(CREATE TABLE dbo.git_repo (
	id INT IDENTITY NOT NULL PRIMARY KEY,
	dir VARCHAR(260) NOT NULL,
	db VARCHAR(255) NULL,
	branch VARCHAR(50) NULL,
	server VARCHAR(15) NULL
);)");
	} else
		cout << "Table master.dbo.git_repo already exists.\n";

	if (!object_exists(tds, "dbo.git")) {
		cout << "Creating table master.dbo.git.\n";

		tds.run(R"(CREATE TABLE dbo.git (
	id INT IDENTITY NOT NULL PRIMARY KEY,
	repo INT NOT NULL FOREIGN KEY REFERENCES dbo.git_repo(id),
	username NVARCHAR(MAX) NOT NULL,
	description VARCHAR(MAX) NOT NULL,
	dto DATETIMEOFFSET(0) NOT NULL,
	tran_id BIGINT NULL
);)");
	} else
		cout << "Table master.dbo.git already exists.\n";

	if (!object_exists(tds, "dbo.git_files")) {
		cout << "Creating table master.dbo.git_files.\n";

		tds.run(R"(CREATE TABLE dbo.git_files (
	file_id INT IDENTITY NOT NULL PRIMARY KEY,
	id INT NOT NULL FOREIGN KEY REFERENCES dbo.git(id),
	filename VARCHAR(260),
	data VARBINARY(MAX) NULL
);)");
	} else
		cout << "Table master.dbo.git_files already exists.\n";

	cout << "Granting INSERT permissions on dbo.git to public.\n";
	tds.run("GRANT INSERT ON dbo.git TO public");

	cout << "Granting INSERT permissions on dbo.git_files to public.\n";
	tds.run("GRANT INSERT ON dbo.git_files TO public");

	vector<string> dbs;

	{
		tds::query sq(tds, "SELECT name FROM master.sys.databases WHERE name NOT IN ('tempdb', 'master') ORDER BY name");

		while (sq.fetch_row()) {
			dbs.emplace_back((string)sq[0]);
		}
	}

	if (!dbs.empty()) {
		string db_string;

		for (const auto& s : dbs) {
			if (!db_string.empty())
				db_string += ", ";

			db_string += s;
		}

		while (true) {
			cout << endl;
			auto db = prompt_str("Add to database? Choices are " + db_string + ". Press Enter to skip.");

			if (db.empty())
				break;

			bool found = false;
			for (const auto& s : dbs) {
				if (s == db) {
					found = true;
					break;
				}
			}

			if (!found) {
				cerr << format("Database {} not found.\n", db);
				continue;
			}

			optional<unsigned int> repo_num;

			{
				tds::query sq(tds, "SELECT id FROM master.dbo.git_repo WHERE db = ? AND server IS NULL", db);

				if (sq.fetch_row())
					repo_num = (unsigned int)sq[0];
			}

			bool do_dump = false;

			if (repo_num.has_value())
				cout << format("Repository {} already set up for database {}.\n", repo_num.value(), db);
			else {
				cout << format("Setting up repository for database {}.\n", db);

				string path;

				while (true) {
					path = prompt_str("Repository path:");

					if (path.empty())
						continue;

					try {
						if (filesystem::exists(path))
							cout << format("Directory {} already exists.\n", path);
						else {
							cout << format("Creating directory {}.\n", path);

							filesystem::create_directories(path);
						}
					} catch (const exception& e) {
						cerr << e.what() << endl;
						continue;
					}

					break;
				}

				// FIXME - ask if it should be bare?

				git_repository_ptr repo;

				auto ret = git_repository_open(out_ptr(repo), path.c_str());

				if (ret == GIT_OK)
					cout << "Repository already exists.\n";
				else if (ret != GIT_ENOTFOUND)
					throw formatted_error("git_repository_open returned {}", ret);
				else {
					cout << format("Creating new repository in {}.\n", path);

					ret = git_repository_init(out_ptr(repo), path.c_str(), false);

					if (ret)
						throw formatted_error("git_repository_open returned {}", ret);
				}

				optional<string> branchv;

				auto branch = prompt_str("Branch name (leave blank for master):");

				if (!branch.empty())
					branchv = branch;

				{
					tds::query sq(tds, "INSERT INTO master.dbo.git_repo(dir, db, branch) OUTPUT inserted.id VALUES(?, ?, ?)", path, db, branchv);

					if (!sq.fetch_row())
						throw runtime_error("Could not get ID of new repository.");

					repo_num = (unsigned int)sq[0];
				}

				cout << format("Created repository {} for {} in {}.\n", repo_num.value(), db, path);

				do_dump = true;
			}

			cout << "Installing trigger.\n";
			install_trigger(tds, db, get_exe_path(), repo_num.value());

			if (do_dump) {
				cout << "Doing initial dump.\n";
				dump_sql2(tds, repo_num.value());
			}
		}
	}

	// FIXME - set up SQL Agent jobs
}

#ifdef _WIN32

static optional<u16string> get_environment_variable(const u16string& name) {
	auto len = GetEnvironmentVariableW((WCHAR*)name.c_str(), nullptr, 0);

	if (len == 0) {
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
			return nullopt;

		return u"";
	}

	u16string ret(len, 0);

	if (GetEnvironmentVariableW((WCHAR*)name.c_str(), (WCHAR*)ret.data(), len) == 0)
		throw last_error("GetEnvironmentVariableW", GetLastError());

	while (!ret.empty() && ret.back() == 0) {
		ret.pop_back();
	}

	return ret;
}

#else

static optional<string> get_environment_variable(const string& name) {
	auto ret = getenv(name.c_str());

	if (!ret)
		return nullopt;

	return ret;
}

#endif

static void print_usage() {
	cerr << R"(Usage:
    gitsql flush
    gitsql object <schema> <object> <commit> <filename> [database]
    gitsql dump <repo-id>
    gitsql show <object>
    gitsql show <database> <object id>
    gitsql master <repo> <smk>
    gitsql install <server>
)";
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	string db_server;

	if (argc < 2) {
		print_usage();
		return 1;
	}

#ifdef _WIN32
	auto cmd = tds::utf16_to_utf8((char16_t*)argv[1]);
#else
	string_view cmd = argv[1];
#endif

	if (cmd != "flush" && cmd != "object" && cmd != "dump" && cmd != "show" && cmd != "master" && cmd != "install") {
		print_usage();
		return 1;
	}

	try {
#ifdef _WIN32
		auto db_server_env = get_environment_variable(u"DB_RMTSERVER");

		if (!db_server_env.has_value())
			db_server = "localhost"; // SQL Import Wizard doesn't provide DB_RMTSERVER
		else {
			db_server = tds::utf16_to_utf8(db_server_env.value());

			if (db_server == "(local)") // SQL Agent does this
				db_server = "localhost";
		}

		auto db_username_env = get_environment_variable(u"DB_USERNAME");

		if (db_username_env.has_value())
			db_username = tds::utf16_to_utf8(db_username_env.value());

		auto db_password_env = get_environment_variable(u"DB_PASSWORD");

		if (db_password_env.has_value())
			db_password = tds::utf16_to_utf8(db_password_env.value());
#else
		auto db_server_env = get_environment_variable("DB_RMTSERVER");

		if (!db_server_env.has_value())
			db_server = "localhost"; // SQL Import Wizard doesn't provide DB_RMTSERVER
		else {
			db_server = db_server_env.value();

			if (db_server == "(local)") // SQL Agent does this
				db_server = "localhost";
		}

		auto db_username_env = get_environment_variable("DB_USERNAME");

		if (db_username_env.has_value())
			db_username = db_username_env.value();

		auto db_password_env = get_environment_variable("DB_PASSWORD");

		if (db_password_env.has_value())
			db_password = db_password_env.value();
#endif

		if (cmd == "install") {
			if (argc < 3)
				throw runtime_error("Too few arguments.");

#ifdef _WIN32
			u16string_view u16server = (char16_t*)argv[2];

			auto server = tds::utf16_to_utf8(u16server);
#else
			string_view server = argv[2];
#endif

			tds::tds tds(server, db_username, db_password, db_app);

			install(tds);

			return 0;
		}

		if (cmd == "flush") {
			lockfile lf;

			flush_git(db_server);
		} else if (cmd == "object") {
			if (argc < 6)
				throw runtime_error("Too few arguments.");

			optional<u16string> bind_token;

#ifdef _WIN32
			bind_token = get_environment_variable(u"DB_BIND_TOKEN");
#else
			auto bind_token_u8 = get_environment_variable("DB_BIND_TOKEN");

			if (bind_token_u8.has_value())
				bind_token = tds::utf8_to_utf16(bind_token_u8.value());
#endif

			unsigned int commit_id;

			{
#ifdef _WIN32
				auto commit_id_str = tds::utf16_to_utf8(argv[4]);
#else
				string commit_id_str = argv[4];
#endif

				auto [ptr, ec] = from_chars(commit_id_str.data(), commit_id_str.data() + commit_id_str.length(), commit_id);

				if (ptr != commit_id_str.data() + commit_id_str.length())
					throw formatted_error("Invalid commit ID \"{}\".", commit_id_str);
			}

#ifdef _WIN32
			u16string_view schema = (char16_t*)argv[2];
			u16string_view object = (char16_t*)argv[3];
			u16string_view filename = (char16_t*)argv[5];
			u16string_view db = argc >= 7 ? (char16_t*)argv[6] : u"";
#else
			string_view u8schema = argv[2];
			string_view u8object = argv[3];
			string_view u8filename = argv[5];
			string_view u8db = argc >= 7 ? argv[6] : "";

			auto schema = tds::utf8_to_utf16(u8schema);
			auto object = tds::utf8_to_utf16(u8object);
			auto filename = tds::utf8_to_utf16(u8filename);
			auto db = tds::utf8_to_utf16(u8db);
#endif
			tds::tds tds(db_server, db_username, db_password, db_app);

			write_object_ddl(tds, schema, object, bind_token, commit_id, filename, db);
		} else if (cmd == "dump") {
			unsigned int repo_id;

			{
#ifdef _WIN32
				auto repo_id_str = tds::utf16_to_utf8(argv[2]);
#else
				string repo_id_str = argv[2];
#endif

				auto [ptr, ec] = from_chars(repo_id_str.data(), repo_id_str.data() + repo_id_str.length(), repo_id);

				if (ptr != repo_id_str.data() + repo_id_str.length())
					throw formatted_error("Invalid repository ID \"{}\".", repo_id_str);
			}

			lockfile lf;
			tds::tds tds(db_server, db_username, db_password, db_app);

			dump_sql2(tds, repo_id);
		} else if (cmd == "show") {
			if (argc < 3)
				throw runtime_error("Too few arguments.");

			optional<u16string> bind_token;

#ifdef _WIN32
			bind_token = get_environment_variable(u"DB_BIND_TOKEN");
#else
			auto bind_token_u8 = get_environment_variable("DB_BIND_TOKEN");

			if (bind_token_u8.has_value())
				bind_token = tds::utf8_to_utf16(bind_token_u8.value());
#endif

			tds::tds tds(db_server, db_username, db_password, db_app);

			if (bind_token.has_value()) {
				tds::rpc r(tds, u"sp_bindsession", tds::utf16_to_utf8(bind_token.value()));

				while (r.fetch_row()) { } // wait for last packet
			}

			if (argc >= 4) {
				int32_t id;

#ifdef _WIN32
				u16string_view db = (char16_t*)argv[2];
#else
				string_view u8db = argv[2];
				auto db = tds::utf8_to_utf16(u8db);
#endif

#ifdef _WIN32
				auto id_str = tds::utf16_to_utf8(argv[3]);
#else
				string id_str = argv[3];
#endif

				auto [ptr, ec] = from_chars(id_str.data(), id_str.data() + id_str.length(), id);

				if (ptr != id_str.data() + id_str.length())
					throw formatted_error("Invalid object ID \"{}\".", id_str);

				if (!db.empty())
					tds.run(tds::no_check{u"USE " + brackets_escape(db)});

				auto ddl = object_ddl_id(tds, id);

				cout << ddl;
			} else {
#ifdef _WIN32
				u16string_view object = (char16_t*)argv[2];
#else
				string_view u8object = argv[2];
				auto object = tds::utf8_to_utf16(u8object);
#endif

				auto onp = tds::parse_object_name(object);

				if (!onp.server.empty())
					throw runtime_error("Cannot show definition of objects on remote servers.");

				if (!onp.db.empty())
					tds.run(tds::no_check{u"USE " + brackets_escape(onp.db)});
				else if (!onp.name.empty() && onp.name.front() == u'#')
					tds.run(u"USE tempdb");

				auto ddl = object_ddl(tds, onp.schema, onp.name);

				cout << ddl;
			}
		} else if (cmd == "master") {
			unsigned int repo;

			if (argc < 4)
				throw runtime_error("Too few arguments.");

#ifdef _WIN32
			auto repo_str = tds::utf16_to_utf8((char16_t*)argv[2]);
#else
			string_view repo_str = argv[2];
#endif
			auto [ptr, ec] = from_chars(repo_str.data(), repo_str.data() + repo_str.length(), repo);

			if (ptr != repo_str.data() + repo_str.length())
				throw formatted_error("Unable to interpret \"{}\" as integer.", repo_str);

#ifdef _WIN32
			auto smk_str = tds::utf16_to_utf8((char16_t*)argv[3]);
#else
			string_view smk_str = argv[3];
#endif

			if (smk_str.size() % 2)
				throw runtime_error("SMK must be even number of characters.");

			vector<std::byte> smk;

			smk.reserve(smk_str.size() / 2);

			for (size_t i = 0; i < smk_str.size(); i += 2) {
				uint8_t b;

				auto [ptr, ec] = from_chars(&smk_str[i], &smk_str[i] + 2, b, 16);

				if (ptr != &smk_str[i] + 2)
					throw runtime_error("Malformed SMK.");

				smk.push_back(std::byte{b});
			}

			dump_master(db_server, repo, smk);
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;

		if (cmd != "show") {
			try {
				tds::tds tds(db_server, db_username, db_password, db_app);
				tds.run("INSERT INTO Sandbox.gitsql(timestamp, message) VALUES(GETDATE(), ?)", string_view(e.what()));
			} catch (...) {
			}
		}

		return 1;
	}

	return 0;
}
