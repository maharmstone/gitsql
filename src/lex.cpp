#include "lex.h"

using namespace std;

static enum lex identify_word(string_view s) {
    if (s[0] == '[')
        return lex::identifier;

    string t;

    t.reserve(s.size());

    for (auto c : s) {
        if (c >= 'a' && c <= 'z')
            t += c - 'a' + 'A';
        else if ((c >= 'A' && c <= 'Z') || c == '_')
            t += c;
        else
            return lex::identifier;
    }

    for (const auto& rw : reserved_words) {
        if (rw.s == t)
            return rw.type;
    }

    return lex::identifier;
}

// FIXME - make this constexpr and add static_assert tests
static size_t parse_number(string_view s) {
    size_t ends = 0, ends2;

    if (s[0] == '.' || s[0] == '-' || s[0] == '+') {
        // FIXME - +.e0 should be valid number
        if (s.size() == 1 || s[1] < '0' || s[1] > '9')
            return 0;

        ends = 1;
    }

    for (size_t i = ends; i < s.size(); i++) {
        if (s[i] < '0' || s[i] > '9') {
            ends2 = i;
            break;
        }
    }

    if (ends2 == 0)
        return s.size();

    ends = ends2;

    if (s[0] != '.' && s.size() >= ends && s[ends] == '.') {
        ends2 = 0;

        ends++;

        for (size_t i = ends; i < s.size(); i++) {
            if (s[i] < '0' || s[i] > '9') {
                ends2 = i;
                break;
            }
        }

        if (ends2 == 0)
            return s.size();

        ends = ends2;
    }

    if (s.size() >= ends && (s[ends] == 'e' || s[ends] == 'E')) {
        ends2 = 0;

        ends++;

        for (size_t i = ends; i < s.size(); i++) {
            if (s[i] < '0' || s[i] > '9') {
                ends2 = i;
                break;
            }
        }

        if (ends2 == 0)
            return s.size();

        ends = ends2;
    }

    return ends;
}

list<word> parse(string_view s) {
    list<word> words;
    string_view w;

    while (!s.empty()) {
        auto c = s.front();

        // FIXME - @ and $
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '#' || (!w.empty() && c >= '0' && c <= '9')) {
            if (w.empty() && c == 'N' && s.size() >= 2 && s[1] == '\'') { // Unicode string literal
                size_t sb = 2;

                do {
                    auto sb2 = s.find('\'', sb);

                    if (sb2 == string::npos)
                        throw runtime_error("Unterminated quotation marks.");

                    if (s.size() >= sb2 + 1 && s[sb2 + 1] == '\'') { // handle ''
                        sb = sb2 + 2;
                        continue;
                    }

                    words.emplace_back(lex::string_literal, s.substr(0, sb2 + 1));
                    s = s.substr(sb2 + 1);
                    break;
                } while (true);

                continue;
            }

            if (w.empty())
                w = s.substr(0, 1);
            else
                w = string_view(w.data(), w.size() + 1);

            s = s.substr(1);
            continue;
        }

        if (!w.empty()) {
            words.emplace_back(identify_word(w), w);
            w = string_view();
        }

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (words.empty() || words.back().type != lex::whitespace)
                words.emplace_back(lex::whitespace, string_view(s.data(), 1));
            else
                words.back().val = string_view(words.back().val.data(), words.back().val.size() + 1);
        } else if (c == ';')
            words.emplace_back(lex::semicolon, string_view(s.data(), 1));
        else if (c == '=')
            words.emplace_back(lex::equals, string_view(s.data(), 1));
        else if (c == '(')
            words.emplace_back(lex::open_bracket, string_view(s.data(), 1));
        else if (c == ')')
            words.emplace_back(lex::close_bracket, string_view(s.data(), 1));
        else if (c == ',')
            words.emplace_back(lex::comma, string_view(s.data(), 1));
        else if (c == '>')
            words.emplace_back(lex::greater_than, string_view(s.data(), 1));
        else if (c == '<')
            words.emplace_back(lex::less_than, string_view(s.data(), 1));
        else if (c == ':')
            words.emplace_back(lex::colon, string_view(s.data(), 1));
        else if (c == '!')
            words.emplace_back(lex::exclamation_point, string_view(s.data(), 1));
        else if (c == '*')
            words.emplace_back(lex::asterisk, string_view(s.data(), 1));
        else if (c == '&')
            words.emplace_back(lex::ampersand, string_view(s.data(), 1));
        else if (c == '%')
            words.emplace_back(lex::percent, string_view(s.data(), 1));
        else if (c == '|')
            words.emplace_back(lex::pipe, string_view(s.data(), 1));
        else if (c == '{')
            words.emplace_back(lex::open_brace, string_view(s.data(), 1));
        else if (c == '}')
            words.emplace_back(lex::close_brace, string_view(s.data(), 1));
        else if (c == '/') {
            if (s.size() >= 2 && s[1] == '*') { // multi-line comment
                auto end = s.find("*/", 2);

                // FIXME - nested comments

                if (end == string::npos)
                    throw runtime_error("Unterminated block comment.");

                words.emplace_back(lex::multi_comment, s.substr(0, end + 2));
                s = s.substr(end + 2);

                continue;
            } else
                words.emplace_back(lex::slash, string_view(s.data(), 1));
        } else if (c == '-' && s.size() > 1 && s[1] == '-') { // single-line comment
            auto nl = s.find('\n'); // FIXME - should also be \r

            if (nl == string::npos) {
                words.emplace_back(lex::single_comment, s);
                break;
            }

            words.emplace_back(lex::single_comment, s.substr(0, nl + 1));
            s = s.substr(nl + 1);
            continue;
        } else if (c == '[') {
            size_t sb = 0;

            do {
                auto sb2 = s.find(']', sb);

                if (sb2 == string::npos)
                    throw runtime_error("Unterminated square brackets.");

                if (s.size() >= sb2 + 1 && s[sb2 + 1] == ']') { // handle ]]
                    sb = sb2 + 2;
                    continue;
                }

                words.emplace_back(lex::identifier, s.substr(0, sb2 + 1));
                s = s.substr(sb2 + 1);
                break;
            } while (true);

            continue;
        } else if (c == '"') {
            size_t sb = 1;

            do {
                auto sb2 = s.find('"', sb);

                if (sb2 == string::npos)
                    throw runtime_error("Unterminated double quotation marks.");

                if (s.size() >= sb2 + 1 && s[sb2 + 1] == '"') { // handle ""
                    sb = sb2 + 2;
                    continue;
                }

                words.emplace_back(lex::identifier, s.substr(0, sb2 + 1));
                s = s.substr(sb2 + 1);
                break;
            } while (true);

            continue;
        } else if (c == '@') { // variable
            size_t ends = 0;
            for (size_t i = 1; i < s.size(); i++) {
                if ((s[i] < 'A' || s[i] > 'Z') && (s[i] < 'a' || s[i] > 'z') && s[i] != '_' && s[i] != '#' && s[i] != '@' && s[i] != '$') {
                    ends = i;
                    break;
                }
            }

            if (ends == 0) {
                words.emplace_back(lex::variable, s);
                break;
            }

            words.emplace_back(lex::variable, s.substr(0, ends));
            s = s.substr(ends);
            continue;
        } else if (c == '.' || c == '-' || c == '+' || (c >= '0' && c <= '9')) { // number
            if (c == '0' && s.size() >= 2 && s[1] == 'x') { // binary literal
                size_t ends = 0;
                for (size_t i = 2; i < s.size(); i++) {
                    if ((s[i] < 'A' || s[i] > 'F') && (s[i] < 'a' || s[i] > 'f') && (s[i] < '0' || s[i] > '9')) {
                        ends = i;
                        break;
                    }
                }

                if (ends == 0) {
                    words.emplace_back(lex::binary_literal, s);
                    break;
                }

                words.emplace_back(lex::binary_literal, s.substr(0, ends));
                s = s.substr(ends);
                continue;
            }

            auto ends = parse_number(s);

            if (ends == 0) {
                switch (c) {
                    case '.':
                        words.emplace_back(lex::full_stop, string_view(s.data(), 1));
                        break;

                    case '-':
                        words.emplace_back(lex::minus, string_view(s.data(), 1));
                        break;

                    case '+':
                        words.emplace_back(lex::plus, string_view(s.data(), 1));
                        break;

                    default:
                        throw runtime_error("Unexpected error when trying to parse number.");

                }

                s = s.substr(1);
                continue;
            }

            words.emplace_back(lex::number, s.substr(0, ends));

            s = s.substr(ends);
            continue;
        } else if (c == '\'') { // string literal
            size_t sb = 1;

            do {
                auto sb2 = s.find('\'', sb);

                if (sb2 == string::npos)
                    throw runtime_error("Unterminated quotation marks.");

                if (s.size() >= sb2 + 1 && s[sb2 + 1] == '\'') { // handle ''
                    sb = sb2 + 2;
                    continue;
                }

                words.emplace_back(lex::string_literal, s.substr(0, sb2 + 1));
                s = s.substr(sb2 + 1);
                break;
            } while (true);

            continue;
        } else
            throw runtime_error("Unhandled character '" + string(1, c) + "'.");

        s = s.substr(1);
    }

    if (!w.empty()) {
        words.emplace_back(identify_word(w), w);
        w = string_view();
    }

    return words;
}
