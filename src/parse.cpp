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

static bool is_reserved(string_view sv) {
	string s;

	s.reserve(sv.size());

	for (auto c : sv) {
		if (c >= 'a' && c <= 'z')
			s += c - 'a' + 'A';
		else if ((c >= 'A' && c <= 'Z') || c == '_')
			s += c;
		else
			return false;
	}

	for (const auto& rw : reserved_words) {
		if (rw.s == s)
			return true;
	}

	return false;
}

static bool needs_escaping(string_view s) {
	if (s.empty())
		return true;

	for (auto c : s) {
		if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_' && c != '@' && c != '#' && c != '$')
			return true;
	}

	// can't have number or dollar sign at beginning

	if ((s.front() >= '0' && s.front() <= '9') || s.front() == '$')
		return true;

	// would change meaning if not escaped

	if (s.front() == '@')
		return true;

	return false;
}

static bool is_wordlike(enum lex l) {
	switch (l) {
		case lex::whitespace:
		case lex::single_comment:
		case lex::multi_comment:
		case lex::full_stop:
		case lex::slash:
		case lex::semicolon:
		case lex::variable:
		case lex::equals:
		case lex::open_bracket:
		case lex::close_bracket:
		case lex::number:
		case lex::minus:
		case lex::plus:
		case lex::asterisk:
		case lex::ampersand:
		case lex::percent:
		case lex::pipe:
		case lex::string_literal:
		case lex::binary_literal:
		case lex::comma:
		case lex::greater_than:
		case lex::less_than:
		case lex::colon:
		case lex::exclamation_point:
		case lex::open_brace:
		case lex::close_brace:
			return false;

		default:
			return true;
	}
}

string dequote(string_view sql) {
	auto words = parse(sql);
	string ret;

	ret.reserve(sql.size());

	for (auto it = words.begin(); it != words.end(); it++) {
		const auto& w = *it;

		if (w.type == lex::identifier && !w.val.empty() && w.val.front() == '[') {
			string s;
			string_view sv = w.val.substr(1, w.val.size() - 2);

			if (is_reserved(sv) || needs_escaping(sv))
				ret.append(w.val);
			else {
				if (it != words.begin()) {
					const auto& prev_w = *prev(it);

					if (is_wordlike(prev_w.type) && (prev_w.type != lex::identifier || prev_w.val.back() != ']'))
						ret.append(" ");
				}

				ret.append(sv);

				if (it != prev(words.end())) {
					const auto& next_w = *next(it);

					if (is_wordlike(next_w.type) && (next_w.type != lex::identifier || next_w.val.front() != '['))
						words.emplace(next(it), lex::whitespace, " ");
				}
			}
		} else
			ret.append(w.val);
	}

	return ret;
}
