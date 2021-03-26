#pragma once

#include <windows.h>
#include <string>
#include <span>
#include <tdscpp.h>

static __inline std::u16string utf8_to_utf16(const std::string_view& s) {
	std::u16string ret;

	if (s.empty())
		return u"";

	auto len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.length(), nullptr, 0);

	if (len == 0)
		throw std::runtime_error("MultiByteToWideChar 1 failed.");

	ret.resize(len);

	len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.length(), (wchar_t*)ret.data(), len);

	if (len == 0)
		throw std::runtime_error("MultiByteToWideChar 2 failed.");

	return ret;
}

static __inline std::string utf16_to_utf8(const std::u16string_view& s) {
#ifdef _WIN32
	std::string ret;

	if (s.empty())
		return "";

	auto len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)s.data(), (int)s.length(), nullptr, 0,
								   nullptr, nullptr);

	if (len == 0)
		throw std::runtime_error("WideCharToMultiByte 1 failed.");

	ret.resize(len);

	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)s.data(), (int)s.length(), ret.data(), len,
							  nullptr, nullptr);

	if (len == 0)
		throw std::runtime_error("WideCharToMultiByte 2 failed.");

	return ret;
#else
	wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> convert;

	return convert.to_bytes(sv.data(), sv.data() + sv.length());
#endif
}

class last_error : public std::exception {
public:
	last_error(const std::string_view& function, int le) {
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

					nice_msg = utf16_to_utf8(s);
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


class handle_closer {
public:
	typedef HANDLE pointer;

	void operator()(HANDLE h) {
		if (h == INVALID_HANDLE_VALUE)
			return;

		CloseHandle(h);
	}
};

typedef std::unique_ptr<HANDLE, handle_closer> unique_handle;

// table.cpp
std::string table_ddl(tds::tds& tds, int64_t id);
std::string brackets_escape(const std::string_view& s);

// ldap.cpp
void get_ldap_details_from_sid(PSID sid, std::string& name, std::string& email);
