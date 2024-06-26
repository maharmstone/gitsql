#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <span>
#include <format>
#include <tdscpp.h>
#include "lex.h"

static const std::string_view db_app = "GitSQL";

class formatted_error : public std::exception {
public:
	template<typename... Args>
	formatted_error(std::format_string<Args...> s, Args&&... args) : msg(std::format(s, std::forward<Args>(args)...)) {
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	std::string msg;
};

#ifdef _WIN32

class last_error : public std::exception {
public:
	last_error(std::string_view function, unsigned long le) {
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

#else

class errno_error : public std::exception {
public:
	errno_error(std::string_view function, int en);

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	std::string msg;
};

#endif

struct git_update;

// gitsql.cpp
std::string get_current_username();
void get_current_user_details(std::string& name, std::string& email);
void do_dump_sql(tds::tds& tds, git_update& gu);

// table.cpp
std::string table_ddl(tds::tds& tds, int64_t id, bool nolock);
std::string brackets_escape(std::string_view s);
std::u16string brackets_escape(std::u16string_view s);
std::string type_to_string(std::string_view name, int max_length, int precision, int scale);

// ldap.cpp
#ifdef _WIN32
void get_ldap_details_from_sid(PSID sid, std::string& name, std::string& email);
#else
void get_ldap_details_from_name(std::string_view username, std::string& name, std::string& email);
void get_ldap_details_from_full_name(std::string_view username, std::string& name, std::string& email);
#endif

// parse.cpp
std::string munge_definition(std::string_view sql, std::string_view schema, std::string_view name,
							 enum lex type);
std::string dequote(std::string_view sql);
std::string cleanup_sql(std::string_view sql);

// master.cpp
void dump_master(std::string_view db_server, unsigned int repo_num, std::span<const std::byte> smk);
