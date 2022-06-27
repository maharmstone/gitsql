#pragma once

#include <windows.h>
#include <string>
#include <span>
#include <tdscpp.h>
#include <fmt/format.h>
#include <fmt/compile.h>
#include "lex.h"

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
	last_error(std::string_view function, int le) {
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

// gitsql.cpp
std::string object_perms(tds::tds& tds, int64_t id, const std::string& dbs, const std::string& name);

// table.cpp
std::string table_ddl(tds::tds& tds, int64_t id);
std::string brackets_escape(std::string_view s);
std::u16string brackets_escape(const std::u16string_view& s);

// ldap.cpp
void get_ldap_details_from_sid(PSID sid, std::string& name, std::string& email);

// parse.cpp
std::string munge_definition(std::string_view sql, std::string_view schema, std::string_view name,
							 enum lex type);
