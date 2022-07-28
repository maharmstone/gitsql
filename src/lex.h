#pragma once

#include <string_view>
#include <list>
#include <fmt/format.h>

enum class lex {
    whitespace,
    single_comment,
    multi_comment,
    identifier,
    full_stop,
    slash,
    semicolon,
    variable,
    equals,
    open_bracket,
    close_bracket,
    number,
    minus,
    plus,
    asterisk,
    ampersand,
    percent,
    pipe,
    string_literal,
    binary_literal,
    money_literal,
    comma,
    greater_than,
    less_than,
    colon,
    exclamation_point,
    open_brace,
    close_brace,
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
    WRITETEXT,
};

static const struct {
    std::string_view s;
    enum lex type;
} reserved_words[] = {
    { "ADD", lex::ADD },
    { "ALL", lex::ALL },
    { "ALTER", lex::ALTER },
    { "AND", lex::AND },
    { "ANY", lex::ANY },
    { "AS", lex::AS },
    { "ASC", lex::ASC },
    { "AUTHORIZATION", lex::AUTHORIZATION },
    { "BACKUP", lex::BACKUP },
    { "BEGIN", lex::BEGIN },
    { "BETWEEN", lex::BETWEEN },
    { "BREAK", lex::BREAK },
    { "BROWSE", lex::BROWSE },
    { "BULK", lex::BULK },
    { "BY", lex::BY },
    { "CASCADE", lex::CASCADE },
    { "CASE", lex::CASE },
    { "CHECK", lex::CHECK },
    { "CHECKPOINT", lex::CHECKPOINT },
    { "CLOSE", lex::CLOSE },
    { "CLUSTERED", lex::CLUSTERED },
    { "COALESCE", lex::COALESCE },
    { "COLLATE", lex::COLLATE },
    { "COLUMN", lex::COLUMN },
    { "COMMIT", lex::COMMIT },
    { "COMPUTE", lex::COMPUTE },
    { "CONSTRAINT", lex::CONSTRAINT },
    { "CONTAINS", lex::CONTAINS },
    { "CONTAINSTABLE", lex::CONTAINSTABLE },
    { "CONTINUE", lex::CONTINUE },
    { "CONVERT", lex::CONVERT },
    { "CREATE", lex::CREATE },
    { "CROSS", lex::CROSS },
    { "CURRENT", lex::CURRENT },
    { "CURRENT_DATE", lex::CURRENT_DATE },
    { "CURRENT_TIME", lex::CURRENT_TIME },
    { "CURRENT_TIMESTAMP", lex::CURRENT_TIMESTAMP },
    { "CURRENT_USER", lex::CURRENT_USER },
    { "CURSOR", lex::CURSOR },
    { "DATABASE", lex::DATABASE },
    { "DBCC", lex::DBCC },
    { "DEALLOCATE", lex::DEALLOCATE },
    { "DECLARE", lex::DECLARE },
    { "DEFAULT", lex::DEFAULT },
    { "DELETE", lex::SQL_DELETE },
    { "DENY", lex::DENY },
    { "DESC", lex::DESC },
    { "DISTINCT", lex::DISTINCT },
    { "DISTRIBUTED", lex::DISTRIBUTED },
    { "DOUBLE", lex::DOUBLE },
    { "DROP", lex::DROP },
    { "ELSE", lex::ELSE },
    { "END", lex::END },
    { "ERRLVL", lex::ERRLVL },
    { "ESCAPE", lex::ESCAPE },
    { "EXCEPT", lex::EXCEPT },
    { "EXEC", lex::EXEC },
    { "EXECUTE", lex::EXEC },
    { "EXISTS", lex::EXISTS },
    { "EXIT", lex::EXIT },
    { "EXTERNAL", lex::EXTERNAL },
    { "FETCH", lex::FETCH },
    { "FILE", lex::FILE },
    { "FILLFACTOR", lex::FILLFACTOR },
    { "FOR", lex::FOR },
    { "FOREIGN", lex::FOREIGN },
    { "FREETEXT", lex::FREETEXT },
    { "FREETEXTTABLE", lex::FREETEXTTABLE },
    { "FROM", lex::FROM },
    { "FULL", lex::FULL },
    { "FUNCTION", lex::FUNCTION },
    { "GOTO", lex::GOTO },
    { "GRANT", lex::GRANT },
    { "GROUP", lex::GROUP },
    { "HAVING", lex::HAVING },
    { "HOLDLOCK", lex::HOLDLOCK },
    { "IDENTITY", lex::IDENTITY },
    { "IDENTITY_INSERT", lex::IDENTITY_INSERT },
    { "IDENTITYCOL", lex::IDENTITYCOL },
    { "IF", lex::IF },
    { "IN", lex::SQL_IN },
    { "INDEX", lex::INDEX },
    { "INNER", lex::INNER },
    { "INSERT", lex::INSERT },
    { "INTERSECT", lex::INTERSECT },
    { "INTO", lex::INTO },
    { "IS", lex::IS },
    { "JOIN", lex::JOIN },
    { "KEY", lex::KEY },
    { "KILL", lex::KILL },
    { "LEFT", lex::LEFT },
    { "LIKE", lex::LIKE },
    { "LINENO", lex::LINENO },
    { "MERGE", lex::MERGE },
    { "NATIONAL", lex::NATIONAL },
    { "NOCHECK", lex::NOCHECK },
    { "NONCLUSTERED", lex::NONCLUSTERED },
    { "NOT", lex::NOT },
    { "NULL", lex::SQL_NULL },
    { "NULLIF", lex::NULLIF },
    { "OF", lex::OF },
    { "OFF", lex::OFF },
    { "OFFSETS", lex::OFFSETS },
    { "ON", lex::ON },
    { "OPEN", lex::OPEN },
    { "OPENDATASOURCE", lex::OPENDATASOURCE },
    { "OPENQUERY", lex::OPENQUERY },
    { "OPENROWSET", lex::OPENROWSET },
    { "OPENXML", lex::OPENXML },
    { "OPTION", lex::OPTION },
    { "OR", lex::OR },
    { "ORDER", lex::ORDER },
    { "OUTER", lex::OUTER },
    { "OVER", lex::OVER },
    { "PERCENT", lex::PERCENT },
    { "PIVOT", lex::PIVOT },
    { "PLAN", lex::PLAN },
    { "PRIMARY", lex::PRIMARY },
    { "PRINT", lex::PRINT },
    { "PROC", lex::PROCEDURE },
    { "PROCEDURE", lex::PROCEDURE },
    { "PUBLIC", lex::PUBLIC },
    { "RAISERROR", lex::RAISERROR },
    { "READ", lex::READ },
    { "READTEXT", lex::READTEXT },
    { "RECONFIGURE", lex::RECONFIGURE },
    { "REFERENCES", lex::REFERENCES },
    { "REPLICATION", lex::REPLICATION },
    { "RESTORE", lex::RESTORE },
    { "RESTRICT", lex::RESTRICT },
    { "RETURN", lex::RETURN },
    { "REVERT", lex::REVERT },
    { "REVOKE", lex::REVOKE },
    { "RIGHT", lex::RIGHT },
    { "ROLLBACK", lex::ROLLBACK },
    { "ROWCOUNT", lex::ROWCOUNT },
    { "ROWGUIDCOL", lex::ROWGUIDCOL },
    { "RULE", lex::RULE },
    { "SAVE", lex::SAVE },
    { "SCHEMA", lex::SCHEMA },
    { "SELECT", lex::SELECT },
    { "SEMANTICKEYPHRASETABLE", lex::SEMANTICKEYPHRASETABLE },
    { "SEMANTICSIMILARITYDETAILSTABLE", lex::SEMANTICSIMILARITYDETAILSTABLE },
    { "SEMANTICSIMILARITYTABLE", lex::SEMANTICSIMILARITYTABLE },
    { "SESSION_USER", lex::SESSION_USER },
    { "SET", lex::SET },
    { "SETUSER", lex::SETUSER },
    { "SHUTDOWN", lex::SHUTDOWN },
    { "SOME", lex::SOME },
    { "STATISTICS", lex::STATISTICS },
    { "SYSTEM_USER", lex::SYSTEM_USER },
    { "TABLE", lex::TABLE },
    { "TABLESAMPLE", lex::TABLESAMPLE },
    { "TEXTSIZE", lex::TEXTSIZE },
    { "THEN", lex::THEN },
    { "TO", lex::TO },
    { "TOP", lex::TOP },
    { "TRAN", lex::TRAN },
    { "TRANSACTION", lex::TRANSACTION },
    { "TRIGGER", lex::TRIGGER },
    { "TRUNCATE", lex::TRUNCATE },
    { "TRY_CONVERT", lex::TRY_CONVERT },
    { "TSEQUAL", lex::TSEQUAL },
    { "UNION", lex::UNION },
    { "UNIQUE", lex::UNIQUE },
    { "UNPIVOT", lex::UNPIVOT },
    { "UPDATE", lex::UPDATE },
    { "UPDATETEXT", lex::UPDATETEXT },
    { "USE", lex::USE },
    { "USER", lex::USER },
    { "VALUES", lex::VALUES },
    { "VARYING", lex::VARYING },
    { "VIEW", lex::VIEW },
    { "WAITFOR", lex::WAITFOR },
    { "WHEN", lex::WHEN },
    { "WHERE", lex::WHERE },
    { "WHILE", lex::WHILE },
    { "WITH", lex::WITH },
    { "WRITETEXT", lex::WRITETEXT },
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
    auto format(enum lex l, format_context& ctx) const {
        switch (l) {
            case lex::whitespace:
                return fmt::format_to(ctx.out(), "whitespace");
            case lex::single_comment:
                return fmt::format_to(ctx.out(), "single_comment");
            case lex::multi_comment:
                return fmt::format_to(ctx.out(), "multi_comment");
            case lex::identifier:
                return fmt::format_to(ctx.out(), "identifier");
            case lex::full_stop:
                return fmt::format_to(ctx.out(), "full_stop");
            case lex::slash:
                return fmt::format_to(ctx.out(), "slash");
            case lex::semicolon:
                return fmt::format_to(ctx.out(), "semicolon");
            case lex::variable:
                return fmt::format_to(ctx.out(), "variable");
            case lex::equals:
                return fmt::format_to(ctx.out(), "equals");
            case lex::open_bracket:
                return fmt::format_to(ctx.out(), "open_bracket");
            case lex::close_bracket:
                return fmt::format_to(ctx.out(), "close_bracket");
            case lex::number:
                return fmt::format_to(ctx.out(), "number");
            case lex::minus:
                return fmt::format_to(ctx.out(), "minus");
            case lex::plus:
                return fmt::format_to(ctx.out(), "plus");
            case lex::asterisk:
                return fmt::format_to(ctx.out(), "asterisk");
            case lex::ampersand:
                return fmt::format_to(ctx.out(), "ampersand");
            case lex::percent:
                return fmt::format_to(ctx.out(), "percent");
            case lex::pipe:
                return fmt::format_to(ctx.out(), "pipe");
            case lex::string_literal:
                return fmt::format_to(ctx.out(), "string_literal");
            case lex::binary_literal:
                return fmt::format_to(ctx.out(), "binary_literal");
            case lex::money_literal:
                return fmt::format_to(ctx.out(), "money_literal");
            case lex::comma:
                return fmt::format_to(ctx.out(), "comma");
            case lex::greater_than:
                return fmt::format_to(ctx.out(), "greater_than");
            case lex::less_than:
                return fmt::format_to(ctx.out(), "less_than");
            case lex::colon:
                return fmt::format_to(ctx.out(), "colon");
            case lex::exclamation_point:
                return fmt::format_to(ctx.out(), "exclamation_point");
            case lex::open_brace:
                return fmt::format_to(ctx.out(), "open_brace");
            case lex::close_brace:
                return fmt::format_to(ctx.out(), "close_brace");
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
                return fmt::format_to(ctx.out(), "{}", (int)l);
        }
    }
};

struct word {
    enum lex type;
    std::string_view val;
};

std::list<word> parse(std::string_view s);
