#include "gitsql.h"

using namespace std;

extern string db_username, db_password;

void dump_master(const string& db_server, unsigned int repo, span<std::byte> smk) {
	// FIXME - different instances?

	for (auto b : smk) {
		fmt::print("{:02x} ", b);
	}
	fmt::print("\n");

	// FIXME

	fmt::print("FIXME\n");
}
