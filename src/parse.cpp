#include "gitsql.h"

using namespace std;

string munge_definition(string_view sql, string_view schema, string_view name,
						enum lex type) {
	auto words = parse(sql);

	words.remove_if([](const word& w) {
        return w.type == lex::whitespace ||
               w.type == lex::single_comment ||
               w.type == lex::multi_comment;
    });

	if (words.empty())
		return string{sql};

	if (words.front().type != lex::CREATE)
		return string{sql};

	auto create = words.front();

	words.pop_front();

	if (words.empty())
		return string{sql};

	if (words.front().type != type)
		return string{sql};

	words.pop_front();

	if (words.empty())
		return string{sql};

	if (words.front().type != lex::identifier)
		return string{sql};

	auto sv = sql.substr((words.front().val.data() + words.front().val.size()) - sql.data());

	words.pop_front();

	if (words.empty())
		return string{sql};

	if (words.front().type == lex::full_stop) {
		words.pop_front();

		if (words.empty())
			return string{sql};

		if (words.front().type != lex::identifier)
			return string{sql};

		sv = sql.substr((words.front().val.data() + words.front().val.size()) - sql.data());

		words.pop_front();

		if (words.empty())
			return string{sql};
	}

	string sql2{string_view(sql).substr(0, create.val.data() - sql.data())};

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
