#include <string>
#include <optional>
#include <vector>
#include <stdexcept>
#include <tdscpp.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <codecvt>
#endif

#include "gitsql.h"

using namespace std;

static const vector<string_view> reserved_words{"ADD", "ALL", "ALTER", "AND", "ANY", "AS", "ASC", "AUTHORIZATION", "BACKUP", "BEGIN", "BETWEEN", "BREAK", "BROWSE", "BULK", "BY", "CASCADE", "CASE", "CHECK", "CHECKPOINT", "CLOSE", "CLUSTERED", "COALESCE", "COLLATE", "COLUMN", "COMMIT", "COMPUTE", "CONSTRAINT", "CONTAINS", "CONTAINSTABLE", "CONTINUE", "CONVERT", "CREATE", "CROSS", "CURRENT", "CURRENT_DATE", "CURRENT_TIME", "CURRENT_TIMESTAMP", "CURRENT_USER", "CURSOR", "DATABASE", "DBCC", "DEALLOCATE", "DECLARE", "DEFAULT", "DELETE", "DENY", "DESC", "DISTINCT", "DISTRIBUTED", "DOUBLE", "DROP", "ELSE", "END", "ERRLVL", "ESCAPE", "EXCEPT", "EXEC", "EXECUTE", "EXISTS", "EXIT", "EXTERNAL", "FETCH", "FILE", "FILLFACTOR", "FOR", "FOREIGN", "FREETEXT", "FREETEXTTABLE", "FROM", "FULL", "FUNCTION", "GOTO", "GRANT", "GROUP", "HAVING", "HOLDLOCK", "IDENTITY", "IDENTITY_INSERT", "IDENTITYCOL", "IF", "IN", "INDEX", "INNER", "INSERT", "INTERSECT", "INTO", "IS", "JOIN", "KEY", "KILL", "LEFT", "LIKE", "LINENO", "MERGE", "NATIONAL", "NOCHECK", "NONCLUSTERED", "NOT", "NULL", "NULLIF", "OF", "OFF", "OFFSETS", "ON", "OPEN", "OPENDATASOURCE", "OPENQUERY", "OPENROWSET", "OPENXML", "OPTION", "OR", "ORDER", "OUTER", "OVER", "PERCENT", "PIVOT", "PLAN", "PRIMARY", "PRINT", "PROC", "PROCEDURE", "PUBLIC", "RAISERROR", "READ", "READTEXT", "RECONFIGURE", "REFERENCES", "REPLICATION", "RESTORE", "RESTRICT", "RETURN", "REVERT", "REVOKE", "RIGHT", "ROLLBACK", "ROWCOUNT", "ROWGUIDCOL", "RULE", "SAVE", "SCHEMA", "SELECT", "SEMANTICKEYPHRASETABLE", "SEMANTICSIMILARITYDETAILSTABLE", "SEMANTICSIMILARITYTABLE", "SESSION_USER", "SET", "SETUSER", "SHUTDOWN", "SOME", "STATISTICS", "SYSTEM_USER", "TABLE", "TABLESAMPLE", "TEXTSIZE", "THEN", "TO", "TOP", "TRAN", "TRANSACTION", "TRIGGER", "TRUNCATE", "TRY_CONVERT", "TSEQUAL", "UNION", "UNIQUE", "UNPIVOT", "UPDATE", "UPDATETEXT", "USE", "USER", "VALUES", "VARYING", "VIEW", "WAITFOR", "WHEN", "WHERE", "WHILE", "WITH", "WRITETEXT"};

static const vector<u16string_view> reserved_words16{u"ADD", u"ALL", u"ALTER", u"AND", u"ANY", u"AS", u"ASC", u"AUTHORIZATION", u"BACKUP", u"BEGIN", u"BETWEEN", u"BREAK", u"BROWSE", u"BULK", u"BY", u"CASCADE", u"CASE", u"CHECK", u"CHECKPOINT", u"CLOSE", u"CLUSTERED", u"COALESCE", u"COLLATE", u"COLUMN", u"COMMIT", u"COMPUTE", u"CONSTRAINT", u"CONTAINS", u"CONTAINSTABLE", u"CONTINUE", u"CONVERT", u"CREATE", u"CROSS", u"CURRENT", u"CURRENT_DATE", u"CURRENT_TIME", u"CURRENT_TIMESTAMP", u"CURRENT_USER", u"CURSOR", u"DATABASE", u"DBCC", u"DEALLOCATE", u"DECLARE", u"DEFAULT", u"DELETE", u"DENY", u"DESC", u"DISTINCT", u"DISTRIBUTED", u"DOUBLE", u"DROP", u"ELSE", u"END", u"ERRLVL", u"ESCAPE", u"EXCEPT", u"EXEC", u"EXECUTE", u"EXISTS", u"EXIT", u"EXTERNAL", u"FETCH", u"FILE", u"FILLFACTOR", u"FOR", u"FOREIGN", u"FREETEXT", u"FREETEXTTABLE", u"FROM", u"FULL", u"FUNCTION", u"GOTO", u"GRANT", u"GROUP", u"HAVING", u"HOLDLOCK", u"IDENTITY", u"IDENTITY_INSERT", u"IDENTITYCOL", u"IF", u"IN", u"INDEX", u"INNER", u"INSERT", u"INTERSECT", u"INTO", u"IS", u"JOIN", u"KEY", u"KILL", u"LEFT", u"LIKE", u"LINENO", u"MERGE", u"NATIONAL", u"NOCHECK", u"NONCLUSTERED", u"NOT", u"NULL", u"NULLIF", u"OF", u"OFF", u"OFFSETS", u"ON", u"OPEN", u"OPENDATASOURCE", u"OPENQUERY", u"OPENROWSET", u"OPENXML", u"OPTION", u"OR", u"ORDER", u"OUTER", u"OVER", u"PERCENT", u"PIVOT", u"PLAN", u"PRIMARY", u"PRINT", u"PROC", u"PROCEDURE", u"PUBLIC", u"RAISERROR", u"READ", u"READTEXT", u"RECONFIGURE", u"REFERENCES", u"REPLICATION", u"RESTORE", u"RESTRICT", u"RETURN", u"REVERT", u"REVOKE", u"RIGHT", u"ROLLBACK", u"ROWCOUNT", u"ROWGUIDCOL", u"RULE", u"SAVE", u"SCHEMA", u"SELECT", u"SEMANTICKEYPHRASETABLE", u"SEMANTICSIMILARITYDETAILSTABLE", u"SEMANTICSIMILARITYTABLE", u"SESSION_USER", u"SET", u"SETUSER", u"SHUTDOWN", u"SOME", u"STATISTICS", u"SYSTEM_USER", u"TABLE", u"TABLESAMPLE", u"TEXTSIZE", u"THEN", u"TO", u"TOP", u"TRAN", u"TRANSACTION", u"TRIGGER", u"TRUNCATE", u"TRY_CONVERT", u"TSEQUAL", u"UNION", u"UNIQUE", u"UNPIVOT", u"UPDATE", u"UPDATETEXT", u"USE", u"USER", u"VALUES", u"VARYING", u"VIEW", u"WAITFOR", u"WHEN", u"WHERE", u"WHILE", u"WITH", u"WRITETEXT"};

template<typename T>
static bool is_reserved_word(const basic_string_view<T>& sv) {
	auto s = basic_string<T>(sv);

	for (auto& c : s) {
		if (c >= 'a' && c <= 'z')
			c += 'A' - 'a';
		else if (c < 'A' || c > 'Z')
			return false;
	}

	if constexpr (is_same_v<T, char>) {
		for (const auto& w : reserved_words) {
			if (w == s)
				return true;
		}
	} else if constexpr (is_same_v<T, char16_t>) {
		for (const auto& w : reserved_words16) {
			if (w == s)
				return true;
		}
	}

	return false;
}

template<typename T>
static bool need_escaping(const basic_string_view<T>& s) {
	// see https://docs.microsoft.com/en-us/previous-versions/sql/sql-server-2008-r2/ms175874(v=sql.105)

	if (s.empty())
		return true;

	for (auto c : s) {
		if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_' && c != '@' && c != '#' && c != '$')
			return true;
	}

	// can't have number or dollar sign at beginning

	if ((s.front() >= '0' && s.front() <= '9') || s.front() == '$')
		return true;

	return is_reserved_word(s);
}

template<typename T>
static basic_string<T> brackets_escape2(const basic_string_view<T>& s) {
	basic_string<T> ret;

	if (!need_escaping(s))
		return basic_string<T>(s);

	ret.reserve(s.length() + 2);
	ret.push_back('[');

	for (auto c : s) {
		if (c == ']') {
			ret.push_back(']');
			ret.push_back(']');
		} else
			ret.push_back(c);
	}

	ret.push_back(']');

	return ret;
}

string brackets_escape(string_view s) {
	return brackets_escape2(s);
}

u16string brackets_escape(const u16string_view& s) {
	return brackets_escape2(s);
}

struct column {
	column(const string& name, const string& type, int max_length, bool nullable, int precision,
		   int scale, const tds::value& def, int column_id, bool is_identity, bool is_computed,
		   bool is_persisted, const string& computed_definition, const tds::value& collation) :
		name(name), type(type), max_length(max_length), nullable(nullable), precision(precision),
		scale(scale), def(def.is_null ? optional<string>(nullopt) : optional<string>(def)),
		column_id(column_id), is_identity(is_identity),
		is_computed(is_computed), is_persisted(is_persisted), computed_definition(computed_definition),
		collation(collation.is_null ? optional<string>(nullopt) : optional<string>(collation)) { }

	string name, type;
	int max_length;
	bool nullable;
	int precision, scale;
	optional<string> def;
	unsigned int column_id;
	bool is_identity, is_computed, is_persisted;
	string computed_definition;
	optional<string> collation;
};

struct index_column {
	index_column(const column& col, bool is_desc, bool is_included) : col(col), is_desc(is_desc), is_included(is_included) { }

	const column& col;
	bool is_desc;
	bool is_included;
};

struct table_index {
	table_index(const string& name, unsigned int type, bool is_unique, bool is_primary_key) : name(name), type(type), is_unique(is_unique), is_primary_key(is_primary_key) { }

	string name;
	unsigned int type;
	bool is_unique, is_primary_key;
	vector<index_column> columns;
};

struct constraint {
	constraint(const string& definition, unsigned int column_id) : definition(definition), column_id(column_id) { }

	string definition;
	unsigned int column_id;
};

struct foreign_key_column {
	foreign_key_column(const column& col, const string& schema, const string& table, const string& other_column) : col(col), schema(schema), table(table), other_column(other_column) { }

	const column& col;
	string schema, table, other_column;
};

struct foreign_key {
	foreign_key(unsigned int delete_referential_action, unsigned int update_referential_action) : delete_referential_action(delete_referential_action), update_referential_action(update_referential_action) { }

	unsigned int delete_referential_action, update_referential_action;
	vector<foreign_key_column> cols;
};

static void replace_all(string& source, string_view from, string_view to) {
	string new_string;
	new_string.reserve(source.length());

	string::size_type last_pos = 0;
	string::size_type find_pos;

	while ((find_pos = source.find(from, last_pos)) != string::npos) {
		new_string.append(source, last_pos, find_pos - last_pos);
		new_string += to;
		last_pos = find_pos + from.length();
	}

	new_string += source.substr(last_pos);

	source.swap(new_string);
}

static string quote_escape(string s) {
	replace_all(s, "'", "''");

	return s;
}

static string dump_table(tds::tds& tds, const string& escaped_name) {
	string cols, s;

	tds::query sq(tds, tds::no_check{"SELECT * FROM " + escaped_name});

	while (sq.fetch_row()) {
		string vals;
		auto column_count = sq.num_columns();

		if (cols.empty()) {
			for (uint16_t i = 0; i < column_count; i++) {
				const auto& col = sq[i];

				if (!cols.empty())
					cols += ", ";

				cols += brackets_escape(tds::utf16_to_utf8(col.name));
			}
		}

		s += "INSERT INTO " + escaped_name + "(" + cols + ") VALUES(";

		for (uint16_t i = 0; i < column_count; i++) {
			const auto& col = sq[i];

			if (vals != "")
				vals += ", ";

			if (col.is_null)
				vals += "NULL";
			else {
				switch (col.type) {
					case tds::sql_type::INTN:
					case tds::sql_type::TINYINT:
					case tds::sql_type::SMALLINT:
					case tds::sql_type::INT:
					case tds::sql_type::BIGINT:
					case tds::sql_type::BITN:
					case tds::sql_type::BIT:
						vals += to_string((int64_t)col);
						break;

					case tds::sql_type::FLOAT:
					case tds::sql_type::REAL:
					case tds::sql_type::FLTN:
						vals += to_string((double)col);
						break;

					// FIXME - VARBINARY

					default:
						vals += "'" + quote_escape(string(col)) + "'";
						break;
				}
			}
		}

		s += vals;

		s += ");\n";
	}

	return s;
}

string table_ddl(tds::tds& tds, int64_t id) {
	int64_t seed_value, increment_value;
	string table, schema, type;
	vector<column> columns;
	vector<table_index> indices;
	vector<constraint> constraints;
	vector<foreign_key> foreign_keys;
	string escaped_name, ddl;
	bool has_included_indices = false, fulldump = false;

	{
		tds::query sq(tds, R"(
SELECT objects.name,
	schemas.name,
	identity_columns.seed_value,
	identity_columns.increment_value,
	objects.type
FROM sys.objects
JOIN sys.schemas ON schemas.schema_id = objects.schema_id
LEFT JOIN sys.identity_columns ON identity_columns.object_id = objects.object_id
WHERE objects.object_id = ?
)", id);

		if (!sq.fetch_row())
			throw formatted_error("Cannot find name for object ID {}.", id);

		table = (string)sq[0];
		schema = (string)sq[1];
		seed_value = (int64_t)sq[2];
		increment_value = (int64_t)sq[3];
		type = (string)sq[4];
	}

	if (type == "TT") {
		tds::query sq(tds, "SELECT name, SCHEMA_NAME(schema_id) FROM sys.table_types WHERE type_table_object_id = ?", id);

		if (!sq.fetch_row())
			throw formatted_error("Could not find real name for table type {}.{}.", schema, table);

		table = (string)sq[0];
		schema = (string)sq[1];
	}

	{
		tds::query sq (tds, R"(
SELECT columns.name,
	CASE WHEN types.is_user_defined = 0 THEN UPPER(types.name) ELSE types.name END,
	columns.max_length,
	columns.is_nullable,
	columns.precision,
	columns.scale,
	default_constraints.definition,
	columns.column_id,
	columns.is_identity,
	columns.is_computed,
	computed_columns.is_persisted,
	computed_columns.definition,
	CASE WHEN columns.collation_name != CONVERT(VARCHAR(MAX),DATABASEPROPERTYEX(DB_NAME(), 'Collation')) THEN columns.collation_name END
FROM sys.columns
JOIN sys.types ON types.user_type_id = columns.user_type_id
LEFT JOIN sys.default_constraints ON default_constraints.parent_object_id = columns.object_id AND default_constraints.parent_column_id  = columns.column_id
LEFT JOIN sys.computed_columns ON computed_columns.object_id = columns.object_id AND computed_columns.column_id = columns.column_id
WHERE columns.object_id = ?
ORDER BY columns.column_id
)", id);

		while (sq.fetch_row()) {
			columns.emplace_back((string)sq[0], (string)sq[1], (int)sq[2], (int)sq[3] != 0, (int)sq[4], (int)sq[5],
								 sq[6], (unsigned int)sq[7], (unsigned int)sq[8] != 0, (unsigned int)sq[9] != 0,
								 (unsigned int)sq[10] != 0, (string)sq[11], sq[12]);
		}
	}

	{
		string last_name;

		tds::query sq(tds, "SELECT indexes.name, indexes.type, indexes.is_unique, indexes.is_primary_key, index_columns.column_id, index_columns.is_descending_key, index_columns.is_included_column FROM sys.indexes JOIN sys.index_columns ON index_columns.object_id = indexes.object_id AND index_columns.index_id = indexes.index_id WHERE indexes.object_id = ? ORDER BY indexes.index_id, index_columns.index_column_id", id);

		while (sq.fetch_row()) {
			bool found = false;
			auto col_id = (unsigned int)sq[4];

			if ((string)sq[0] != last_name) {
				last_name = (string)sq[0];
				indices.emplace_back(last_name, (unsigned int)sq[1], (unsigned int)sq[2] != 0, (unsigned int)sq[3] != 0);
			}

			for (const auto& col : columns) {
				if (col.column_id == col_id) {
					indices.back().columns.emplace_back(col, (unsigned int)sq[5] != 0, (unsigned int)sq[6] != 0);
					found = true;
					break;
				}
			}

			if (!found)
				throw formatted_error("Could not find column no. {}.", col_id);
		}
	}

	{
		tds::query sq(tds, "SELECT definition, parent_column_id FROM sys.check_constraints WHERE parent_object_id = ? ORDER BY name", id);

		while (sq.fetch_row()) {
			constraints.emplace_back((string)sq[0], (unsigned int)sq[1]);
		}
	}

	{
		int64_t last_num = 0;

		tds::query sq(tds, R"(
SELECT foreign_key_columns.constraint_object_id, foreign_key_columns.parent_column_id, OBJECT_SCHEMA_NAME(foreign_key_columns.referenced_object_id), OBJECT_NAME(foreign_key_columns.referenced_object_id), columns.name, foreign_keys.delete_referential_action, foreign_keys.update_referential_action
FROM sys.foreign_key_columns
JOIN sys.columns ON columns.object_id = foreign_key_columns.referenced_object_id AND columns.column_id = foreign_key_columns.referenced_column_id
JOIN sys.foreign_keys ON foreign_keys.object_id = foreign_key_columns.constraint_object_id
WHERE foreign_key_columns.parent_object_id = ?
ORDER BY foreign_key_columns.constraint_object_id, foreign_key_columns.constraint_column_id
)", id);

		while (sq.fetch_row()) {
			if (last_num != (int64_t)sq[0]) {
				foreign_keys.emplace_back((unsigned int)sq[5], (unsigned int)sq[6]);
				last_num = (int64_t)sq[0];
			}

			for (const auto& col : columns) {
				if (col.column_id == (unsigned int)sq[1]) {
					foreign_keys.back().cols.emplace_back(col, (string)sq[2], (string)sq[3], (string)sq[4]);
					break;
				}
			}
		}
	}

	escaped_name = ((!table.empty() && table.front() == '#') ? "" : (brackets_escape(schema) + ".")) + brackets_escape(table);

	if (type == "TT") {
		ddl = "DROP TYPE IF EXISTS " + escaped_name + ";\n\n";
		ddl += "CREATE TYPE " + escaped_name + " AS TABLE (\n";
	} else {
		ddl = "DROP TABLE IF EXISTS " + escaped_name + ";\n\n";
		ddl += "CREATE TABLE " + escaped_name + " (\n";
	}

	{
		bool first = true;

		for (const auto& col : columns) {
			if (!first)
				ddl += ",\n";

			first = false;

			ddl += "    " + brackets_escape(col.name);

			if (!col.is_computed) {
				ddl += " " + brackets_escape(col.type);

				if (col.type == "CHAR" || col.type == "VARCHAR" || col.type == "BINARY" || col.type == "VARBINARY")
					ddl += "(" + (col.max_length == -1 ? "MAX" : to_string(col.max_length)) + ")";
				else if (col.type == "NCHAR" || col.type == "NVARCHAR")
					ddl += "(" + (col.max_length == -1 ? "MAX" : to_string(col.max_length / 2)) + ")";
				else if (col.type == "DECIMAL" || col.type == "NUMERIC")
					ddl += "(" + to_string(col.precision) +"," + to_string(col.scale) + ")";
				else if ((col.type == "TIME" || col.type == "DATETIME2") && col.scale != 7)
					ddl += "(" + to_string(col.scale) + ")";
				else if (col.type == "DATETIMEOFFSET")
					ddl += "(" + to_string(col.scale) + ")"; // FIXME - what's the default for this?

				if (col.def.has_value())
					ddl += " DEFAULT" + col.def.value();

				if (col.is_identity) {
					ddl += " IDENTITY";

					if (seed_value != 1 || increment_value != 1)
						ddl += "(" + to_string(seed_value) + "," + to_string(increment_value) + ")";
				}

				if (col.collation.has_value())
					ddl += " COLLATE " + col.collation.value();

				ddl += " " + (col.nullable ? "NULL"s : "NOT NULL"s);

				{
					bool first_con = true;

					for (const auto& con : constraints) {
						if (con.column_id == col.column_id) {
							if (!first_con)
								ddl += ", ";

							first_con = false;

							ddl += " CHECK" + con.definition;
						}
					}

					for (const auto& fk : foreign_keys) {
						if (fk.cols.size() == 1 && &fk.cols.front().col == &col) {
							if (!first_con)
								ddl += ", ";

							ddl += " FOREIGN KEY REFERENCES " + brackets_escape(fk.cols.front().schema) + "." + brackets_escape(fk.cols.front().table) + "(" + brackets_escape(fk.cols.front().other_column) + ")";

							switch (fk.delete_referential_action) {
								case 1:
									ddl += " ON DELETE CASCADE";
								break;

								case 2:
									ddl += " ON DELETE SET NULL";
								break;

								case 3:
									ddl += " ON DELETE SET DEFAULT";
								break;
							}

							switch (fk.update_referential_action) {
								case 1:
									ddl += " ON UPDATE CASCADE";
								break;

								case 2:
									ddl += " ON UPDATE SET NULL";
								break;

								case 3:
									ddl += " ON UPDATE SET DEFAULT";
								break;
							}
						}
					}
				}

				for (const auto& ind : indices) {
					if (ind.is_primary_key) {
						if (ind.columns.size() == 1 && &ind.columns.front().col == &col) {
							ddl += " PRIMARY KEY";

							if (ind.type == 2)
								ddl += " NONCLUSTERED";
						}

						break;
					}
				}
			} else {
				ddl += " AS " + col.computed_definition;

				if (col.is_persisted)
					ddl += " PERSISTED";
			}
		}
	}

	for (const auto& ind : indices) {
		bool has_included_columns = false;

		if (!ind.is_primary_key) {
			for (const auto& col : ind.columns) {
				if (col.is_included) {
					has_included_columns = true;
					break;
				}
			}
		}

		if (!has_included_columns) {
			if (!ind.is_primary_key || ind.columns.size() > 1) {
				bool first = true;

				ddl += ",\n";

				if (ind.is_primary_key)
					ddl += "    PRIMARY KEY "s + (ind.type == 2 ? "NONCLUSTERED " : "") + "(";
				else
 					ddl += "    INDEX "s + brackets_escape(ind.name) + " " + (ind.is_unique ? "UNIQUE " : "") + (ind.type == 1 ? "CLUSTERED " : "") + "(";

				for (const auto& col : ind.columns) {
					if (!first)
						ddl += ", ";

					first = false;

					ddl += brackets_escape(col.col.name);

					if (col.is_desc)
						ddl += " DESC";
				}

				ddl += ")";
			}
		} else
			has_included_indices = true;
	}

	for (const auto& con : constraints) {
		if (con.column_id == 0)
			ddl += ",\n    CHECK" + con.definition;
	}

	for (const auto& fk : foreign_keys) {
		if (fk.cols.size() > 1) {
			bool first = true;

			ddl += ",\n    FOREIGN KEY (";

			for (const auto& c : fk.cols) {
				if (!first)
					ddl += ", ";

				first = false;

				ddl += brackets_escape(c.col.name);
			}

			ddl += ") REFERENCES " + brackets_escape(fk.cols.front().schema) + "." + brackets_escape(fk.cols.front().table) + "(";

			first = true;

			for (const auto& c : fk.cols) {
				if (!first)
					ddl += ", ";

				first = false;

				ddl += brackets_escape(c.other_column);
			}

			ddl += ")";

			switch (fk.delete_referential_action) {
				case 1:
					ddl += " ON DELETE CASCADE";
				break;

				case 2:
					ddl += " ON DELETE SET NULL";
				break;

				case 3:
					ddl += " ON DELETE SET DEFAULT";
				break;
			}

			switch (fk.update_referential_action) {
				case 1:
					ddl += " ON UPDATE CASCADE";
				break;

				case 2:
					ddl += " ON UPDATE SET NULL";
				break;

				case 3:
					ddl += " ON UPDATE SET DEFAULT";
				break;
			}
		}
	}

	ddl += "\n";

	ddl += ");\n";

	if (has_included_indices) {
		ddl += "\n";

		for (const auto& ind : indices) {
			if (!ind.is_primary_key) {
				bool has_included_columns = false;

				for (const auto& col : ind.columns) {
					if (col.is_included) {
						has_included_columns = true;
						break;
					}
				}

				if (has_included_columns) {
					bool first = true;

					ddl += "CREATE "s + (ind.is_unique ? "UNIQUE " : "") + (ind.type == 1 ? "CLUSTERED " : "") + "INDEX " + brackets_escape(ind.name) + " ON " + escaped_name + " (";

					for (const auto& col : ind.columns) {
						if (!col.is_included) {
							if (!first)
								ddl += ", ";

							first = false;

							ddl += brackets_escape(col.col.name);

							if (col.is_desc)
								ddl += " DESC";
						}
					}

					ddl += ") INCLUDE (";

					first = true;

					for (const auto& col : ind.columns) {
						if (col.is_included) {
							if (!first)
								ddl += ", ";

							first = false;

							ddl += brackets_escape(col.col.name);

							if (col.is_desc)
								ddl += " DESC";
						}
					}

					ddl += ");\n";
				}
			}
		}
	}

	{
		tds::query sq(tds, R"(
SELECT sql_modules.definition, triggers.is_disabled, triggers.name
FROM sys.triggers
JOIN sys.sql_modules ON sql_modules.object_id = triggers.object_id
WHERE triggers.parent_id = ?)", id);

		while (sq.fetch_row()) {
			auto trig = (string)sq[0];
			bool disabled = (unsigned int)sq[1] != 0;

			replace_all(trig, "\r\n", "\n");

			ddl += "\nGO\n" + trig;

			if (disabled)
				ddl += "\nDISABLE TRIGGER " + brackets_escape((string)sq[2]) + " ON " + escaped_name + ";\nGO";
		}
	}

	{
		tds::query sq(tds, R"(
SELECT * FROM sys.extended_properties
WHERE major_id = ? AND
	minor_id = 0 AND
	class = 1 AND
	name = 'fulldump'
)", id);

		if (sq.fetch_row())
			fulldump = true;
	}

	if (fulldump)
		ddl += "\n" + dump_table(tds, escaped_name);

	return ddl;
}
