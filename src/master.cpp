#include "gitsql.h"

using namespace std;

extern string db_username, db_password;

struct linked_server {
	linked_server(string_view srvname, string_view name, span<const uint8_t> pwdhash) : srvname(srvname), name(name), pwdhash(pwdhash.begin(), pwdhash.end()) {
	}

	string srvname;
	string name;
	vector<uint8_t> pwdhash;
};

static vector<uint8_t> aes256_cbc_decrypt(span<const uint8_t> iv, span<const uint8_t> enc) {
	vector<uint8_t> ret;

	// FIXME

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
			tds::query sq(dac, R"(SELECT sysservers.srvname, syslnklgns.name, syslnklgns.pwdhash
FROM master.sys.syslnklgns
JOIN master.sys.sysservers on syslnklgns.srvid=sysservers.srvid
WHERE LEN(syslnklgns.pwdhash) > 0)");

			while (sq.fetch_row()) {
				servs.emplace_back((string)sq[0], (string)sq[1], sq[2].val);
			}
		}
	}

	for (const auto& serv : servs) {
		fmt::print("srvname: {}\n", serv.srvname);
		fmt::print("name: {}\n", serv.name);
		fmt::print("pwdhash: ");

		for (auto b : serv.pwdhash) {
			fmt::print("{:02x} ", b);
		}
		fmt::print("\n");

		auto hash = span(serv.pwdhash);
		auto plaintext = aes256_cbc_decrypt(hash.subspan(4, 16), hash.subspan(20));

		fmt::print("plaintext: ");
		for (auto b : plaintext) {
			fmt::print("{:02x} ", b);
		}
		fmt::print("\n");

		fmt::print("\n");
	}

	// FIXME

	fmt::print("FIXME\n");
}
