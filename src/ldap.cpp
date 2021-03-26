#ifdef _WIN32
#include <windows.h>
#include <winldap.h>
#include <winber.h>
#include <sddl.h>
#else
// FIXME
#endif

#include <iostream>
#include <vector>
#include <map>
#include <span>

using namespace std;

static __inline u16string utf8_to_utf16(const string_view& s) {
	u16string ret;

	if (s.empty())
		return u"";

	auto len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.length(), nullptr, 0);

	if (len == 0)
		throw runtime_error("MultiByteToWideChar 1 failed.");

	ret.resize(len);

	len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.length(), (wchar_t*)ret.data(), len);

	if (len == 0)
		throw runtime_error("MultiByteToWideChar 2 failed.");

	return ret;
}

static __inline string utf16_to_utf8(const u16string_view& s) {
	string ret;

	if (s.empty())
		return "";

	auto len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)s.data(), (int)s.length(), nullptr, 0,
								   nullptr, nullptr);

	if (len == 0)
		throw runtime_error("WideCharToMultiByte 1 failed.");

	ret.resize(len);

	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)s.data(), (int)s.length(), ret.data(), len,
							  nullptr, nullptr);

	if (len == 0)
		throw runtime_error("WideCharToMultiByte 2 failed.");

	return ret;
}

class last_error : public exception {
public:
	last_error(const string_view& function, int le) {
		string nice_msg;

		{
			char16_t* fm;

			if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
				le, 0, reinterpret_cast<LPWSTR>(&fm), 0, nullptr)) {
				try {
					u16string_view s = fm;

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

		msg = string(function) + " failed (error " + to_string(le) + (!nice_msg.empty() ? (", " + nice_msg) : "") + ").";
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	string msg;
};

class ldap_error : public exception {
public:
	ldap_error(const string& func, ULONG err) {
		auto errmsg = ldap_err2string(err);

		if (errmsg)
			msg = func + " failed: " + errmsg;
		else
			msg = func + " failed: error " + to_string(err);
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	string msg;
};

class ldapobj {
public:
	ldapobj() {
		LDAPMessage* res = nullptr;
		BerElement* ber = nullptr;

		ld = ldap_init(nullptr, LDAP_PORT);
		if (!ld)
			throw ldap_error("ldap_init", LdapGetLastError());

		auto err = ldap_connect(ld, nullptr);

		if (err != LDAP_SUCCESS) {
			auto exc = ldap_error("ldap_connect", err);
			ldap_unbind(ld);
			throw exc;
		}

		err = ldap_bind_s(ld, nullptr, nullptr, LDAP_AUTH_NEGOTIATE);

		if (err != LDAP_SUCCESS) {
			auto exc = ldap_error("ldap_bind_s", err);
			ldap_unbind(ld);
			throw exc;
		}

		const char* atts[] = { "defaultNamingContext", nullptr };

		err = ldap_search_s(ld, nullptr, LDAP_SCOPE_BASE, nullptr,
							(char**)atts, false, &res);
		if (err != LDAP_SUCCESS) {
			auto exc = ldap_error("ldap_search_s", err);
			ldap_unbind(ld);
			throw exc;
		}

		auto att = ldap_first_attribute(ld, res, &ber);

		while (att) {
			auto val = ldap_get_values(ld, res, att);

			if (val && val[0])
				naming_context = val[0];

			if (val)
				ldap_value_free(val);

			att = ldap_next_attribute(ld, res, ber);
		}

		if (ber)
			ber_free(ber, 0);

		if (res)
			ldap_msgfree(res);

		if (naming_context.empty()) {
			ldap_unbind(ld);
			throw runtime_error("Could not get LDAP naming context.");
		}
	}

	~ldapobj() {
		ldap_unbind(ld);
	}

	map<string, string> search(const string& filter, const vector<string>& atts) {
		char* att;
		LDAPMessage* res = nullptr;
		BerElement* ber = nullptr;
		vector<char*> attlist;
		map<string, string> values;

		if (!atts.empty()) {
			for (const auto& att : atts) {
				attlist.push_back((char*)att.c_str());
			}

			attlist.push_back(nullptr);
		}

		auto ret = ldap_search_s(ld, (char*)naming_context.c_str(), LDAP_SCOPE_SUBTREE, (char*)filter.c_str(),
								 atts.empty() ? nullptr : &attlist[0], false, &res);

		if (ret != LDAP_SUCCESS) {
			if (res)
				ldap_msgfree(res);

			throw ldap_error("ldap_search_s", ret);
		}

		att = ldap_first_attribute(ld, res, &ber);

		while (att) {
			auto val = ldap_get_values_len(ld, res, att);

			if (val && val[0])
				values[att] = string(val[0]->bv_val, val[0]->bv_len);
			else
				values[att] = "";

			if (val)
				ldap_value_free_len(val);

			att = ldap_next_attribute(ld, res, ber);
		}

 		if (ber)
 			ber_free(ber, 0);

		if (res)
			ldap_msgfree(res);

		return values;
	}

	vector<string> search_multiple(const string& filter, const string& att) {
		vector<string> values;
		char* s;
		LDAPMessage* res = nullptr;
		BerElement* ber = nullptr;
		array<char*, 2> attlist;

		attlist[0] = (char*)att.c_str();
		attlist[1] = nullptr;

		auto ret = ldap_search_s(ld, (char*)naming_context.c_str(), LDAP_SCOPE_SUBTREE, (char*)filter.c_str(),
								 &attlist[0], false, &res);

		if (ret != LDAP_SUCCESS) {
			if (res)
				ldap_msgfree(res);

			throw ldap_error("ldap_search_s", ret);
		}

		s = ldap_first_attribute(ld, res, &ber);

		while (s) {
			auto val = ldap_get_values_len(ld, res, s);

			if (val) {
				unsigned int i = 0;

				while (val[i]) {
					values.emplace_back(string_view(val[i]->bv_val, val[i]->bv_len));
					i++;
				}

				ldap_value_free_len(val);
			}

			s = ldap_next_attribute(ld, res, ber);
		}

		if (ber)
			ber_free(ber, 0);

		if (res)
			ldap_msgfree(res);

		return values;
	}

private:
	LDAP* ld;
	string naming_context;
};

static string hex_byte(uint8_t v) {
	char s[3];

	if ((v & 0xf0) >= 0xa0)
		s[0] = (char)((v >> 4) - 0xa + 'A');
	else
		s[0] = (char)((v >> 4) + '0');

	if ((v & 0xf) >= 0xa)
		s[1] = (v & 0xf) - 0xa + 'A';
	else
		s[1] = (v & 0xf) + '0';

	s[2] = 0;

	return s;
}

void get_ldap_details_from_sid(const span<std::byte>& sid, string& name, string& email) {
	ldapobj l;
	string binsid;
	DWORD sidlen;

	sidlen = GetLengthSid(sid.data());
	binsid.reserve(3 * sidlen);
	for (unsigned int i = 0; i < sidlen; i++) {
		binsid += "\\";
		binsid += hex_byte(((uint8_t*)sid.data())[i]);
	}

	auto ret = l.search("(objectSid=" + binsid + ")", { "givenName", "sn", "name", "mail" });

	if (ret.count("givenName") != 0 && ret.count("sn") != 0)
		name = ret.at("givenName") + " " + ret.at("sn");
	else if (ret.count("name") != 0)
		name = ret.at("name");
	else
		name = "";

	if (ret.count("mail") != 0)
		email = ret.at("mail");
	else
		email = "";
}
