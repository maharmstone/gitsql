#include "gitsql.h"

using namespace std;

static enum sql_word identify_word(const string_view& s) {
	if (s == "ADD")
		return sql_word::ADD;
	else if (s == "ALL")
		return sql_word::ALL;
	else if (s == "ALTER")
		return sql_word::ALTER;
	else if (s == "AND")
		return sql_word::AND;
	else if (s == "ANY")
		return sql_word::ANY;
	else if (s == "AS")
		return sql_word::AS;
	else if (s == "ASC")
		return sql_word::ASC;
	else if (s == "AUTHORIZATION")
		return sql_word::AUTHORIZATION;
	else if (s == "BACKUP")
		return sql_word::BACKUP;
	else if (s == "BEGIN")
		return sql_word::BEGIN;
	else if (s == "BETWEEN")
		return sql_word::BETWEEN;
	else if (s == "BREAK")
		return sql_word::BREAK;
	else if (s == "BROWSE")
		return sql_word::BROWSE;
	else if (s == "BULK")
		return sql_word::BULK;
	else if (s == "BY")
		return sql_word::BY;
	else if (s == "CASCADE")
		return sql_word::CASCADE;
	else if (s == "CASE")
		return sql_word::CASE;
	else if (s == "CHECK")
		return sql_word::CHECK;
	else if (s == "CHECKPOINT")
		return sql_word::CHECKPOINT;
	else if (s == "CLOSE")
		return sql_word::CLOSE;
	else if (s == "CLUSTERED")
		return sql_word::CLUSTERED;
	else if (s == "COALESCE")
		return sql_word::COALESCE;
	else if (s == "COLLATE")
		return sql_word::COLLATE;
	else if (s == "COLUMN")
		return sql_word::COLUMN;
	else if (s == "COMMIT")
		return sql_word::COMMIT;
	else if (s == "COMPUTE")
		return sql_word::COMPUTE;
	else if (s == "CONSTRAINT")
		return sql_word::CONSTRAINT;
	else if (s == "CONTAINS")
		return sql_word::CONTAINS;
	else if (s == "CONTAINSTABLE")
		return sql_word::CONTAINSTABLE;
	else if (s == "CONTINUE")
		return sql_word::CONTINUE;
	else if (s == "CONVERT")
		return sql_word::CONVERT;
	else if (s == "CREATE")
		return sql_word::CREATE;
	else if (s == "CROSS")
		return sql_word::CROSS;
	else if (s == "CURRENT")
		return sql_word::CURRENT;
	else if (s == "CURRENT_DATE")
		return sql_word::CURRENT_DATE;
	else if (s == "CURRENT_TIME")
		return sql_word::CURRENT_TIME;
	else if (s == "CURRENT_TIMESTAMP")
		return sql_word::CURRENT_TIMESTAMP;
	else if (s == "CURRENT_USER")
		return sql_word::CURRENT_USER;
	else if (s == "CURSOR")
		return sql_word::CURSOR;
	else if (s == "DATABASE")
		return sql_word::DATABASE;
	else if (s == "DBCC")
		return sql_word::DBCC;
	else if (s == "DEALLOCATE")
		return sql_word::DEALLOCATE;
	else if (s == "DECLARE")
		return sql_word::DECLARE;
	else if (s == "DEFAULT")
		return sql_word::DEFAULT;
	else if (s == "DELETE")
		return sql_word::SQL_DELETE;
	else if (s == "DENY")
		return sql_word::DENY;
	else if (s == "DESC")
		return sql_word::DESC;
	else if (s == "DISTINCT")
		return sql_word::DISTINCT;
	else if (s == "DISTRIBUTED")
		return sql_word::DISTRIBUTED;
	else if (s == "DOUBLE")
		return sql_word::DOUBLE;
	else if (s == "DROP")
		return sql_word::DROP;
	else if (s == "ELSE")
		return sql_word::ELSE;
	else if (s == "END")
		return sql_word::END;
	else if (s == "ERRLVL")
		return sql_word::ERRLVL;
	else if (s == "ESCAPE")
		return sql_word::ESCAPE;
	else if (s == "EXCEPT")
		return sql_word::EXCEPT;
	else if (s == "EXEC")
		return sql_word::EXEC;
	else if (s == "EXECUTE")
		return sql_word::EXECUTE;
	else if (s == "EXISTS")
		return sql_word::EXISTS;
	else if (s == "EXIT")
		return sql_word::EXIT;
	else if (s == "EXTERNAL")
		return sql_word::EXTERNAL;
	else if (s == "FETCH")
		return sql_word::FETCH;
	else if (s == "FILE")
		return sql_word::FILE;
	else if (s == "FILLFACTOR")
		return sql_word::FILLFACTOR;
	else if (s == "FOR")
		return sql_word::FOR;
	else if (s == "FOREIGN")
		return sql_word::FOREIGN;
	else if (s == "FREETEXT")
		return sql_word::FREETEXT;
	else if (s == "FREETEXTTABLE")
		return sql_word::FREETEXTTABLE;
	else if (s == "FROM")
		return sql_word::FROM;
	else if (s == "FULL")
		return sql_word::FULL;
	else if (s == "FUNCTION")
		return sql_word::FUNCTION;
	else if (s == "GOTO")
		return sql_word::GOTO;
	else if (s == "GRANT")
		return sql_word::GRANT;
	else if (s == "GROUP")
		return sql_word::GROUP;
	else if (s == "HAVING")
		return sql_word::HAVING;
	else if (s == "HOLDLOCK")
		return sql_word::HOLDLOCK;
	else if (s == "IDENTITY")
		return sql_word::IDENTITY;
	else if (s == "IDENTITY_INSERT")
		return sql_word::IDENTITY_INSERT;
	else if (s == "IDENTITYCOL")
		return sql_word::IDENTITYCOL;
	else if (s == "IF")
		return sql_word::IF;
	else if (s == "IN")
		return sql_word::SQL_IN;
	else if (s == "INDEX")
		return sql_word::INDEX;
	else if (s == "INNER")
		return sql_word::INNER;
	else if (s == "INSERT")
		return sql_word::INSERT;
	else if (s == "INTERSECT")
		return sql_word::INTERSECT;
	else if (s == "INTO")
		return sql_word::INTO;
	else if (s == "IS")
		return sql_word::IS;
	else if (s == "JOIN")
		return sql_word::JOIN;
	else if (s == "KEY")
		return sql_word::KEY;
	else if (s == "KILL")
		return sql_word::KILL;
	else if (s == "LEFT")
		return sql_word::LEFT;
	else if (s == "LIKE")
		return sql_word::LIKE;
	else if (s == "LINENO")
		return sql_word::LINENO;
	else if (s == "MERGE")
		return sql_word::MERGE;
	else if (s == "NATIONAL")
		return sql_word::NATIONAL;
	else if (s == "NOCHECK")
		return sql_word::NOCHECK;
	else if (s == "NONCLUSTERED")
		return sql_word::NONCLUSTERED;
	else if (s == "NOT")
		return sql_word::NOT;
	else if (s == "NULL")
		return sql_word::SQL_NULL;
	else if (s == "NULLIF")
		return sql_word::NULLIF;
	else if (s == "OF")
		return sql_word::OF;
	else if (s == "OFF")
		return sql_word::OFF;
	else if (s == "OFFSETS")
		return sql_word::OFFSETS;
	else if (s == "ON")
		return sql_word::ON;
	else if (s == "OPEN")
		return sql_word::OPEN;
	else if (s == "OPENDATASOURCE")
		return sql_word::OPENDATASOURCE;
	else if (s == "OPENQUERY")
		return sql_word::OPENQUERY;
	else if (s == "OPENROWSET")
		return sql_word::OPENROWSET;
	else if (s == "OPENXML")
		return sql_word::OPENXML;
	else if (s == "OPTION")
		return sql_word::OPTION;
	else if (s == "OR")
		return sql_word::OR;
	else if (s == "ORDER")
		return sql_word::ORDER;
	else if (s == "OUTER")
		return sql_word::OUTER;
	else if (s == "OVER")
		return sql_word::OVER;
	else if (s == "PERCENT")
		return sql_word::PERCENT;
	else if (s == "PIVOT")
		return sql_word::PIVOT;
	else if (s == "PLAN")
		return sql_word::PLAN;
	else if (s == "PRIMARY")
		return sql_word::PRIMARY;
	else if (s == "PRINT")
		return sql_word::PRINT;
	else if (s == "PROC" || s == "PROCEDURE")
		return sql_word::PROCEDURE;
	else if (s == "PUBLIC")
		return sql_word::PUBLIC;
	else if (s == "RAISERROR")
		return sql_word::RAISERROR;
	else if (s == "READ")
		return sql_word::READ;
	else if (s == "READTEXT")
		return sql_word::READTEXT;
	else if (s == "RECONFIGURE")
		return sql_word::RECONFIGURE;
	else if (s == "REFERENCES")
		return sql_word::REFERENCES;
	else if (s == "REPLICATION")
		return sql_word::REPLICATION;
	else if (s == "RESTORE")
		return sql_word::RESTORE;
	else if (s == "RESTRICT")
		return sql_word::RESTRICT;
	else if (s == "RETURN")
		return sql_word::RETURN;
	else if (s == "REVERT")
		return sql_word::REVERT;
	else if (s == "REVOKE")
		return sql_word::REVOKE;
	else if (s == "RIGHT")
		return sql_word::RIGHT;
	else if (s == "ROLLBACK")
		return sql_word::ROLLBACK;
	else if (s == "ROWCOUNT")
		return sql_word::ROWCOUNT;
	else if (s == "ROWGUIDCOL")
		return sql_word::ROWGUIDCOL;
	else if (s == "RULE")
		return sql_word::RULE;
	else if (s == "SAVE")
		return sql_word::SAVE;
	else if (s == "SCHEMA")
		return sql_word::SCHEMA;
	else if (s == "SELECT")
		return sql_word::SELECT;
	else if (s == "SEMANTICKEYPHRASETABLE")
		return sql_word::SEMANTICKEYPHRASETABLE;
	else if (s == "SEMANTICSIMILARITYDETAILSTABLE")
		return sql_word::SEMANTICSIMILARITYDETAILSTABLE;
	else if (s == "SEMANTICSIMILARITYTABLE")
		return sql_word::SEMANTICSIMILARITYTABLE;
	else if (s == "SESSION_USER")
		return sql_word::SESSION_USER;
	else if (s == "SET")
		return sql_word::SET;
	else if (s == "SETUSER")
		return sql_word::SETUSER;
	else if (s == "SHUTDOWN")
		return sql_word::SHUTDOWN;
	else if (s == "SOME")
		return sql_word::SOME;
	else if (s == "STATISTICS")
		return sql_word::STATISTICS;
	else if (s == "SYSTEM_USER")
		return sql_word::SYSTEM_USER;
	else if (s == "TABLE")
		return sql_word::TABLE;
	else if (s == "TABLESAMPLE")
		return sql_word::TABLESAMPLE;
	else if (s == "TEXTSIZE")
		return sql_word::TEXTSIZE;
	else if (s == "THEN")
		return sql_word::THEN;
	else if (s == "TO")
		return sql_word::TO;
	else if (s == "TOP")
		return sql_word::TOP;
	else if (s == "TRAN")
		return sql_word::TRAN;
	else if (s == "TRANSACTION")
		return sql_word::TRANSACTION;
	else if (s == "TRIGGER")
		return sql_word::TRIGGER;
	else if (s == "TRUNCATE")
		return sql_word::TRUNCATE;
	else if (s == "TRY_CONVERT")
		return sql_word::TRY_CONVERT;
	else if (s == "TSEQUAL")
		return sql_word::TSEQUAL;
	else if (s == "UNION")
		return sql_word::UNION;
	else if (s == "UNIQUE")
		return sql_word::UNIQUE;
	else if (s == "UNPIVOT")
		return sql_word::UNPIVOT;
	else if (s == "UPDATE")
		return sql_word::UPDATE;
	else if (s == "UPDATETEXT")
		return sql_word::UPDATETEXT;
	else if (s == "USE")
		return sql_word::USE;
	else if (s == "USER")
		return sql_word::USER;
	else if (s == "VALUES")
		return sql_word::VALUES;
	else if (s == "VARYING")
		return sql_word::VARYING;
	else if (s == "VIEW")
		return sql_word::VIEW;
	else if (s == "WAITFOR")
		return sql_word::WAITFOR;
	else if (s == "WHEN")
		return sql_word::WHEN;
	else if (s == "WHERE")
		return sql_word::WHERE;
	else if (s == "WHILE")
		return sql_word::WHILE;
	else if (s == "WITH")
		return sql_word::WITH;
	else if (s == "WRITETEXT")
		return sql_word::WRITETEXT;

	return sql_word::identifier;
}

static enum sql_word next_word(string_view& sv) {
	if (sv.empty())
		return sql_word::whitespace;

	switch (sv.front()) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			sv = sv.substr(1);

			while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r' || sv.front() == '\n')) {
				sv = sv.substr(1);
			}

			return sql_word::whitespace;

		case '(':
			sv = sv.substr(1);
			return sql_word::open_bracket;

		case ')':
			sv = sv.substr(1);
			return sql_word::close_bracket;

		case ',':
			sv = sv.substr(1);
			return sql_word::comma;

		case ';':
			sv = sv.substr(1);
			return sql_word::semicolon;

		case '.':
			sv = sv.substr(1);
			return sql_word::full_stop;

		case '-':
			if (sv.length() >= 2 && sv[1] == '-') {
				sv = sv.substr(2);

				while (!sv.empty() && sv.front() != '\n') {
					sv = sv.substr(1);
				}

				return sql_word::comment;
			}

			sv = sv.substr(1);
			return sql_word::minus;

		case '/':
			if (sv.length() >= 2 && sv[1] == '*') {
				unsigned int level = 1;

				sv = sv.substr(2);

				while (!sv.empty()) {
					if (sv.length() >= 2 && sv[0] == '/' && sv[1] == '*') {
						level++;
						sv = sv.substr(2);
					} else if (sv.length() >= 2 && sv[0] == '*' && sv[1] == '/') {
						level--;
						sv = sv.substr(2);

						if (level == 0)
							return sql_word::comment;
					} else
						sv = sv.substr(1);
				}

				return sql_word::comment;
			}

			sv = sv.substr(1);
			return sql_word::slash;

		case '=':
			sv = sv.substr(1);
			return sql_word::equals;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case '_':
		case '@':
		case '#': {
			auto word = sv;
			bool uc = true, non_letter = false;

			// can't have number or dollar sign at beginning

			for (size_t i = 0; i < word.length(); i++) {
				if ((word[i] < 'A' || word[i] > 'Z') && (word[i] < 'a' || word[i] > 'z') && (word[i] < '0' || word[i] > '9') && word[i] != '_' && word[i] != '@' && word[i] != '#' && word[i] != '$') {
					word = word.substr(0, i);
					break;
				}

				if (word[i] < 'A' || word[i] > 'Z') {
					uc = false;

					if (word[i] < 'a' || word[i] > 'z')
						non_letter = true;
				}
			}

			sv = sv.substr(word.length());

			if (non_letter)
				return sql_word::identifier;

			if (uc)
				return identify_word(word);
			else {
				string s(word.length(), 0);

				for (size_t i = 0; i < word.length(); i++) {
					if (word[i] >= 'A' && word[i] <= 'Z')
						s[i] = word[i];
					else
						s[i] = word[i] + 'A' - 'a';
				}

				return identify_word(s);
			}
		}

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			sv = sv.substr(1);

			while (!sv.empty() && (sv.front() >= '0' && sv.front() <= '9')) {
				sv = sv.substr(1);
			}

			return sql_word::number;

		case '\'':
			sv = sv.substr(1);

			while (!sv.empty()) {
				if (sv.front() == '\'') {
					if (sv.length() >= 2 && sv[1] == '\'')
						sv = sv.substr(2);
					else {
						sv = sv.substr(1);
						return sql_word::string;
					}
				} else
					sv = sv.substr(1);
			}

			return sql_word::string;

		case '[':
			sv = sv.substr(1);

			while (!sv.empty()) {
				if (sv.front() == ']') {
					if (sv.length() >= 2 && sv[1] == ']')
						sv = sv.substr(2);
					else {
						sv = sv.substr(1);
						return sql_word::identifier;
					}
				} else
					sv = sv.substr(1);
			}

			return sql_word::identifier;
	}

	sv = "";

	return sql_word::unknown;
}
