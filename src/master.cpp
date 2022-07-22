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

void dump_master(const string& db_server, unsigned int repo, span<std::byte> smk) {
	vector<linked_server> servs;

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
				fmt::print("!\n");
				servs.emplace_back((string)sq[0], (string)sq[1], sq[2].val);
			}
		}
	}

	for (auto b : smk) {
		fmt::print("{:02x} ", b);
	}
	fmt::print("\n");

	// FIXME

	fmt::print("FIXME\n");
}
