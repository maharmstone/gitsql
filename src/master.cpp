#include "gitsql.h"

using namespace std;

extern string db_username, db_password;

void dump_master(const string& db_server, unsigned int repo) {
	vector<uint8_t> smkenc;

	// FIXME - different instances?

	tds::tds tds(db_server, db_username, db_password, db_app);

	{
		tds::query sq(tds, "SELECT SUBSTRING(crypt_property, 9, LEN(crypt_property) - 8) FROM master.sys.key_encryptions WHERE key_id = 102 AND (thumbprint=0x03 OR thumbprint=0x0300000001)");

		if (!sq.fetch_row())
			throw runtime_error("Unable to fetch SMK from master.sys.key_encryptions.");

		smkenc.assign(sq[0].val.begin(), sq[0].val.end());
	}

	fmt::print("FIXME\n");

	// FIXME - decode SMK
}
