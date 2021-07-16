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
	last_error(const std::string_view& function, int le) {
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

enum class sql_word {
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
struct fmt::formatter<enum sql_word> {
	constexpr auto parse(format_parse_context& ctx) {
		auto it = ctx.begin();

		if (it != ctx.end() && *it != '}')
			throw format_error("invalid format");

		return it;
	}

	template<typename format_context>
	auto format(enum sql_word t, format_context& ctx) const {
		switch (t) {
			case sql_word::unknown:
				return fmt::format_to(ctx.out(), "unknown");
			case sql_word::whitespace:
				return fmt::format_to(ctx.out(), "whitespace");
			case sql_word::comment:
				return fmt::format_to(ctx.out(), "comment");
			case sql_word::identifier:
				return fmt::format_to(ctx.out(), "identifier");
			case sql_word::number:
				return fmt::format_to(ctx.out(), "number");
			case sql_word::string:
				return fmt::format_to(ctx.out(), "string");
			case sql_word::open_bracket:
				return fmt::format_to(ctx.out(), "open_bracket");
			case sql_word::close_bracket:
				return fmt::format_to(ctx.out(), "close_bracket");
			case sql_word::comma:
				return fmt::format_to(ctx.out(), "comma");
			case sql_word::semicolon:
				return fmt::format_to(ctx.out(), "semicolon");
			case sql_word::full_stop:
				return fmt::format_to(ctx.out(), "full_stop");
			case sql_word::minus:
				return fmt::format_to(ctx.out(), "minus");
			case sql_word::slash:
				return fmt::format_to(ctx.out(), "slash");
			case sql_word::equals:
				return fmt::format_to(ctx.out(), "equals");
			case sql_word::ADD:
				return fmt::format_to(ctx.out(), "ADD");
			case sql_word::ALL:
				return fmt::format_to(ctx.out(), "ALL");
			case sql_word::ALTER:
				return fmt::format_to(ctx.out(), "ALTER");
			case sql_word::AND:
				return fmt::format_to(ctx.out(), "AND");
			case sql_word::ANY:
				return fmt::format_to(ctx.out(), "ANY");
			case sql_word::AS:
				return fmt::format_to(ctx.out(), "AS");
			case sql_word::ASC:
				return fmt::format_to(ctx.out(), "ASC");
			case sql_word::AUTHORIZATION:
				return fmt::format_to(ctx.out(), "AUTHORIZATION");
			case sql_word::BACKUP:
				return fmt::format_to(ctx.out(), "BACKUP");
			case sql_word::BEGIN:
				return fmt::format_to(ctx.out(), "BEGIN");
			case sql_word::BETWEEN:
				return fmt::format_to(ctx.out(), "BETWEEN");
			case sql_word::BREAK:
				return fmt::format_to(ctx.out(), "BREAK");
			case sql_word::BROWSE:
				return fmt::format_to(ctx.out(), "BROWSE");
			case sql_word::BULK:
				return fmt::format_to(ctx.out(), "BULK");
			case sql_word::BY:
				return fmt::format_to(ctx.out(), "BY");
			case sql_word::CASCADE:
				return fmt::format_to(ctx.out(), "CASCADE");
			case sql_word::CASE:
				return fmt::format_to(ctx.out(), "CASE");
			case sql_word::CHECK:
				return fmt::format_to(ctx.out(), "CHECK");
			case sql_word::CHECKPOINT:
				return fmt::format_to(ctx.out(), "CHECKPOINT");
			case sql_word::CLOSE:
				return fmt::format_to(ctx.out(), "CLOSE");
			case sql_word::CLUSTERED:
				return fmt::format_to(ctx.out(), "CLUSTERED");
			case sql_word::COALESCE:
				return fmt::format_to(ctx.out(), "COALESCE");
			case sql_word::COLLATE:
				return fmt::format_to(ctx.out(), "COLLATE");
			case sql_word::COLUMN:
				return fmt::format_to(ctx.out(), "COLUMN");
			case sql_word::COMMIT:
				return fmt::format_to(ctx.out(), "COMMIT");
			case sql_word::COMPUTE:
				return fmt::format_to(ctx.out(), "COMPUTE");
			case sql_word::CONSTRAINT:
				return fmt::format_to(ctx.out(), "CONSTRAINT");
			case sql_word::CONTAINS:
				return fmt::format_to(ctx.out(), "CONTAINS");
			case sql_word::CONTAINSTABLE:
				return fmt::format_to(ctx.out(), "CONTAINSTABLE");
			case sql_word::CONTINUE:
				return fmt::format_to(ctx.out(), "CONTINUE");
			case sql_word::CONVERT:
				return fmt::format_to(ctx.out(), "CONVERT");
			case sql_word::CREATE:
				return fmt::format_to(ctx.out(), "CREATE");
			case sql_word::CROSS:
				return fmt::format_to(ctx.out(), "CROSS");
			case sql_word::CURRENT:
				return fmt::format_to(ctx.out(), "CURRENT");
			case sql_word::CURRENT_DATE:
				return fmt::format_to(ctx.out(), "CURRENT_DATE");
			case sql_word::CURRENT_TIME:
				return fmt::format_to(ctx.out(), "CURRENT_TIME");
			case sql_word::CURRENT_TIMESTAMP:
				return fmt::format_to(ctx.out(), "CURRENT_TIMESTAMP");
			case sql_word::CURRENT_USER:
				return fmt::format_to(ctx.out(), "CURRENT_USER");
			case sql_word::CURSOR:
				return fmt::format_to(ctx.out(), "CURSOR");
			case sql_word::DATABASE:
				return fmt::format_to(ctx.out(), "DATABASE");
			case sql_word::DBCC:
				return fmt::format_to(ctx.out(), "DBCC");
			case sql_word::DEALLOCATE:
				return fmt::format_to(ctx.out(), "DEALLOCATE");
			case sql_word::DECLARE:
				return fmt::format_to(ctx.out(), "DECLARE");
			case sql_word::DEFAULT:
				return fmt::format_to(ctx.out(), "DEFAULT");
			case sql_word::SQL_DELETE:
				return fmt::format_to(ctx.out(), "SQL_DELETE");
			case sql_word::DENY:
				return fmt::format_to(ctx.out(), "DENY");
			case sql_word::DESC:
				return fmt::format_to(ctx.out(), "DESC");
			case sql_word::DISTINCT:
				return fmt::format_to(ctx.out(), "DISTINCT");
			case sql_word::DISTRIBUTED:
				return fmt::format_to(ctx.out(), "DISTRIBUTED");
			case sql_word::DOUBLE:
				return fmt::format_to(ctx.out(), "DOUBLE");
			case sql_word::DROP:
				return fmt::format_to(ctx.out(), "DROP");
			case sql_word::ELSE:
				return fmt::format_to(ctx.out(), "ELSE");
			case sql_word::END:
				return fmt::format_to(ctx.out(), "END");
			case sql_word::ERRLVL:
				return fmt::format_to(ctx.out(), "ERRLVL");
			case sql_word::ESCAPE:
				return fmt::format_to(ctx.out(), "ESCAPE");
			case sql_word::EXCEPT:
				return fmt::format_to(ctx.out(), "EXCEPT");
			case sql_word::EXEC:
				return fmt::format_to(ctx.out(), "EXEC");
			case sql_word::EXECUTE:
				return fmt::format_to(ctx.out(), "EXECUTE");
			case sql_word::EXISTS:
				return fmt::format_to(ctx.out(), "EXISTS");
			case sql_word::EXIT:
				return fmt::format_to(ctx.out(), "EXIT");
			case sql_word::EXTERNAL:
				return fmt::format_to(ctx.out(), "EXTERNAL");
			case sql_word::FETCH:
				return fmt::format_to(ctx.out(), "FETCH");
			case sql_word::FILE:
				return fmt::format_to(ctx.out(), "FILE");
			case sql_word::FILLFACTOR:
				return fmt::format_to(ctx.out(), "FILLFACTOR");
			case sql_word::FOR:
				return fmt::format_to(ctx.out(), "FOR");
			case sql_word::FOREIGN:
				return fmt::format_to(ctx.out(), "FOREIGN");
			case sql_word::FREETEXT:
				return fmt::format_to(ctx.out(), "FREETEXT");
			case sql_word::FREETEXTTABLE:
				return fmt::format_to(ctx.out(), "FREETEXTTABLE");
			case sql_word::FROM:
				return fmt::format_to(ctx.out(), "FROM");
			case sql_word::FULL:
				return fmt::format_to(ctx.out(), "FULL");
			case sql_word::FUNCTION:
				return fmt::format_to(ctx.out(), "FUNCTION");
			case sql_word::GOTO:
				return fmt::format_to(ctx.out(), "GOTO");
			case sql_word::GRANT:
				return fmt::format_to(ctx.out(), "GRANT");
			case sql_word::GROUP:
				return fmt::format_to(ctx.out(), "GROUP");
			case sql_word::HAVING:
				return fmt::format_to(ctx.out(), "HAVING");
			case sql_word::HOLDLOCK:
				return fmt::format_to(ctx.out(), "HOLDLOCK");
			case sql_word::IDENTITY:
				return fmt::format_to(ctx.out(), "IDENTITY");
			case sql_word::IDENTITY_INSERT:
				return fmt::format_to(ctx.out(), "IDENTITY_INSERT");
			case sql_word::IDENTITYCOL:
				return fmt::format_to(ctx.out(), "IDENTITYCOL");
			case sql_word::IF:
				return fmt::format_to(ctx.out(), "IF");
			case sql_word::SQL_IN:
				return fmt::format_to(ctx.out(), "SQL_IN");
			case sql_word::INDEX:
				return fmt::format_to(ctx.out(), "INDEX");
			case sql_word::INNER:
				return fmt::format_to(ctx.out(), "INNER");
			case sql_word::INSERT:
				return fmt::format_to(ctx.out(), "INSERT");
			case sql_word::INTERSECT:
				return fmt::format_to(ctx.out(), "INTERSECT");
			case sql_word::INTO:
				return fmt::format_to(ctx.out(), "INTO");
			case sql_word::IS:
				return fmt::format_to(ctx.out(), "IS");
			case sql_word::JOIN:
				return fmt::format_to(ctx.out(), "JOIN");
			case sql_word::KEY:
				return fmt::format_to(ctx.out(), "KEY");
			case sql_word::KILL:
				return fmt::format_to(ctx.out(), "KILL");
			case sql_word::LEFT:
				return fmt::format_to(ctx.out(), "LEFT");
			case sql_word::LIKE:
				return fmt::format_to(ctx.out(), "LIKE");
			case sql_word::LINENO:
				return fmt::format_to(ctx.out(), "LINENO");
			case sql_word::MERGE:
				return fmt::format_to(ctx.out(), "MERGE");
			case sql_word::NATIONAL:
				return fmt::format_to(ctx.out(), "NATIONAL");
			case sql_word::NOCHECK:
				return fmt::format_to(ctx.out(), "NOCHECK");
			case sql_word::NONCLUSTERED:
				return fmt::format_to(ctx.out(), "NONCLUSTERED");
			case sql_word::NOT:
				return fmt::format_to(ctx.out(), "NOT");
			case sql_word::SQL_NULL:
				return fmt::format_to(ctx.out(), "SQL_NULL");
			case sql_word::NULLIF:
				return fmt::format_to(ctx.out(), "NULLIF");
			case sql_word::OF:
				return fmt::format_to(ctx.out(), "OF");
			case sql_word::OFF:
				return fmt::format_to(ctx.out(), "OFF");
			case sql_word::OFFSETS:
				return fmt::format_to(ctx.out(), "OFFSETS");
			case sql_word::ON:
				return fmt::format_to(ctx.out(), "ON");
			case sql_word::OPEN:
				return fmt::format_to(ctx.out(), "OPEN");
			case sql_word::OPENDATASOURCE:
				return fmt::format_to(ctx.out(), "OPENDATASOURCE");
			case sql_word::OPENQUERY:
				return fmt::format_to(ctx.out(), "OPENQUERY");
			case sql_word::OPENROWSET:
				return fmt::format_to(ctx.out(), "OPENROWSET");
			case sql_word::OPENXML:
				return fmt::format_to(ctx.out(), "OPENXML");
			case sql_word::OPTION:
				return fmt::format_to(ctx.out(), "OPTION");
			case sql_word::OR:
				return fmt::format_to(ctx.out(), "OR");
			case sql_word::ORDER:
				return fmt::format_to(ctx.out(), "ORDER");
			case sql_word::OUTER:
				return fmt::format_to(ctx.out(), "OUTER");
			case sql_word::OVER:
				return fmt::format_to(ctx.out(), "OVER");
			case sql_word::PERCENT:
				return fmt::format_to(ctx.out(), "PERCENT");
			case sql_word::PIVOT:
				return fmt::format_to(ctx.out(), "PIVOT");
			case sql_word::PLAN:
				return fmt::format_to(ctx.out(), "PLAN");
			case sql_word::PRIMARY:
				return fmt::format_to(ctx.out(), "PRIMARY");
			case sql_word::PRINT:
				return fmt::format_to(ctx.out(), "PRINT");
			case sql_word::PROCEDURE:
				return fmt::format_to(ctx.out(), "PROCEDURE");
			case sql_word::PUBLIC:
				return fmt::format_to(ctx.out(), "PUBLIC");
			case sql_word::RAISERROR:
				return fmt::format_to(ctx.out(), "RAISERROR");
			case sql_word::READ:
				return fmt::format_to(ctx.out(), "READ");
			case sql_word::READTEXT:
				return fmt::format_to(ctx.out(), "READTEXT");
			case sql_word::RECONFIGURE:
				return fmt::format_to(ctx.out(), "RECONFIGURE");
			case sql_word::REFERENCES:
				return fmt::format_to(ctx.out(), "REFERENCES");
			case sql_word::REPLICATION:
				return fmt::format_to(ctx.out(), "REPLICATION");
			case sql_word::RESTORE:
				return fmt::format_to(ctx.out(), "RESTORE");
			case sql_word::RESTRICT:
				return fmt::format_to(ctx.out(), "RESTRICT");
			case sql_word::RETURN:
				return fmt::format_to(ctx.out(), "RETURN");
			case sql_word::REVERT:
				return fmt::format_to(ctx.out(), "REVERT");
			case sql_word::REVOKE:
				return fmt::format_to(ctx.out(), "REVOKE");
			case sql_word::RIGHT:
				return fmt::format_to(ctx.out(), "RIGHT");
			case sql_word::ROLLBACK:
				return fmt::format_to(ctx.out(), "ROLLBACK");
			case sql_word::ROWCOUNT:
				return fmt::format_to(ctx.out(), "ROWCOUNT");
			case sql_word::ROWGUIDCOL:
				return fmt::format_to(ctx.out(), "ROWGUIDCOL");
			case sql_word::RULE:
				return fmt::format_to(ctx.out(), "RULE");
			case sql_word::SAVE:
				return fmt::format_to(ctx.out(), "SAVE");
			case sql_word::SCHEMA:
				return fmt::format_to(ctx.out(), "SCHEMA");
			case sql_word::SELECT:
				return fmt::format_to(ctx.out(), "SELECT");
			case sql_word::SEMANTICKEYPHRASETABLE:
				return fmt::format_to(ctx.out(), "SEMANTICKEYPHRASETABLE");
			case sql_word::SEMANTICSIMILARITYDETAILSTABLE:
				return fmt::format_to(ctx.out(), "SEMANTICSIMILARITYDETAILSTABLE");
			case sql_word::SEMANTICSIMILARITYTABLE:
				return fmt::format_to(ctx.out(), "SEMANTICSIMILARITYTABLE");
			case sql_word::SESSION_USER:
				return fmt::format_to(ctx.out(), "SESSION_USER");
			case sql_word::SET:
				return fmt::format_to(ctx.out(), "SET");
			case sql_word::SETUSER:
				return fmt::format_to(ctx.out(), "SETUSER");
			case sql_word::SHUTDOWN:
				return fmt::format_to(ctx.out(), "SHUTDOWN");
			case sql_word::SOME:
				return fmt::format_to(ctx.out(), "SOME");
			case sql_word::STATISTICS:
				return fmt::format_to(ctx.out(), "STATISTICS");
			case sql_word::SYSTEM_USER:
				return fmt::format_to(ctx.out(), "SYSTEM_USER");
			case sql_word::TABLE:
				return fmt::format_to(ctx.out(), "TABLE");
			case sql_word::TABLESAMPLE:
				return fmt::format_to(ctx.out(), "TABLESAMPLE");
			case sql_word::TEXTSIZE:
				return fmt::format_to(ctx.out(), "TEXTSIZE");
			case sql_word::THEN:
				return fmt::format_to(ctx.out(), "THEN");
			case sql_word::TO:
				return fmt::format_to(ctx.out(), "TO");
			case sql_word::TOP:
				return fmt::format_to(ctx.out(), "TOP");
			case sql_word::TRAN:
				return fmt::format_to(ctx.out(), "TRAN");
			case sql_word::TRANSACTION:
				return fmt::format_to(ctx.out(), "TRANSACTION");
			case sql_word::TRIGGER:
				return fmt::format_to(ctx.out(), "TRIGGER");
			case sql_word::TRUNCATE:
				return fmt::format_to(ctx.out(), "TRUNCATE");
			case sql_word::TRY_CONVERT:
				return fmt::format_to(ctx.out(), "TRY_CONVERT");
			case sql_word::TSEQUAL:
				return fmt::format_to(ctx.out(), "TSEQUAL");
			case sql_word::UNION:
				return fmt::format_to(ctx.out(), "UNION");
			case sql_word::UNIQUE:
				return fmt::format_to(ctx.out(), "UNIQUE");
			case sql_word::UNPIVOT:
				return fmt::format_to(ctx.out(), "UNPIVOT");
			case sql_word::UPDATE:
				return fmt::format_to(ctx.out(), "UPDATE");
			case sql_word::UPDATETEXT:
				return fmt::format_to(ctx.out(), "UPDATETEXT");
			case sql_word::USE:
				return fmt::format_to(ctx.out(), "USE");
			case sql_word::USER:
				return fmt::format_to(ctx.out(), "USER");
			case sql_word::VALUES:
				return fmt::format_to(ctx.out(), "VALUES");
			case sql_word::VARYING:
				return fmt::format_to(ctx.out(), "VARYING");
			case sql_word::VIEW:
				return fmt::format_to(ctx.out(), "VIEW");
			case sql_word::WAITFOR:
				return fmt::format_to(ctx.out(), "WAITFOR");
			case sql_word::WHEN:
				return fmt::format_to(ctx.out(), "WHEN");
			case sql_word::WHERE:
				return fmt::format_to(ctx.out(), "WHERE");
			case sql_word::WHILE:
				return fmt::format_to(ctx.out(), "WHILE");
			case sql_word::WITH:
				return fmt::format_to(ctx.out(), "WITH");
			case sql_word::WRITETEXT:
				return fmt::format_to(ctx.out(), "WRITETEXT");
			default:
				return fmt::format_to(ctx.out(), "{}", (int)t);
		}
	}
};

// gitsql.cpp
std::string object_perms(tds::tds& tds, int64_t id, const std::string& dbs, const std::string& name);
void replace_all(std::string& source, const std::string& from, const std::string& to);

// table.cpp
std::string table_ddl(tds::tds& tds, int64_t id);
std::string brackets_escape(const std::string_view& s);
std::u16string brackets_escape(const std::u16string_view& s);

// ldap.cpp
void get_ldap_details_from_sid(PSID sid, std::string& name, std::string& email);

// parse.cpp
std::string munge_definition(const std::string_view& sql, const std::string_view& schema, const std::string_view& name,
							 enum sql_word type);
