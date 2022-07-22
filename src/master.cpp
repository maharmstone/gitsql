#include "gitsql.h"
#include "aes.h"

using namespace std;

extern string db_username, db_password;

struct linked_server {
	linked_server(string_view srvname, string_view name, span<const uint8_t> pwdhash, string_view srvproduct,
				  string_view providername, string_view datasource, string_view providerstring) :
				  srvname(srvname), name(name), pwdhash(pwdhash.begin(), pwdhash.end()), srvproduct(srvproduct),
				  providername(providername), datasource(datasource), providerstring(providerstring) {
	}

	string srvname;
	string name;
	vector<uint8_t> pwdhash;
	string srvproduct;
	string providername;
	string datasource;
	string providerstring;
};

static vector<uint8_t> aes256_cbc_decrypt(span<const std::byte> key, span<const uint8_t> iv, span<const uint8_t> enc) {
	vector<uint8_t> ret;
	AES_ctx ctx;

	ret.resize(enc.size());
	memcpy(ret.data(), enc.data(), enc.size());

	AES256_init_ctx_iv(&ctx, (uint8_t*)key.data(), iv.data());
	AES256_CBC_decrypt_buffer(&ctx, ret.data(), ret.size());

	fmt::print("ret.size() = {}\n", ret.size());

	return ret;
}

void dump_master(const string& db_server, unsigned int repo, span<std::byte> smk) {
	vector<linked_server> servs;

	if (smk.size() == 16)
		throw runtime_error("3DES SMK not supported.");
	else if (smk.size() != 32)
		throw runtime_error("Invaldi SMK length.");

	tds::options opts(db_server, db_username, db_password);

	// FIXME - different instances?

	opts.app_name = db_app;
	opts.port = 1434; // FIXME - find DAC port by querying server

	{
		tds::tds dac(opts);

		{
			tds::query sq(dac, R"(SELECT sysservers.srvname, syslnklgns.name, syslnklgns.pwdhash, sysservers.srvproduct, sysservers.providername, sysservers.datasource, sysservers.providerstring
FROM master.sys.syslnklgns
JOIN master.sys.sysservers on syslnklgns.srvid=sysservers.srvid
WHERE LEN(syslnklgns.pwdhash) > 0)");

			while (sq.fetch_row()) {
				servs.emplace_back((string)sq[0], (string)sq[1], sq[2].val, (string)sq[3], (string)sq[4], (string)sq[5], (string)sq[6]);
			}
		}
	}

	for (const auto& serv : servs) {
		fmt::print("srvname: {}\n", serv.srvname);
		fmt::print("name: {}\n", serv.name);
		fmt::print("srvproduct: {}\n", serv.srvproduct);
		fmt::print("providername: {}\n", serv.providername);
		fmt::print("datasource: {}\n", serv.datasource);
		fmt::print("providerstring: {}\n", serv.providerstring);
		fmt::print("pwdhash: ");

		for (auto b : serv.pwdhash) {
			fmt::print("{:02x} ", b);
		}
		fmt::print("\n");

		auto hash = span(serv.pwdhash);
		auto plaintext = aes256_cbc_decrypt(smk, hash.subspan(4, 16), hash.subspan(20));

		fmt::print("plaintext: ");
		for (auto b : plaintext) {
			fmt::print("{:02x} ", b);
		}
		fmt::print("\n");

		if (plaintext.size() >= 8 && *(uint32_t*)plaintext.data() == 0xbaadf00d) {
			auto len = *(uint16_t*)&plaintext[6];
			auto u16sv = u16string_view((char16_t*)&plaintext[8], len / sizeof(char16_t));

			fmt::print("\"{}\"\n", tds::utf16_to_utf8(u16sv));
		}

		fmt::print("\n");
	}

	// FIXME

	fmt::print("FIXME\n");
}
