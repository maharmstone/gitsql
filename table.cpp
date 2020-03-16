#include <string>
#include <optional>
#include <vector>
#include <iostream>
#include <tdscpp.h>

using namespace std;

static bool need_escaping(const string_view& s) {
	// see https://docs.microsoft.com/en-us/previous-versions/sql/sql-server-2008-r2/ms175874(v=sql.105)

	if (s.empty())
		return true;

	for (const auto& c : s) {
		if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_' && c != '@' && c != '#' && c != '$')
			return true;
	}

	// can't have number or dollar sign at beginning
	
	if ((s.front() >= '0' && s.front() <= '9') || s.front() == '$')
		return true;

	// FIXME - check not reserved word

	return false;
}

static string brackets_escape(const string_view& s) {
	string ret;

	if (!need_escaping(s))
		return string(s);

	ret = "[";

	for (const auto& c : s) {
		if (c == ']')
			ret += "]]";
		else
			ret += c;
	}

	ret += "]";

	return ret;
}

struct column {
	column(const string& name, const string& type, int max_length, bool nullable, int precision, int scale, const optional<string>& default, int column_id, bool is_identity, bool is_computed, bool is_persisted, const string& computed_definition) : name(name), type(type), max_length(max_length), nullable(nullable), precision(precision), scale(scale), default(default), column_id(column_id), is_identity(is_identity), is_computed(is_computed), is_persisted(is_persisted), computed_definition(computed_definition) { }

	string name, type, computed_definition;
	int max_length, precision, scale;
	unsigned int column_id;
	bool nullable, is_identity, is_computed, is_persisted;
	optional<string> default;
};

struct index_column {
	index_column(const column& col, bool is_desc, bool is_included) : col(col), is_desc(is_desc), is_included(is_included) { } 

	const column& col;
	bool is_desc;
	bool is_included;
};

struct index {
	index(const string& name, unsigned int type, bool is_unique, bool is_primary_key) : name(name), type(type), is_unique(is_unique), is_primary_key(is_primary_key) { }

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

void table_test(tds::Conn& tds, const string_view& schema, const string_view& table) {
	int64_t id, seed_value, increment_value;
	vector<column> columns;
	vector<index> indices;
	vector<constraint> constraints;
	string escaped_name, ddl;
	bool has_included_indices = false;

	{
		tds::Query sq(tds, R"(
SELECT objects.object_id,
	identity_columns.seed_value,
	identity_columns.increment_value
FROM sys.objects
JOIN sys.schemas ON schemas.schema_id = objects.schema_id
LEFT JOIN sys.identity_columns ON identity_columns.object_id = objects.object_id
WHERE objects.name = ? AND schemas.name = ?
)", table, schema);

		if (!sq.fetch_row())
			throw runtime_error("Cannot find object ID for "s + string(schema) + "."s + string(table) + "."s);

		id = (int64_t)sq[0];
		seed_value = (int64_t)sq[1];
		increment_value = (int64_t)sq[2];
	}

	{
		tds::Query sq (tds, R"(
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
	computed_columns.definition
FROM sys.columns
JOIN sys.types ON types.user_type_id = columns.user_type_id
LEFT JOIN sys.default_constraints ON default_constraints.parent_object_id = columns.object_id AND default_constraints.parent_column_id  = columns.column_id
LEFT JOIN sys.computed_columns ON computed_columns.object_id = columns.object_id AND computed_columns.column_id = columns.column_id
WHERE columns.object_id = ?
ORDER BY columns.column_id
)", id);

		while (sq.fetch_row()) {
			columns.emplace_back(sq[0], sq[1], (int)sq[2], (int)sq[3] != 0, (int)sq[4], (int)sq[5], sq[6], (unsigned int)sq[7], (unsigned int)sq[8] != 0, (unsigned int)sq[9] != 0, (unsigned int)sq[10] != 0, sq[11]);
		}
	}

	{
		string last_name;

		tds::Query sq(tds, "SELECT indexes.name, indexes.type, indexes.is_unique, indexes.is_primary_key, index_columns.column_id, index_columns.is_descending_key, index_columns.is_included_column FROM sys.indexes JOIN sys.index_columns ON index_columns.object_id = indexes.object_id AND index_columns.index_id = indexes.index_id WHERE indexes.object_id = ? ORDER BY indexes.index_id, index_columns.index_column_id", id);

		while (sq.fetch_row()) {
			bool found = false;

			if ((string)sq[0] != last_name) {
				last_name = sq[0];
				indices.emplace_back(last_name, (unsigned int)sq[1], (unsigned int)sq[2] != 0, (unsigned int)sq[3] != 0);
			}

			for (const auto& col : columns) {
				if (col.column_id == (unsigned int)sq[4]) {
					indices.back().columns.emplace_back(col, (unsigned int)sq[5] != 0, (unsigned int)sq[6] != 0);
					found = true;
					break;
				}
			}

			if (!found)
				throw runtime_error("Could not find column no. " + (string)sq[4] + ".");
		}
	}

	{
		tds::Query sq(tds, "SELECT definition, parent_column_id FROM sys.check_constraints WHERE parent_object_id = ? ORDER BY name", id);

		while (sq.fetch_row()) {
			constraints.emplace_back(sq[0], (unsigned int)sq[1]);
		}
	}

	escaped_name = brackets_escape(schema) + "." + brackets_escape(table);

	ddl = "DROP TABLE IF EXISTS " + escaped_name + ";\n\n";
	ddl += "CREATE TABLE " + escaped_name + " (\n";

	{
		bool first = true;

		for (const auto& col : columns) {
			if (!first)
				ddl += ",\n";
		
			first = false;

			ddl += "    " + brackets_escape(col.name);
			
			if (!col.is_computed) {
				ddl += " " + brackets_escape(col.type);

				if (col.type == "CHAR" || col.type == "VARCHAR" || col.type == "BINARY" || col.type == "VARBINARY" || col.type == "NCHAR" || col.type == "NVARCHAR")
					ddl += "(" + (col.max_length == -1 ? "MAX" : to_string(col.max_length)) + ")";
				else if (col.type == "DECIMAL" || col.type == "NUMERIC")
					ddl += "(" + to_string(col.precision) +"," + to_string(col.scale) + ")";
				else if ((col.type == "TIME" || col.type == "DATETIME2") && col.scale != 7)
					ddl += "(" + to_string(col.scale) + ")";
				else if (col.type == "DATETIMEOFFSET")
					ddl += "(" + to_string(col.scale) + ")"; // FIXME - what's the default for this?

				// FIXME - collation?

				if (col.default.has_value())
					ddl += " DEFAULT" + col.default.value();

				if (col.is_identity) {
					ddl += " IDENTITY";

					if (seed_value != 1 || increment_value != 1)
						ddl += "(" + to_string(seed_value) + "," + to_string(increment_value) + ")";
				}

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

	// FIXME - foreign keys

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

		if (!has_included_columns && (!ind.is_primary_key || ind.columns.size() > 1)) {
			bool first = true;

			ddl += ",\n";

			if (ind.is_primary_key)
				ddl += "    PRIMARY KEY "s + (ind.type == 2 ? "NONCLUSTERED " : "") + "(";
			else
 				ddl += "    INDEX "s + (ind.is_unique ? "UNIQUE " : "") + (ind.type == 1 ? "CLUSTERED " : "") + brackets_escape(ind.name) + " (";

			for (const auto& col : ind.columns) {
				if (!first)
					ddl += ", ";

				first = false;

				ddl += brackets_escape(col.col.name);

				if (col.is_desc)
					ddl += " DESC";
			}

			ddl += ")";
		} else
			has_included_indices = true;
	}

	for (const auto& con : constraints) {
		if (con.column_id == 0)
			ddl += ",\n    CHECK" + con.definition;
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

					ddl += ");";
				}
			}
		}
	}

	cout << ddl << endl;
}
