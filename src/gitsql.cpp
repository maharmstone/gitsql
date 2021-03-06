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
#include <tdscpp.h>
#include "git.h"
#include "gitsql.h"

using namespace std;

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
			msg += fmt::format("{}", en);
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

static string grant_string(const vector<sql_perms>& perms, const string& obj) {
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

		ret += " ON " + obj;
		ret += " TO ";
		ret += brackets_escape(p.user);
		ret += ";\n";
	}

	return ret;
}

static string get_schema_definition(tds::tds& tds, const string& name, const string& dbs) {
	vector<sql_perms> perms;
	string ret = "CREATE SCHEMA " + brackets_escape(name) + ";\n";

	{
		tds::query sq(tds, tds::no_check{R"(SELECT database_permissions.state_desc,
	database_permissions.permission_name,
	USER_NAME(database_permissions.grantee_principal_id)
FROM )" + dbs + R"(sys.database_permissions
JOIN )" + dbs + R"(sys.database_principals ON database_principals.principal_id = database_permissions.grantee_principal_id
WHERE database_permissions.class_desc = 'SCHEMA' AND
	database_permissions.major_id = SCHEMA_ID(?)
ORDER BY USER_NAME(database_permissions.grantee_principal_id),
	database_permissions.state_desc,
	database_permissions.permission_name)"}, name);

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

string object_perms(tds::tds& tds, int64_t id, const string& dbs, const string& name) {
	vector<sql_perms> perms;

	{
		tds::query sq(tds, tds::no_check{R"(SELECT database_permissions.state_desc,
database_permissions.permission_name,
USER_NAME(database_permissions.grantee_principal_id)
FROM )" + dbs + R"(sys.database_permissions
JOIN )" + dbs + R"(sys.database_principals ON database_principals.principal_id = database_permissions.grantee_principal_id
WHERE database_permissions.class_desc = 'OBJECT_OR_COLUMN' AND
	database_permissions.major_id = ?
ORDER BY USER_NAME(database_permissions.grantee_principal_id),
	database_permissions.state_desc,
	database_permissions.permission_name)"}, id);

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

static void get_current_user_details(string& name, string& email) {
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

static void dump_sql(tds::tds& tds, const filesystem::path& repo_dir, const string& db, const string& branch) {
	string dbs;
	list<git_file> files;

	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);

	if (!db.empty())
		dbs = db + ".";

	vector<sql_obj> objs;

	{
		tds::query sq(tds, tds::no_check{R"(SELECT schemas.name,
COALESCE(table_types.name, objects.name),
sql_modules.definition,
RTRIM(objects.type),
objects.object_id,
CASE WHEN EXISTS (SELECT * FROM )" + dbs + R"(sys.database_permissions WHERE class_desc = 'OBJECT_OR_COLUMN' AND major_id = objects.object_id) THEN 1 ELSE 0 END,
sql_modules.uses_quoted_identifier
FROM )" + dbs + R"(sys.objects
LEFT JOIN )" + dbs + R"(sys.sql_modules ON sql_modules.object_id = objects.object_id
LEFT JOIN )" + dbs + R"(sys.table_types ON objects.type = 'TT' AND table_types.type_table_object_id = objects.object_id
JOIN )" + dbs + R"(sys.schemas ON schemas.schema_id = COALESCE(table_types.schema_id, objects.schema_id)
LEFT JOIN )" + dbs + R"(sys.extended_properties ON extended_properties.major_id = objects.object_id AND extended_properties.minor_id = 0 AND extended_properties.name = 'microsoft_database_tools_support'
WHERE objects.type IN ('V','P','FN','TF','IF','U','TT') AND
	(extended_properties.value IS NULL OR extended_properties.value != 1)
ORDER BY schemas.name, objects.name)"});

		while (sq.fetch_row()) {
			objs.emplace_back((string)sq[0], (string)sq[1], (string)sq[2], (string)sq[3], (int64_t)sq[4],
							  (unsigned int)sq[5] != 0, (unsigned int)sq[6] != 0);
		}
	}

	{
		tds::query sq(tds, tds::no_check{"SELECT triggers.name, sql_modules.definition FROM " + dbs + "sys.triggers LEFT JOIN " + dbs + "sys.sql_modules ON sql_modules.object_id=triggers.object_id WHERE triggers.parent_class_desc = 'DATABASE'"});

		while (sq.fetch_row()) {
			objs.emplace_back("db_triggers", (string)sq[0], (string)sq[1]);
		}
	}

	{
		vector<string> schemas;

		{
			tds::query sq(tds, tds::no_check{"SELECT name FROM " + dbs + "sys.schemas WHERE name != 'sys' AND name != 'INFORMATION_SCHEMA'"});

			while (sq.fetch_row()) {
				schemas.emplace_back(sq[0]);
			}
		}

		for (const auto& v : schemas) {
			objs.emplace_back("schemas", (string)v, get_schema_definition(tds, v, dbs));
		}
	}

	{
		vector<pair<string, int64_t>> roles;

		{
			tds::query sq(tds, tds::no_check{"SELECT name, principal_id FROM " + dbs + "sys.database_principals WHERE type = 'R'"});

			while (sq.fetch_row()) {
				roles.emplace_back(sq[0], (int64_t)sq[1]);
			}
		}

		for (const auto& r : roles) {
			objs.emplace_back("principals", r.first, get_role_definition(tds, r.first, r.second));
		}
	}

	{
		tds::query sq(tds, tds::no_check{R"(SELECT name,
system_type_id,
SCHEMA_NAME(schema_id),
max_length,
precision,
scale,
is_nullable
FROM )" + dbs + R"(sys.types
WHERE is_user_defined = 1 AND is_table_type = 0)"});

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

		if (obj.type == "U" || obj.type == "TT")
			obj.def = table_ddl(tds, obj.id);
		else if (obj.type == "V")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::VIEW);
		else if (obj.type == "P")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::PROCEDURE);
		else if (obj.type == "FN" || obj.type == "TF" || obj.type == "IF")
			obj.def = munge_definition(obj.def, obj.schema, obj.name, lex::FUNCTION);

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
			obj.def += object_perms(tds, obj.id, dbs, brackets_escape(obj.schema) + "." + brackets_escape(obj.name));

		if (obj.type == "P" && !obj.quoted_identifier)
			obj.def = "SET QUOTED_IDENTIFIER OFF;\nGO\n\n" + obj.def;

		filename += sanitize_fn(obj.name) + ".sql";

		files.emplace_back(filename, obj.def);
	}

	string name, email;

	get_current_user_details(name, email);

	GitRepo repo(repo_dir.string());

	update_git(repo, name, email, "Update", files, true, nullopt, branch);

	if (!repo.is_bare() && repo.branch_is_head(branch)) {
		git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

		opts.checkout_strategy = GIT_CHECKOUT_FORCE;

		repo.checkout_head(&opts);
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

	if (auto bs = u8username.find("\\"); bs != string::npos)
		u8username = u8username.substr(bs + 1);

	try {
		get_ldap_details_from_name(u8username, name, email);
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

class repo {
public:
	repo(unsigned int id, string_view dir, string_view branch) :
		id(id), dir(dir), branch(branch) { }

	unsigned int id;
	string dir;
	string branch;
};

static void flush_git(const string& db_server) {
	vector<repo> repos;

	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);

	{
		tds::tds tds(db_server, db_username, db_password, db_app);
		tds.run("SET LOCK_TIMEOUT 0; SET XACT_ABORT ON; DELETE FROM Restricted.Git WHERE (SELECT COUNT(*) FROM Restricted.GitFiles WHERE id = Git.id) = 0");

		{
			tds::query sq(tds, "SET LOCK_TIMEOUT 0; SET XACT_ABORT ON; SELECT Git.repo, GitRepo.dir, GitRepo.branch FROM (SELECT repo FROM Restricted.Git GROUP BY repo) Git JOIN Restricted.GitRepo ON GitRepo.id=Git.repo");

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
			list<git_file> files;
			bool clear_all = false;
			list<unsigned int> delete_commits;
			list<unsigned int> delete_files;

			tds::tds tds(db_server, db_username, db_password, db_app);

			{
				tds::trans trans(tds);
				tds::query sq(tds, R"(
SET LOCK_TIMEOUT 0;
SET XACT_ABORT ON;

SELECT
	Git.id,
	Git.username,
	Git.description,
	Git.dto,
	GitFiles.file_id,
	GitFiles.filename,
	GitFiles.data
FROM Restricted.Git
JOIN Restricted.GitFiles ON GitFiles.id = Git.id
WHERE Git.id = (SELECT MIN(id) FROM Restricted.Git WHERE repo = ?) OR (Git.tran_id = (SELECT tran_id FROM Restricted.Git WHERE id = (SELECT MIN(id) FROM Restricted.Git WHERE repo = ?)) AND Git.repo = ?)
ORDER BY Git.id
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

						files.emplace_back((string)sq[5], sq[6]);
					}

					if (!sq.fetch_row())
						break;
				} while (true);
			}

			if (files.size() > 0 || clear_all)
				update_git(repo, name, email, description, files, clear_all, dto, r.branch);

			if (!delete_commits.empty()) {
				tds::trans trans(tds);

				while (!delete_commits.empty()) {
					tds.run("DELETE FROM Restricted.Git WHERE id=?", delete_commits.front());
					tds.run("DELETE FROM Restricted.GitFiles WHERE id=?", delete_commits.front());
					delete_commits.pop_front();
				}

				trans.commit();
			}
		}

		if (!repo.is_bare() && repo.branch_is_head(r.branch.empty() ? "master" : r.branch)) {
			git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

			opts.checkout_strategy = GIT_CHECKOUT_FORCE;

			repo.checkout_head(&opts);
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
        else if (res != WAIT_OBJECT_0)
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

	if (type == "U") // table
		ddl = table_ddl(tds, id);
	else if (type == "V")
		ddl = munge_definition(ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::VIEW);
	else if (type == "P")
		ddl = munge_definition(ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::PROCEDURE);
	else if (type == "FN" || type == "TF" || type == "IF")
		ddl = munge_definition(ddl, tds::utf16_to_utf8(schema), tds::utf16_to_utf8(object), lex::FUNCTION);

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
		ddl += object_perms(tds, id, "", brackets_escape(tds::utf16_to_utf8(schema)) + "." + brackets_escape(tds::utf16_to_utf8(object)));

	return ddl;
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

	tds.run("INSERT INTO Restricted.GitFiles(id, filename, data) VALUES(?, ?, ?)", commit_id, filename, tds::to_bytes(ddl));
}

static void dump_sql2(tds::tds& tds, unsigned int repo_num) {
	string repo_dir, db, server, branch;

	{
		tds::query sq(tds, "SELECT dir, db, server, branch FROM Restricted.GitRepo WHERE id = ?", repo_num);

		if (!sq.fetch_row())
			throw formatted_error("Repo {} not found in Restricted.GitRepo.", repo_num);

		repo_dir = (string)sq[0];
		db = (string)sq[1];
		server = (string)sq[2];
		branch = (string)sq[3];
	}

	if (server.empty()) {
		auto old_db = tds::utf16_to_utf8(tds.db_name());

		if (db != old_db)
			tds.run(tds::no_check{"USE " + brackets_escape(db)});

		dump_sql(tds, repo_dir, db, branch.empty() ? "master" : branch);

		if (db != old_db)
			tds.run(tds::no_check{"USE " + brackets_escape(old_db)});
	} else {
		tds::tds tds2(server, db_username, db_password, db_app);

		tds2.run(tds::no_check{"USE " + brackets_escape(db)});
		dump_sql(tds2, repo_dir, db, branch.empty() ? "master" : branch);
	}
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
	fmt::print(stderr, R"(Usage:
    gitsql flush
    gitsql object <schema> <object> <commit> <filename> [database]
    gitsql dump <repo-id>
    gitsql show <object>
)");
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

	if (cmd != "flush" && cmd != "object" && cmd != "dump" && cmd != "show" && cmd != "master") {
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

			int32_t commit_id;

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
			int32_t repo_id;

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

#ifdef _WIN32
			u16string_view object = (char16_t*)argv[2];
#else
			string_view u8object = argv[2];
			auto object = tds::utf8_to_utf16(u8object);
#endif
			tds::tds tds(db_server, db_username, db_password, db_app);

			if (bind_token.has_value()) {
				tds::rpc r(tds, u"sp_bindsession", tds::utf16_to_utf8(bind_token.value()));

				while (r.fetch_row()) { } // wait for last packet
			}

			auto onp = tds::parse_object_name(object);

			if (!onp.server.empty())
				throw runtime_error("Cannot show definition of objects on remote servers.");

			if (!onp.db.empty())
				tds.run(tds::no_check{u"USE " + brackets_escape(onp.db)});
			else if (!onp.name.empty() && onp.name.front() == u'#')
				tds.run(u"USE tempdb");

			auto ddl = object_ddl(tds, onp.schema, onp.name);

			fmt::print("{}", ddl);
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
		fmt::print(stderr, "{}\n", e.what());

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
