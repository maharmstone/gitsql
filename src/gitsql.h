#pragma once

#include <windows.h>
#include <string>
#include <span>
#include <tdscpp.h>
#include <fmt/format.h>
#include <fmt/compile.h>

class _formatted_error : public std::exception {
public:
	template<typename T, typename... Args>
	_formatted_error(const T& s, Args&&... args) {
		msg = fmt::format(s, std::forward<Args>(args)...);
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	std::string msg;
};

#define formatted_error(s, ...) _formatted_error(FMT_COMPILE(s), __VA_ARGS__)

class last_error : public std::exception {
public:
	last_error(std::string_view function, int le) {
		std::string nice_msg;

		{
			char16_t* fm;

			if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
							   le, 0, reinterpret_cast<LPWSTR>(&fm), 0, nullptr)) {
				try {
					std::u16string_view s = fm;

					while (!s.empty() && (s[s.length() - 1] == u'\r' || s[s.length() - 1] == u'\n')) {
						s.remove_suffix(1);
					}

					nice_msg = tds::utf16_to_utf8(s);
				} catch (...) {
					LocalFree(fm);
					throw;
				}

				LocalFree(fm);
				}
		}

		msg = std::string(function) + " failed (error " + std::to_string(le) + (!nice_msg.empty() ? (", " + nice_msg) : "") + ").";
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	std::string msg;
};

enum class lex {
	unknown,
	whitespace,
	comment,
	identifier,
	number,
	string,
	open_bracket,
	close_bracket,
	comma,
	semicolon,
	full_stop,
	minus,
	slash,
	equals,
	ADD,
	ALL,
	ALTER,
	AND,
	ANY,
	AS,
	ASC,
	AUTHORIZATION,
	BACKUP,
	BEGIN,
	BETWEEN,
	BREAK,
	BROWSE,
	BULK,
	BY,
	CASCADE,
	CASE,
	CHECK,
	CHECKPOINT,
	CLOSE,
	CLUSTERED,
	COALESCE,
	COLLATE,
	COLUMN,
	COMMIT,
	COMPUTE,
	CONSTRAINT,
	CONTAINS,
	CONTAINSTABLE,
	CONTINUE,
	CONVERT,
	CREATE,
	CROSS,
	CURRENT,
	CURRENT_DATE,
	CURRENT_TIME,
	CURRENT_TIMESTAMP,
	CURRENT_USER,
	CURSOR,
	DATABASE,
	DBCC,
	DEALLOCATE,
	DECLARE,
	DEFAULT,
	SQL_DELETE,
	DENY,
	DESC,
	DISTINCT,
	DISTRIBUTED,
	DOUBLE,
	DROP,
	ELSE,
	END,
	ERRLVL,
	ESCAPE,
	EXCEPT,
	EXEC,
	EXECUTE,
	EXISTS,
	EXIT,
	EXTERNAL,
	FETCH,
	FILE,
	FILLFACTOR,
	FOR,
	FOREIGN,
	FREETEXT,
	FREETEXTTABLE,
	FROM,
	FULL,
	FUNCTION,
	GOTO,
	GRANT,
	GROUP,
	HAVING,
	HOLDLOCK,
	IDENTITY,
	IDENTITY_INSERT,
	IDENTITYCOL,
	IF,
	SQL_IN,
	INDEX,
	INNER,
	INSERT,
	INTERSECT,
	INTO,
	IS,
	JOIN,
	KEY,
	KILL,
	LEFT,
	LIKE,
	LINENO,
	MERGE,
	NATIONAL,
	NOCHECK,
	NONCLUSTERED,
	NOT,
	SQL_NULL,
	NULLIF,
	OF,
	OFF,
	OFFSETS,
	ON,
	OPEN,
	OPENDATASOURCE,
	OPENQUERY,
	OPENROWSET,
	OPENXML,
	OPTION,
	OR,
	ORDER,
	OUTER,
	OVER,
	PERCENT,
	PIVOT,
	PLAN,
	PRIMARY,
	PRINT,
	PROC,
	PROCEDURE,
	PUBLIC,
	RAISERROR,
	READ,
	READTEXT,
	RECONFIGURE,
	REFERENCES,
	REPLICATION,
	RESTORE,
	RESTRICT,
	RETURN,
	REVERT,
	REVOKE,
	RIGHT,
	ROLLBACK,
	ROWCOUNT,
	ROWGUIDCOL,
	RULE,
	SAVE,
	SCHEMA,
	SELECT,
	SEMANTICKEYPHRASETABLE,
	SEMANTICSIMILARITYDETAILSTABLE,
	SEMANTICSIMILARITYTABLE,
	SESSION_USER,
	SET,
	SETUSER,
	SHUTDOWN,
	SOME,
	STATISTICS,
	SYSTEM_USER,
	TABLE,
	TABLESAMPLE,
	TEXTSIZE,
	THEN,
	TO,
	TOP,
	TRAN,
	TRANSACTION,
	TRIGGER,
	TRUNCATE,
	TRY_CONVERT,
	TSEQUAL,
	UNION,
	UNIQUE,
	UNPIVOT,
	UPDATE,
	UPDATETEXT,
	USE,
	USER,
	VALUES,
	VARYING,
	VIEW,
	WAITFOR,
	WHEN,
	WHERE,
	WHILE,
	WITH,
	WRITETEXT
};

template<>
struct fmt::formatter<enum lex> {
	constexpr auto parse(format_parse_context& ctx) {
		auto it = ctx.begin();

		if (it != ctx.end() && *it != '}')
			throw format_error("invalid format");

		return it;
	}

	template<typename format_context>
	auto format(enum lex t, format_context& ctx) const {
		switch (t) {
			case lex::unknown:
				return fmt::format_to(ctx.out(), "unknown");
			case lex::whitespace:
				return fmt::format_to(ctx.out(), "whitespace");
			case lex::comment:
				return fmt::format_to(ctx.out(), "comment");
			case lex::identifier:
				return fmt::format_to(ctx.out(), "identifier");
			case lex::number:
				return fmt::format_to(ctx.out(), "number");
			case lex::string:
				return fmt::format_to(ctx.out(), "string");
			case lex::open_bracket:
				return fmt::format_to(ctx.out(), "open_bracket");
			case lex::close_bracket:
				return fmt::format_to(ctx.out(), "close_bracket");
			case lex::comma:
				return fmt::format_to(ctx.out(), "comma");
			case lex::semicolon:
				return fmt::format_to(ctx.out(), "semicolon");
			case lex::full_stop:
				return fmt::format_to(ctx.out(), "full_stop");
			case lex::minus:
				return fmt::format_to(ctx.out(), "minus");
			case lex::slash:
				return fmt::format_to(ctx.out(), "slash");
			case lex::equals:
				return fmt::format_to(ctx.out(), "equals");
			case lex::ADD:
				return fmt::format_to(ctx.out(), "ADD");
			case lex::ALL:
				return fmt::format_to(ctx.out(), "ALL");
			case lex::ALTER:
				return fmt::format_to(ctx.out(), "ALTER");
			case lex::AND:
				return fmt::format_to(ctx.out(), "AND");
			case lex::ANY:
				return fmt::format_to(ctx.out(), "ANY");
			case lex::AS:
				return fmt::format_to(ctx.out(), "AS");
			case lex::ASC:
				return fmt::format_to(ctx.out(), "ASC");
			case lex::AUTHORIZATION:
				return fmt::format_to(ctx.out(), "AUTHORIZATION");
			case lex::BACKUP:
				return fmt::format_to(ctx.out(), "BACKUP");
			case lex::BEGIN:
				return fmt::format_to(ctx.out(), "BEGIN");
			case lex::BETWEEN:
				return fmt::format_to(ctx.out(), "BETWEEN");
			case lex::BREAK:
				return fmt::format_to(ctx.out(), "BREAK");
			case lex::BROWSE:
				return fmt::format_to(ctx.out(), "BROWSE");
			case lex::BULK:
				return fmt::format_to(ctx.out(), "BULK");
			case lex::BY:
				return fmt::format_to(ctx.out(), "BY");
			case lex::CASCADE:
				return fmt::format_to(ctx.out(), "CASCADE");
			case lex::CASE:
				return fmt::format_to(ctx.out(), "CASE");
			case lex::CHECK:
				return fmt::format_to(ctx.out(), "CHECK");
			case lex::CHECKPOINT:
				return fmt::format_to(ctx.out(), "CHECKPOINT");
			case lex::CLOSE:
				return fmt::format_to(ctx.out(), "CLOSE");
			case lex::CLUSTERED:
				return fmt::format_to(ctx.out(), "CLUSTERED");
			case lex::COALESCE:
				return fmt::format_to(ctx.out(), "COALESCE");
			case lex::COLLATE:
				return fmt::format_to(ctx.out(), "COLLATE");
			case lex::COLUMN:
				return fmt::format_to(ctx.out(), "COLUMN");
			case lex::COMMIT:
				return fmt::format_to(ctx.out(), "COMMIT");
			case lex::COMPUTE:
				return fmt::format_to(ctx.out(), "COMPUTE");
			case lex::CONSTRAINT:
				return fmt::format_to(ctx.out(), "CONSTRAINT");
			case lex::CONTAINS:
				return fmt::format_to(ctx.out(), "CONTAINS");
			case lex::CONTAINSTABLE:
				return fmt::format_to(ctx.out(), "CONTAINSTABLE");
			case lex::CONTINUE:
				return fmt::format_to(ctx.out(), "CONTINUE");
			case lex::CONVERT:
				return fmt::format_to(ctx.out(), "CONVERT");
			case lex::CREATE:
				return fmt::format_to(ctx.out(), "CREATE");
			case lex::CROSS:
				return fmt::format_to(ctx.out(), "CROSS");
			case lex::CURRENT:
				return fmt::format_to(ctx.out(), "CURRENT");
			case lex::CURRENT_DATE:
				return fmt::format_to(ctx.out(), "CURRENT_DATE");
			case lex::CURRENT_TIME:
				return fmt::format_to(ctx.out(), "CURRENT_TIME");
			case lex::CURRENT_TIMESTAMP:
				return fmt::format_to(ctx.out(), "CURRENT_TIMESTAMP");
			case lex::CURRENT_USER:
				return fmt::format_to(ctx.out(), "CURRENT_USER");
			case lex::CURSOR:
				return fmt::format_to(ctx.out(), "CURSOR");
			case lex::DATABASE:
				return fmt::format_to(ctx.out(), "DATABASE");
			case lex::DBCC:
				return fmt::format_to(ctx.out(), "DBCC");
			case lex::DEALLOCATE:
				return fmt::format_to(ctx.out(), "DEALLOCATE");
			case lex::DECLARE:
				return fmt::format_to(ctx.out(), "DECLARE");
			case lex::DEFAULT:
				return fmt::format_to(ctx.out(), "DEFAULT");
			case lex::SQL_DELETE:
				return fmt::format_to(ctx.out(), "SQL_DELETE");
			case lex::DENY:
				return fmt::format_to(ctx.out(), "DENY");
			case lex::DESC:
				return fmt::format_to(ctx.out(), "DESC");
			case lex::DISTINCT:
				return fmt::format_to(ctx.out(), "DISTINCT");
			case lex::DISTRIBUTED:
				return fmt::format_to(ctx.out(), "DISTRIBUTED");
			case lex::DOUBLE:
				return fmt::format_to(ctx.out(), "DOUBLE");
			case lex::DROP:
				return fmt::format_to(ctx.out(), "DROP");
			case lex::ELSE:
				return fmt::format_to(ctx.out(), "ELSE");
			case lex::END:
				return fmt::format_to(ctx.out(), "END");
			case lex::ERRLVL:
				return fmt::format_to(ctx.out(), "ERRLVL");
			case lex::ESCAPE:
				return fmt::format_to(ctx.out(), "ESCAPE");
			case lex::EXCEPT:
				return fmt::format_to(ctx.out(), "EXCEPT");
			case lex::EXEC:
				return fmt::format_to(ctx.out(), "EXEC");
			case lex::EXECUTE:
				return fmt::format_to(ctx.out(), "EXECUTE");
			case lex::EXISTS:
				return fmt::format_to(ctx.out(), "EXISTS");
			case lex::EXIT:
				return fmt::format_to(ctx.out(), "EXIT");
			case lex::EXTERNAL:
				return fmt::format_to(ctx.out(), "EXTERNAL");
			case lex::FETCH:
				return fmt::format_to(ctx.out(), "FETCH");
			case lex::FILE:
				return fmt::format_to(ctx.out(), "FILE");
			case lex::FILLFACTOR:
				return fmt::format_to(ctx.out(), "FILLFACTOR");
			case lex::FOR:
				return fmt::format_to(ctx.out(), "FOR");
			case lex::FOREIGN:
				return fmt::format_to(ctx.out(), "FOREIGN");
			case lex::FREETEXT:
				return fmt::format_to(ctx.out(), "FREETEXT");
			case lex::FREETEXTTABLE:
				return fmt::format_to(ctx.out(), "FREETEXTTABLE");
			case lex::FROM:
				return fmt::format_to(ctx.out(), "FROM");
			case lex::FULL:
				return fmt::format_to(ctx.out(), "FULL");
			case lex::FUNCTION:
				return fmt::format_to(ctx.out(), "FUNCTION");
			case lex::GOTO:
				return fmt::format_to(ctx.out(), "GOTO");
			case lex::GRANT:
				return fmt::format_to(ctx.out(), "GRANT");
			case lex::GROUP:
				return fmt::format_to(ctx.out(), "GROUP");
			case lex::HAVING:
				return fmt::format_to(ctx.out(), "HAVING");
			case lex::HOLDLOCK:
				return fmt::format_to(ctx.out(), "HOLDLOCK");
			case lex::IDENTITY:
				return fmt::format_to(ctx.out(), "IDENTITY");
			case lex::IDENTITY_INSERT:
				return fmt::format_to(ctx.out(), "IDENTITY_INSERT");
			case lex::IDENTITYCOL:
				return fmt::format_to(ctx.out(), "IDENTITYCOL");
			case lex::IF:
				return fmt::format_to(ctx.out(), "IF");
			case lex::SQL_IN:
				return fmt::format_to(ctx.out(), "SQL_IN");
			case lex::INDEX:
				return fmt::format_to(ctx.out(), "INDEX");
			case lex::INNER:
				return fmt::format_to(ctx.out(), "INNER");
			case lex::INSERT:
				return fmt::format_to(ctx.out(), "INSERT");
			case lex::INTERSECT:
				return fmt::format_to(ctx.out(), "INTERSECT");
			case lex::INTO:
				return fmt::format_to(ctx.out(), "INTO");
			case lex::IS:
				return fmt::format_to(ctx.out(), "IS");
			case lex::JOIN:
				return fmt::format_to(ctx.out(), "JOIN");
			case lex::KEY:
				return fmt::format_to(ctx.out(), "KEY");
			case lex::KILL:
				return fmt::format_to(ctx.out(), "KILL");
			case lex::LEFT:
				return fmt::format_to(ctx.out(), "LEFT");
			case lex::LIKE:
				return fmt::format_to(ctx.out(), "LIKE");
			case lex::LINENO:
				return fmt::format_to(ctx.out(), "LINENO");
			case lex::MERGE:
				return fmt::format_to(ctx.out(), "MERGE");
			case lex::NATIONAL:
				return fmt::format_to(ctx.out(), "NATIONAL");
			case lex::NOCHECK:
				return fmt::format_to(ctx.out(), "NOCHECK");
			case lex::NONCLUSTERED:
				return fmt::format_to(ctx.out(), "NONCLUSTERED");
			case lex::NOT:
				return fmt::format_to(ctx.out(), "NOT");
			case lex::SQL_NULL:
				return fmt::format_to(ctx.out(), "SQL_NULL");
			case lex::NULLIF:
				return fmt::format_to(ctx.out(), "NULLIF");
			case lex::OF:
				return fmt::format_to(ctx.out(), "OF");
			case lex::OFF:
				return fmt::format_to(ctx.out(), "OFF");
			case lex::OFFSETS:
				return fmt::format_to(ctx.out(), "OFFSETS");
			case lex::ON:
				return fmt::format_to(ctx.out(), "ON");
			case lex::OPEN:
				return fmt::format_to(ctx.out(), "OPEN");
			case lex::OPENDATASOURCE:
				return fmt::format_to(ctx.out(), "OPENDATASOURCE");
			case lex::OPENQUERY:
				return fmt::format_to(ctx.out(), "OPENQUERY");
			case lex::OPENROWSET:
				return fmt::format_to(ctx.out(), "OPENROWSET");
			case lex::OPENXML:
				return fmt::format_to(ctx.out(), "OPENXML");
			case lex::OPTION:
				return fmt::format_to(ctx.out(), "OPTION");
			case lex::OR:
				return fmt::format_to(ctx.out(), "OR");
			case lex::ORDER:
				return fmt::format_to(ctx.out(), "ORDER");
			case lex::OUTER:
				return fmt::format_to(ctx.out(), "OUTER");
			case lex::OVER:
				return fmt::format_to(ctx.out(), "OVER");
			case lex::PERCENT:
				return fmt::format_to(ctx.out(), "PERCENT");
			case lex::PIVOT:
				return fmt::format_to(ctx.out(), "PIVOT");
			case lex::PLAN:
				return fmt::format_to(ctx.out(), "PLAN");
			case lex::PRIMARY:
				return fmt::format_to(ctx.out(), "PRIMARY");
			case lex::PRINT:
				return fmt::format_to(ctx.out(), "PRINT");
			case lex::PROCEDURE:
				return fmt::format_to(ctx.out(), "PROCEDURE");
			case lex::PUBLIC:
				return fmt::format_to(ctx.out(), "PUBLIC");
			case lex::RAISERROR:
				return fmt::format_to(ctx.out(), "RAISERROR");
			case lex::READ:
				return fmt::format_to(ctx.out(), "READ");
			case lex::READTEXT:
				return fmt::format_to(ctx.out(), "READTEXT");
			case lex::RECONFIGURE:
				return fmt::format_to(ctx.out(), "RECONFIGURE");
			case lex::REFERENCES:
				return fmt::format_to(ctx.out(), "REFERENCES");
			case lex::REPLICATION:
				return fmt::format_to(ctx.out(), "REPLICATION");
			case lex::RESTORE:
				return fmt::format_to(ctx.out(), "RESTORE");
			case lex::RESTRICT:
				return fmt::format_to(ctx.out(), "RESTRICT");
			case lex::RETURN:
				return fmt::format_to(ctx.out(), "RETURN");
			case lex::REVERT:
				return fmt::format_to(ctx.out(), "REVERT");
			case lex::REVOKE:
				return fmt::format_to(ctx.out(), "REVOKE");
			case lex::RIGHT:
				return fmt::format_to(ctx.out(), "RIGHT");
			case lex::ROLLBACK:
				return fmt::format_to(ctx.out(), "ROLLBACK");
			case lex::ROWCOUNT:
				return fmt::format_to(ctx.out(), "ROWCOUNT");
			case lex::ROWGUIDCOL:
				return fmt::format_to(ctx.out(), "ROWGUIDCOL");
			case lex::RULE:
				return fmt::format_to(ctx.out(), "RULE");
			case lex::SAVE:
				return fmt::format_to(ctx.out(), "SAVE");
			case lex::SCHEMA:
				return fmt::format_to(ctx.out(), "SCHEMA");
			case lex::SELECT:
				return fmt::format_to(ctx.out(), "SELECT");
			case lex::SEMANTICKEYPHRASETABLE:
				return fmt::format_to(ctx.out(), "SEMANTICKEYPHRASETABLE");
			case lex::SEMANTICSIMILARITYDETAILSTABLE:
				return fmt::format_to(ctx.out(), "SEMANTICSIMILARITYDETAILSTABLE");
			case lex::SEMANTICSIMILARITYTABLE:
				return fmt::format_to(ctx.out(), "SEMANTICSIMILARITYTABLE");
			case lex::SESSION_USER:
				return fmt::format_to(ctx.out(), "SESSION_USER");
			case lex::SET:
				return fmt::format_to(ctx.out(), "SET");
			case lex::SETUSER:
				return fmt::format_to(ctx.out(), "SETUSER");
			case lex::SHUTDOWN:
				return fmt::format_to(ctx.out(), "SHUTDOWN");
			case lex::SOME:
				return fmt::format_to(ctx.out(), "SOME");
			case lex::STATISTICS:
				return fmt::format_to(ctx.out(), "STATISTICS");
			case lex::SYSTEM_USER:
				return fmt::format_to(ctx.out(), "SYSTEM_USER");
			case lex::TABLE:
				return fmt::format_to(ctx.out(), "TABLE");
			case lex::TABLESAMPLE:
				return fmt::format_to(ctx.out(), "TABLESAMPLE");
			case lex::TEXTSIZE:
				return fmt::format_to(ctx.out(), "TEXTSIZE");
			case lex::THEN:
				return fmt::format_to(ctx.out(), "THEN");
			case lex::TO:
				return fmt::format_to(ctx.out(), "TO");
			case lex::TOP:
				return fmt::format_to(ctx.out(), "TOP");
			case lex::TRAN:
				return fmt::format_to(ctx.out(), "TRAN");
			case lex::TRANSACTION:
				return fmt::format_to(ctx.out(), "TRANSACTION");
			case lex::TRIGGER:
				return fmt::format_to(ctx.out(), "TRIGGER");
			case lex::TRUNCATE:
				return fmt::format_to(ctx.out(), "TRUNCATE");
			case lex::TRY_CONVERT:
				return fmt::format_to(ctx.out(), "TRY_CONVERT");
			case lex::TSEQUAL:
				return fmt::format_to(ctx.out(), "TSEQUAL");
			case lex::UNION:
				return fmt::format_to(ctx.out(), "UNION");
			case lex::UNIQUE:
				return fmt::format_to(ctx.out(), "UNIQUE");
			case lex::UNPIVOT:
				return fmt::format_to(ctx.out(), "UNPIVOT");
			case lex::UPDATE:
				return fmt::format_to(ctx.out(), "UPDATE");
			case lex::UPDATETEXT:
				return fmt::format_to(ctx.out(), "UPDATETEXT");
			case lex::USE:
				return fmt::format_to(ctx.out(), "USE");
			case lex::USER:
				return fmt::format_to(ctx.out(), "USER");
			case lex::VALUES:
				return fmt::format_to(ctx.out(), "VALUES");
			case lex::VARYING:
				return fmt::format_to(ctx.out(), "VARYING");
			case lex::VIEW:
				return fmt::format_to(ctx.out(), "VIEW");
			case lex::WAITFOR:
				return fmt::format_to(ctx.out(), "WAITFOR");
			case lex::WHEN:
				return fmt::format_to(ctx.out(), "WHEN");
			case lex::WHERE:
				return fmt::format_to(ctx.out(), "WHERE");
			case lex::WHILE:
				return fmt::format_to(ctx.out(), "WHILE");
			case lex::WITH:
				return fmt::format_to(ctx.out(), "WITH");
			case lex::WRITETEXT:
				return fmt::format_to(ctx.out(), "WRITETEXT");
			default:
				return fmt::format_to(ctx.out(), "{}", (int)t);
		}
	}
};

// gitsql.cpp
std::string object_perms(tds::tds& tds, int64_t id, const std::string& dbs, const std::string& name);

// table.cpp
std::string table_ddl(tds::tds& tds, int64_t id);
std::string brackets_escape(std::string_view s);
std::u16string brackets_escape(const std::u16string_view& s);

// ldap.cpp
void get_ldap_details_from_sid(PSID sid, std::string& name, std::string& email);

// parse.cpp
std::string munge_definition(std::string_view sql, std::string_view schema, std::string_view name,
							 enum lex type);
