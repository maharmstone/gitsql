#ifdef _WIN32
#include <windows.h>
#include <winldap.h>
#include <winber.h>
#include <sddl.h>
#else
#include <ldap.h>
#include <sasl/sasl.h>
#endif

#include <iostream>
#include <vector>
#include <map>
#include <span>
#include <array>
#include <string>
#include <memory>
#include <unordered_map>

using namespace std;

struct string_hash {
	using hash_type = hash<string_view>;
	using is_transparent = void;

	size_t operator()(const char* str) const {
		return hash_type{}(str);
	}

	size_t operator()(string_view str) const   {
		return hash_type{}(str);
	}

	size_t operator()(const string& str) const {
		return hash_type{}(str);
	}

};

static unordered_map<string, pair<string, string>, string_hash, equal_to<>> ldap_cache;
static unordered_map<string, string, string_hash, equal_to<>> ldap_domain;

class ldap_error : public exception {
public:
#ifdef _WIN32
	using ldap_err_t = unsigned long;
#else
	using ldap_err_t = int;
#endif

	ldap_error(const string& func, ldap_err_t err) {
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

class ldap_closer {
public:
	typedef LDAP* pointer;

	void operator()(LDAP* ldap) {
		if (!ldap)
			return;

#ifdef _WIN32
		ldap_unbind(ldap);
#else
		ldap_unbind_ext_s(ldap, nullptr, nullptr);
#endif
	}
};

using ldap_t = unique_ptr<LDAP*, ldap_closer>;

class ldapobj {
public:
	ldapobj();
	map<string, string> search(const string& filter, const vector<string>& atts);
	map<string, string> search_context(const string& filter, const vector<string>& atts,
									   const string& context);

	string naming_context;

private:
	void find_naming_context();

	ldap_t ld;
};

class ldap_message_closer {
public:
	typedef LDAPMessage* pointer;

	void operator()(LDAPMessage* msg) {
		if (msg)
			ldap_msgfree(msg);
	}
};

using ldap_message = unique_ptr<LDAPMessage*, ldap_message_closer>;

class bervals_closer {
public:
	typedef berval** pointer;

	void operator()(berval** bv) {
		if (bv)
			ldap_value_free_len(bv);
	}
};

using bervals_ptr = unique_ptr<berval**, bervals_closer>;

#ifndef _WIN32
static int lutil_sasl_interact(LDAP*, unsigned int, void*, void* in) {
	auto v = (sasl_interact_t*)in;

	while (v->id != SASL_CB_LIST_END) {
		v++;
	}

	return LDAP_SUCCESS;
}
#endif

ldapobj::ldapobj() {
	ldap_error::ldap_err_t err;

#ifdef _WIN32
	ld.reset(ldap_init(nullptr, LDAP_PORT));
	if (!ld)
		throw ldap_error("ldap_init", LdapGetLastError());
#else
	err = ldap_initialize(out_ptr(ld), nullptr);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_initialize", err);
#endif

#ifdef _WIN32
	err = ldap_connect(ld.get(), nullptr);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_connect", err);
#endif

	int version = LDAP_VERSION3;
	err = ldap_set_option(ld.get(), LDAP_OPT_PROTOCOL_VERSION, &version);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_set_option", err);

	err = ldap_set_option(ld.get(), LDAP_OPT_REFERRALS, LDAP_OPT_OFF);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_set_option", err);

#ifdef _WIN32
	err = ldap_bind_s(ld.get(), nullptr, nullptr, LDAP_AUTH_NEGOTIATE);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_bind_s", err);
#else
	err = ldap_sasl_interactive_bind_s(ld.get(), nullptr, nullptr, nullptr, nullptr,
									   LDAP_SASL_QUIET, lutil_sasl_interact, nullptr);

	if (err != LDAP_SUCCESS)
		throw ldap_error("ldap_sasl_interactive_bind_s", err);
#endif

	find_naming_context();
}

void ldapobj::find_naming_context() {
	ldap_message res;
	BerElement* ber = nullptr;

	const char* atts[] = { "defaultNamingContext", nullptr };

	{
		LDAPMessage* tmp = nullptr;

		auto err = ldap_search_ext_s(ld.get(), nullptr, LDAP_SCOPE_BASE, nullptr,
									(char**)atts, false, nullptr, nullptr, nullptr,
									0, &tmp);

		res.reset(tmp);

		if (err != LDAP_SUCCESS)
			throw ldap_error("ldap_search_ext_s", err);
	}

	auto att = ldap_first_attribute(ld.get(), res.get(), &ber);

	while (att) {
		bervals_ptr bv{ldap_get_values_len(ld.get(), res.get(), att)};

		if (bv && ldap_count_values_len(bv.get()) > 0)
			naming_context = string_view(bv.get()[0]->bv_val, bv.get()[0]->bv_len);

		att = ldap_next_attribute(ld.get(), res.get(), ber);
	}

	if (ber)
		ber_free(ber, 0);

	if (naming_context.empty())
		throw runtime_error("Could not get LDAP naming context.");
}

map<string, string> ldapobj::search_context(const string& filter, const vector<string>& atts,
											const string& context) {
	char* att;
	ldap_message res;
	BerElement* ber = nullptr;
	vector<char*> attlist;
	map<string, string> values;

	if (!atts.empty()) {
		for (const auto& att : atts) {
			attlist.push_back((char*)att.c_str());
		}

		attlist.push_back(nullptr);
	}

	{
		LDAPMessage* tmp = nullptr;

		auto ret = ldap_search_ext_s(ld.get(), (char*)context.c_str(), LDAP_SCOPE_SUBTREE, (char*)filter.c_str(),
									atts.empty() ? nullptr : &attlist[0], false, nullptr, nullptr, nullptr,
									0, &tmp);

		res.reset(tmp);

		if (ret != LDAP_SUCCESS)
			throw ldap_error("ldap_search_ext_s", ret);
	}

	att = ldap_first_attribute(ld.get(), res.get(), &ber);

	while (att) {
		bervals_ptr bv{ldap_get_values_len(ld.get(), res.get(), att)};

		if (bv && ldap_count_values_len(bv.get()) > 0)
			values[att] = string_view(bv.get()[0]->bv_val, bv.get()[0]->bv_len);
		else
			values[att] = "";

		att = ldap_next_attribute(ld.get(), res.get(), ber);
	}

	if (ber)
		ber_free(ber, 0);

	return values;
}

map<string, string> ldapobj::search(const string& filter, const vector<string>& atts) {
	return search_context(filter, atts, naming_context);
}

#ifdef _WIN32

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

void get_ldap_details_from_sid(PSID sid, string& name, string& email) {
	span sidsp((const uint8_t*)sid, GetLengthSid(sid));

	// FIXME - is there an easy way to have vector<uint8_t> as the map key rather than string?

	if (auto f = ldap_cache.find(string_view((char*)sidsp.data(), sidsp.size())); f != ldap_cache.end()) {
		const auto& p = *f;

		name = p.second.first;
		email = p.second.second;

		return;
	}

	ldapobj l;
	string binsid;

	binsid.reserve(3 * sidsp.size());
	for (auto b : sidsp) {
		binsid += "\\";
		binsid += hex_byte(b);
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

	ldap_cache.try_emplace(string((char*)sidsp.data(), (char*)sidsp.data() + sidsp.size()), make_pair(name, email));
}

#else

static string resolve_netbios_domain(ldapobj& l, string_view domain) {
	if (auto f = ldap_domain.find(domain); f != ldap_domain.end()) {
		const auto& p = *f;

		return p.second;
	}

	auto res = l.search_context("(nETBIOSName=" + string(domain) + ")", { "nCName" },
								"CN=Partitions,CN=Configuration," + l.naming_context);

	if (!res.contains("nCName"))
		throw runtime_error("Could not resolve NetBIOS domain " + string(domain) + ".");

	const auto& ret = res.at("nCName");

	ldap_domain.try_emplace(string(domain), ret);

	return ret;
}

void get_ldap_details_from_full_name(string_view username, string& name, string& email) {
	if (auto f = ldap_cache.find(username); f != ldap_cache.end()) {
		const auto& p = *f;

		name = p.second.first;
		email = p.second.second;

		return;
	}

	ldapobj l;
	string_view nbdomain;

	if (auto bs = username.find("\\"); bs != string::npos) {
		nbdomain = username.substr(0, bs);
		username = username.substr(bs + 1);
	} else
		throw runtime_error("Domain not provided in username.");

	auto domain = resolve_netbios_domain(l, nbdomain);

	auto ret = l.search_context("(sAMAccountName=" + string(username) + ")", { "givenName", "sn", "name", "mail" },
								domain);

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

	ldap_cache.try_emplace(string(username), make_pair(name, email));
}

void get_ldap_details_from_name(string_view username, string& name, string& email) {
	if (auto f = ldap_cache.find(username); f != ldap_cache.end()) {
		const auto& p = *f;

		name = p.second.first;
		email = p.second.second;

		return;
	}

	ldapobj l;

	auto ret = l.search("(sAMAccountName=" + string(username) + ")", { "givenName", "sn", "name", "mail" });

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

	ldap_cache.try_emplace(string(username), make_pair(name, email));
}

#endif
