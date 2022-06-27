#include "gitsql.h"

using namespace std;

static enum lex identify_word(string_view s) {
	if (s == "ADD")
		return lex::ADD;
	else if (s == "ALL")
		return lex::ALL;
	else if (s == "ALTER")
		return lex::ALTER;
	else if (s == "AND")
		return lex::AND;
	else if (s == "ANY")
		return lex::ANY;
	else if (s == "AS")
		return lex::AS;
	else if (s == "ASC")
		return lex::ASC;
	else if (s == "AUTHORIZATION")
		return lex::AUTHORIZATION;
	else if (s == "BACKUP")
		return lex::BACKUP;
	else if (s == "BEGIN")
		return lex::BEGIN;
	else if (s == "BETWEEN")
		return lex::BETWEEN;
	else if (s == "BREAK")
		return lex::BREAK;
	else if (s == "BROWSE")
		return lex::BROWSE;
	else if (s == "BULK")
		return lex::BULK;
	else if (s == "BY")
		return lex::BY;
	else if (s == "CASCADE")
		return lex::CASCADE;
	else if (s == "CASE")
		return lex::CASE;
	else if (s == "CHECK")
		return lex::CHECK;
	else if (s == "CHECKPOINT")
		return lex::CHECKPOINT;
	else if (s == "CLOSE")
		return lex::CLOSE;
	else if (s == "CLUSTERED")
		return lex::CLUSTERED;
	else if (s == "COALESCE")
		return lex::COALESCE;
	else if (s == "COLLATE")
		return lex::COLLATE;
	else if (s == "COLUMN")
		return lex::COLUMN;
	else if (s == "COMMIT")
		return lex::COMMIT;
	else if (s == "COMPUTE")
		return lex::COMPUTE;
	else if (s == "CONSTRAINT")
		return lex::CONSTRAINT;
	else if (s == "CONTAINS")
		return lex::CONTAINS;
	else if (s == "CONTAINSTABLE")
		return lex::CONTAINSTABLE;
	else if (s == "CONTINUE")
		return lex::CONTINUE;
	else if (s == "CONVERT")
		return lex::CONVERT;
	else if (s == "CREATE")
		return lex::CREATE;
	else if (s == "CROSS")
		return lex::CROSS;
	else if (s == "CURRENT")
		return lex::CURRENT;
	else if (s == "CURRENT_DATE")
		return lex::CURRENT_DATE;
	else if (s == "CURRENT_TIME")
		return lex::CURRENT_TIME;
	else if (s == "CURRENT_TIMESTAMP")
		return lex::CURRENT_TIMESTAMP;
	else if (s == "CURRENT_USER")
		return lex::CURRENT_USER;
	else if (s == "CURSOR")
		return lex::CURSOR;
	else if (s == "DATABASE")
		return lex::DATABASE;
	else if (s == "DBCC")
		return lex::DBCC;
	else if (s == "DEALLOCATE")
		return lex::DEALLOCATE;
	else if (s == "DECLARE")
		return lex::DECLARE;
	else if (s == "DEFAULT")
		return lex::DEFAULT;
	else if (s == "DELETE")
		return lex::SQL_DELETE;
	else if (s == "DENY")
		return lex::DENY;
	else if (s == "DESC")
		return lex::DESC;
	else if (s == "DISTINCT")
		return lex::DISTINCT;
	else if (s == "DISTRIBUTED")
		return lex::DISTRIBUTED;
	else if (s == "DOUBLE")
		return lex::DOUBLE;
	else if (s == "DROP")
		return lex::DROP;
	else if (s == "ELSE")
		return lex::ELSE;
	else if (s == "END")
		return lex::END;
	else if (s == "ERRLVL")
		return lex::ERRLVL;
	else if (s == "ESCAPE")
		return lex::ESCAPE;
	else if (s == "EXCEPT")
		return lex::EXCEPT;
	else if (s == "EXEC")
		return lex::EXEC;
	else if (s == "EXECUTE")
		return lex::EXECUTE;
	else if (s == "EXISTS")
		return lex::EXISTS;
	else if (s == "EXIT")
		return lex::EXIT;
	else if (s == "EXTERNAL")
		return lex::EXTERNAL;
	else if (s == "FETCH")
		return lex::FETCH;
	else if (s == "FILE")
		return lex::FILE;
	else if (s == "FILLFACTOR")
		return lex::FILLFACTOR;
	else if (s == "FOR")
		return lex::FOR;
	else if (s == "FOREIGN")
		return lex::FOREIGN;
	else if (s == "FREETEXT")
		return lex::FREETEXT;
	else if (s == "FREETEXTTABLE")
		return lex::FREETEXTTABLE;
	else if (s == "FROM")
		return lex::FROM;
	else if (s == "FULL")
		return lex::FULL;
	else if (s == "FUNCTION")
		return lex::FUNCTION;
	else if (s == "GOTO")
		return lex::GOTO;
	else if (s == "GRANT")
		return lex::GRANT;
	else if (s == "GROUP")
		return lex::GROUP;
	else if (s == "HAVING")
		return lex::HAVING;
	else if (s == "HOLDLOCK")
		return lex::HOLDLOCK;
	else if (s == "IDENTITY")
		return lex::IDENTITY;
	else if (s == "IDENTITY_INSERT")
		return lex::IDENTITY_INSERT;
	else if (s == "IDENTITYCOL")
		return lex::IDENTITYCOL;
	else if (s == "IF")
		return lex::IF;
	else if (s == "IN")
		return lex::SQL_IN;
	else if (s == "INDEX")
		return lex::INDEX;
	else if (s == "INNER")
		return lex::INNER;
	else if (s == "INSERT")
		return lex::INSERT;
	else if (s == "INTERSECT")
		return lex::INTERSECT;
	else if (s == "INTO")
		return lex::INTO;
	else if (s == "IS")
		return lex::IS;
	else if (s == "JOIN")
		return lex::JOIN;
	else if (s == "KEY")
		return lex::KEY;
	else if (s == "KILL")
		return lex::KILL;
	else if (s == "LEFT")
		return lex::LEFT;
	else if (s == "LIKE")
		return lex::LIKE;
	else if (s == "LINENO")
		return lex::LINENO;
	else if (s == "MERGE")
		return lex::MERGE;
	else if (s == "NATIONAL")
		return lex::NATIONAL;
	else if (s == "NOCHECK")
		return lex::NOCHECK;
	else if (s == "NONCLUSTERED")
		return lex::NONCLUSTERED;
	else if (s == "NOT")
		return lex::NOT;
	else if (s == "NULL")
		return lex::SQL_NULL;
	else if (s == "NULLIF")
		return lex::NULLIF;
	else if (s == "OF")
		return lex::OF;
	else if (s == "OFF")
		return lex::OFF;
	else if (s == "OFFSETS")
		return lex::OFFSETS;
	else if (s == "ON")
		return lex::ON;
	else if (s == "OPEN")
		return lex::OPEN;
	else if (s == "OPENDATASOURCE")
		return lex::OPENDATASOURCE;
	else if (s == "OPENQUERY")
		return lex::OPENQUERY;
	else if (s == "OPENROWSET")
		return lex::OPENROWSET;
	else if (s == "OPENXML")
		return lex::OPENXML;
	else if (s == "OPTION")
		return lex::OPTION;
	else if (s == "OR")
		return lex::OR;
	else if (s == "ORDER")
		return lex::ORDER;
	else if (s == "OUTER")
		return lex::OUTER;
	else if (s == "OVER")
		return lex::OVER;
	else if (s == "PERCENT")
		return lex::PERCENT;
	else if (s == "PIVOT")
		return lex::PIVOT;
	else if (s == "PLAN")
		return lex::PLAN;
	else if (s == "PRIMARY")
		return lex::PRIMARY;
	else if (s == "PRINT")
		return lex::PRINT;
	else if (s == "PROC" || s == "PROCEDURE")
		return lex::PROCEDURE;
	else if (s == "PUBLIC")
		return lex::PUBLIC;
	else if (s == "RAISERROR")
		return lex::RAISERROR;
	else if (s == "READ")
		return lex::READ;

	if (s == "READTEXT")
		return lex::READTEXT;
	else if (s == "RECONFIGURE")
		return lex::RECONFIGURE;
	else if (s == "REFERENCES")
		return lex::REFERENCES;
	else if (s == "REPLICATION")
		return lex::REPLICATION;
	else if (s == "RESTORE")
		return lex::RESTORE;
	else if (s == "RESTRICT")
		return lex::RESTRICT;
	else if (s == "RETURN")
		return lex::RETURN;
	else if (s == "REVERT")
		return lex::REVERT;
	else if (s == "REVOKE")
		return lex::REVOKE;
	else if (s == "RIGHT")
		return lex::RIGHT;
	else if (s == "ROLLBACK")
		return lex::ROLLBACK;
	else if (s == "ROWCOUNT")
		return lex::ROWCOUNT;
	else if (s == "ROWGUIDCOL")
		return lex::ROWGUIDCOL;
	else if (s == "RULE")
		return lex::RULE;
	else if (s == "SAVE")
		return lex::SAVE;
	else if (s == "SCHEMA")
		return lex::SCHEMA;
	else if (s == "SELECT")
		return lex::SELECT;
	else if (s == "SEMANTICKEYPHRASETABLE")
		return lex::SEMANTICKEYPHRASETABLE;
	else if (s == "SEMANTICSIMILARITYDETAILSTABLE")
		return lex::SEMANTICSIMILARITYDETAILSTABLE;
	else if (s == "SEMANTICSIMILARITYTABLE")
		return lex::SEMANTICSIMILARITYTABLE;
	else if (s == "SESSION_USER")
		return lex::SESSION_USER;
	else if (s == "SET")
		return lex::SET;
	else if (s == "SETUSER")
		return lex::SETUSER;
	else if (s == "SHUTDOWN")
		return lex::SHUTDOWN;
	else if (s == "SOME")
		return lex::SOME;
	else if (s == "STATISTICS")
		return lex::STATISTICS;
	else if (s == "SYSTEM_USER")
		return lex::SYSTEM_USER;
	else if (s == "TABLE")
		return lex::TABLE;
	else if (s == "TABLESAMPLE")
		return lex::TABLESAMPLE;
	else if (s == "TEXTSIZE")
		return lex::TEXTSIZE;
	else if (s == "THEN")
		return lex::THEN;
	else if (s == "TO")
		return lex::TO;
	else if (s == "TOP")
		return lex::TOP;
	else if (s == "TRAN")
		return lex::TRAN;
	else if (s == "TRANSACTION")
		return lex::TRANSACTION;
	else if (s == "TRIGGER")
		return lex::TRIGGER;
	else if (s == "TRUNCATE")
		return lex::TRUNCATE;
	else if (s == "TRY_CONVERT")
		return lex::TRY_CONVERT;
	else if (s == "TSEQUAL")
		return lex::TSEQUAL;
	else if (s == "UNION")
		return lex::UNION;
	else if (s == "UNIQUE")
		return lex::UNIQUE;
	else if (s == "UNPIVOT")
		return lex::UNPIVOT;
	else if (s == "UPDATE")
		return lex::UPDATE;
	else if (s == "UPDATETEXT")
		return lex::UPDATETEXT;
	else if (s == "USE")
		return lex::USE;
	else if (s == "USER")
		return lex::USER;
	else if (s == "VALUES")
		return lex::VALUES;
	else if (s == "VARYING")
		return lex::VARYING;
	else if (s == "VIEW")
		return lex::VIEW;
	else if (s == "WAITFOR")
		return lex::WAITFOR;
	else if (s == "WHEN")
		return lex::WHEN;
	else if (s == "WHERE")
		return lex::WHERE;
	else if (s == "WHILE")
		return lex::WHILE;
	else if (s == "WITH")
		return lex::WITH;
	else if (s == "WRITETEXT")
		return lex::WRITETEXT;

	return lex::identifier;
}

static pair<enum lex, string_view> next_word(string_view sv) {
	if (sv.empty())
		return {lex::whitespace, ""};

	enum lex w;
	string_view word;

	switch (sv.front()) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			word = sv;

			for (size_t i = 1; i < word.length(); i++) {
				if (word[i] != ' ' && word[i] != '\t' && word[i] != '\r' && word[i] != '\n') {
					word = word.substr(0, i);
					break;
				}
			}

			w = lex::whitespace;
			break;

		case '(':
			word = sv.substr(0, 1);
			w = lex::open_bracket;
			break;

		case ')':
			word = sv.substr(0, 1);
			w = lex::close_bracket;
			break;

		case ',':
			word = sv.substr(0, 1);
			w = lex::comma;
			break;

		case ';':
			word = sv.substr(0, 1);
			w = lex::semicolon;
			break;

		case '.':
			word = sv.substr(0, 1);
			w = lex::full_stop;
			break;

		case '-':
			word = sv;

			if (sv.length() >= 2 && sv[1] == '-') {
				for (size_t i = 2; i < word.length(); i++) {
					if (word[i] == '\n') {
						word = word.substr(0, i);
						break;
					}
				}

				w = lex::comment;
				break;
			}

			word = sv.substr(0, 1);
			w = lex::minus;
			break;

		case '/':
			if (sv.length() >= 2 && sv[1] == '*') {
				unsigned int level = 1;

				word = sv;

				for (size_t i = 2; i < word.length(); i++) {
					if (i < word.length() - 1) {
						if (word[i] == '/' && word[i + 1] == '*') {
							level++;
							i++;
						} else if (word[i] == '*' && word[i + 1] == '/') {
							level--;
							i++;

							if (level == 0) {
								word = word.substr(0, i + 1);
								break;
							}
						}
					}
				}

				w = lex::comment;
				break;
			}

			word = sv.substr(0, 1);
			w = lex::slash;
			break;

		case '=':
			word = sv.substr(0, 1);
			w = lex::equals;
			break;

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
			bool uc = true, non_letter = false;

			word = sv;

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

			if (non_letter) {
				w = lex::identifier;
				break;
			}

			if (uc)
				w = identify_word(word);
			else {
				string s(word.length(), 0);

				for (size_t i = 0; i < word.length(); i++) {
					if (word[i] >= 'A' && word[i] <= 'Z')
						s[i] = word[i];
					else
						s[i] = word[i] + 'A' - 'a';
				}

				w = identify_word(s);
			}

			break;
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
			word = sv;

			for (size_t i = 1; i < word.length(); i++) {
				if (word[i] < '0' || word[i] > '9') {
					word = word.substr(0, i);
					break;
				}
			}

			w = lex::number;
			break;

		case '\'':
			word = sv;

			for (size_t i = 1; i < word.length(); i++) {
				if (word[i] == '\'') {
					if (i < word.length() - 1 && word[i + 1] == '\'')
						i++;
					else {
						word = word.substr(0, i + 1);
						break;
					}
				}
			}

			w = lex::string;
			break;

		case '[':
			word = sv;

			for (size_t i = 1; i < word.length(); i++) {
				if (word[i] == ']') {
					if (i < word.length() - 1 && word[i + 1] == ']')
						i++;
					else {
						word = word.substr(0, i + 1);
						break;
					}
				}
			}

			w = lex::identifier;
			break;

		default:
			word = sv;
			w = lex::unknown;
	}

	return {w, word};
}

static string_view strip_preamble(string_view& sv) {
	auto s = sv;

	while (!s.empty()) {
		auto w = next_word(s);

		if (w.first != lex::comment && w.first != lex::whitespace)
			break;

		s = s.substr(w.second.length());
	}

	auto preamble = sv.substr(0, sv.length() - s.length());

	sv = s;

	return preamble;
}

static void skip_whitespace(string_view& sv) {
	if (sv.empty())
		return;

	do {
		auto w = next_word(sv);

		if (w.first != lex::whitespace && w.first != lex::comment)
			break;

		sv = sv.substr(w.second.length());
	} while (true);
}

string munge_definition(string_view sql, string_view schema, string_view name,
						enum lex type) {
	auto sv = sql;

	strip_preamble(sv);

	if (sv.empty())
		return string{sql};

	auto w = next_word(sv);

	if (w.first != lex::CREATE)
		return string{sql};

	auto create = w.second;
	sv = sv.substr(w.second.length());

	skip_whitespace(sv);

	if (sv.empty())
		return string{sql};

	w = next_word(sv);

	if (w.first != type)
		return string{sql};

	sv = sv.substr(w.second.length());

	skip_whitespace(sv);

	if (sv.empty())
		return string{sql};

	w = next_word(sv);

	if (w.first != lex::identifier)
		return string{sql};

	sv = sv.substr(w.second.length());

	{
		auto sv2 = sv;
		bool two_part = false;

		skip_whitespace(sv);

		if (!sv.empty()) {
			w = next_word(sv);

			if (w.first == lex::full_stop) {
				sv = sv.substr(w.second.length());
				skip_whitespace(sv);

				if (!sv.empty()) {
					w = next_word(sv);

					if (w.first == lex::identifier) {
						sv = sv.substr(w.second.length());
						two_part = true;
					}
				}
			}
		}

		if (!two_part)
			sv = sv2;
	}

	string sql2{string_view(sql).substr(0, create.data() - sql.data())};

	if (type == lex::PROCEDURE)
		sql2.append("CREATE OR ALTER PROCEDURE ");
	else if (type == lex::VIEW)
		sql2.append("CREATE OR ALTER VIEW ");
	else if (type == lex::FUNCTION)
		sql2.append("CREATE OR ALTER FUNCTION ");

	sql2.append(brackets_escape(schema));
	sql2.append(".");
	sql2.append(brackets_escape(name));
	sql2.append(sv);

	return sql2;
}
