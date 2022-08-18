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
    size_t ends = 0, ends2 = s.size();

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

    if (ends2 == s.size())
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

static string_view next_char(string_view s) {
    auto v = (uint8_t)s.front();

    if (v < 0x80)
        return s.substr(0, 1);
    else if ((v & 0xe0) == 0xc0) {
        if (s.size() < 2)
            throw runtime_error("Malformed UTF-8.");

        return s.substr(0, 2);
    } else if ((v & 0xf0) == 0xe0) {
        if (s.size() < 3)
            throw runtime_error("Malformed UTF-8.");

        return s.substr(0, 3);
    } else if ((v & 0xf8) == 0xf0) {
        if (s.size() < 4)
            throw runtime_error("Malformed UTF-8.");

        return s.substr(0, 3);
    } else
        throw runtime_error("Malformed UTF-8.");
}

static char32_t char_val(string_view s) noexcept {
    char32_t v = (uint8_t)s.front();

    if (v < 0x80)
        return v;
    else if ((v & 0xe0) == 0xc0) {
        v &= 0x1f;
        v <<= 6;
        v |= (uint8_t)s[1] & 0x3f;
    } else if ((v & 0xf0) == 0xe0) {
        v &= 0xf;
        v <<= 12;
        v |= (char32_t)(((uint8_t)s[1] & 0x3f) << 6);
        v |= (uint8_t)s[2] & 0x3f;
    } else if ((v & 0xf8) == 0xf0) {
        v &= 0x7;
        v <<= 18;
        v |= (char32_t)(((uint8_t)s[1] & 0x3f) << 6);
        v |= (char32_t)(((uint8_t)s[2] & 0x3f) << 12);
        v |= (uint8_t)s[3] & 0x3f;
    }

    return v;
}

list<word> parse(string_view s) {
    list<word> words;
    string_view w;

    while (!s.empty()) {
        auto csv = next_char(s);
        auto c = char_val(csv);

        // FIXME - @ and $
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '#' || (!w.empty() && c >= '0' && c <= '9')) {
            if (w.empty() && c == 'N' && s.size() >= 2 && s[1] == '\'') { // Unicode string literal
                size_t sb = 2;

                do {
                    auto sb2 = s.find('\'', sb);

                    if (sb2 == string::npos)
                        throw runtime_error("Unterminated quotation marks.");

                    if (s.size() > sb2 + 1 && s[sb2 + 1] == '\'') { // handle ''
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
                w = csv;
            else
                w = string_view(w.data(), w.size() + csv.size());

            s = s.substr(csv.size());
            continue;
        }

        if (!w.empty()) {
            words.emplace_back(identify_word(w), w);
            w = string_view();
        }

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case 0xa0:
                if (words.empty() || words.back().type != lex::whitespace)
                    words.emplace_back(lex::whitespace, csv);
                else
                    words.back().val = string_view(words.back().val.data(), words.back().val.size() + csv.size());

                break;

            case ';':
                words.emplace_back(lex::semicolon, csv);
                break;

            case '=':
                words.emplace_back(lex::equals, csv);
                break;

            case '(':
                words.emplace_back(lex::open_bracket, csv);
                break;

            case ')':
                words.emplace_back(lex::close_bracket, csv);
                break;

            case ',':
                words.emplace_back(lex::comma, csv);
                break;

            case '>':
                words.emplace_back(lex::greater_than, csv);
                break;

            case '<':
                words.emplace_back(lex::less_than, csv);
                break;

            case ':':
                words.emplace_back(lex::colon, csv);
                break;

            case '!':
                words.emplace_back(lex::exclamation_point, csv);
                break;

            case '*':
                words.emplace_back(lex::asterisk, csv);
                break;

            case '&':
                words.emplace_back(lex::ampersand, csv);
                break;

            case '%':
                words.emplace_back(lex::percent, csv);
                break;

            case '|':
                words.emplace_back(lex::pipe, csv);
                break;

            case '{':
                words.emplace_back(lex::open_brace, csv);
                break;

            case '}':
                words.emplace_back(lex::close_brace, csv);
                break;

            case '/':
                if (s.size() >= 2 && s[1] == '*') { // multi-line comment
                    auto end = s.find("*/", 2);

                    // FIXME - nested comments

                    if (end == string::npos)
                        throw runtime_error("Unterminated block comment.");

                    words.emplace_back(lex::multi_comment, s.substr(0, end + 2));
                    s = s.substr(end + 2);

                    continue;
                } else
                    words.emplace_back(lex::slash, csv);

                break;

            case '[': {
                size_t sb = 1;

                do {
                    auto sb2 = s.find(']', sb);

                    if (sb2 == string::npos)
                        throw runtime_error("Unterminated square brackets.");

                    if (s.size() > sb2 + 1 && s[sb2 + 1] == ']') { // handle ]]
                        sb = sb2 + 2;
                        continue;
                    }

                    words.emplace_back(lex::identifier, s.substr(0, sb2 + 1));
                    s = s.substr(sb2 + 1);
                    break;
                } while (true);

                continue;
            }

            case '"': {
                size_t sb = 1;

                do {
                    auto sb2 = s.find('"', sb);

                    if (sb2 == string::npos)
                        throw runtime_error("Unterminated double quotation marks.");

                    if (s.size() > sb2 + 1 && s[sb2 + 1] == '"') { // handle ""
                        sb = sb2 + 2;
                        continue;
                    }

                    words.emplace_back(lex::identifier, s.substr(0, sb2 + 1));
                    s = s.substr(sb2 + 1);
                    break;
                } while (true);

                continue;
            }

            case '@': { // variable
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
            }

            case '-':
                if (s.size() > 1 && s[1] == '-') { // single-line comment
                    auto nl = s.find('\n'); // FIXME - should also be \r

                    if (nl == string::npos) {
                        words.emplace_back(lex::single_comment, s);
                        break;
                    }

                    words.emplace_back(lex::single_comment, s.substr(0, nl + 1));
                    s = s.substr(nl + 1);
                    continue;
                }
                [[fallthrough]];

            // number
            case '.':
            case '+':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
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
            }

            case '\'': { // string literal
                size_t sb = 1;

                do {
                    auto sb2 = s.find('\'', sb);

                    if (sb2 == string::npos)
                        throw runtime_error("Unterminated quotation marks.");

                    if (s.size() > sb2 + 1 && s[sb2 + 1] == '\'') { // handle ''
                        sb = sb2 + 2;
                        continue;
                    }

                    words.emplace_back(lex::string_literal, s.substr(0, sb2 + 1));
                    s = s.substr(sb2 + 1);
                    break;
                } while (true);

                continue;
            }

            // money literals
            case '$':
            case 0xa2:
            case 0xa3:
            case 0xa4:
            case 0xa5:
            case 0x9f2:
            case 0x9f3:
            case 0xe3f:
            case 0x17db:
            case 0x20a0:
            case 0x20a1:
            case 0x20a2:
            case 0x20a3:
            case 0x20a4:
            case 0x20a5:
            case 0x20a6:
            case 0x20a7:
            case 0x20a8:
            case 0x20a9:
            case 0x20aa:
            case 0x20ab:
            case 0x20ac:
            case 0x20ad:
            case 0x20ae:
            case 0x20af:
            case 0x20b0:
            case 0x20b1:
            case 0xfdfc:
            case 0xfe69:
            case 0xff04:
            case 0xffe0:
            case 0xffe1:
            case 0xffe5:
            case 0xffe6: {
                if (s.size() <= csv.size())
                    throw runtime_error("Currency symbol not followed by number.");

                auto ends = parse_number(s.substr(csv.size()));

                if (ends == 0)
                    throw runtime_error("Unable to parse money literal.");

                ends += csv.size();

                words.emplace_back(lex::money_literal, s.substr(0, ends));

                s = s.substr(ends);
                continue;
            }

            default:
                throw runtime_error("Unhandled character '" + string(csv) + "'.");
        }

        s = s.substr(csv.size());
    }

    if (!w.empty()) {
        words.emplace_back(identify_word(w), w);
        w = string_view();
    }

    return words;
}
