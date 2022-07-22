#include "gitsql.h"
#include "aes.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

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

struct principal {
	principal(string_view name, span<const uint8_t> sid, string_view type, string_view dbname,
			  string_view lang) :
			  name(name), sid(sid.begin(), sid.end()), type(type), dbname(dbname), lang(lang) {
	}

	string name;
	vector<uint8_t> sid;
	string type;
	string dbname;
	string lang;
};

static vector<uint8_t> aes256_cbc_decrypt(span<const std::byte> key, span<const uint8_t> iv, span<const uint8_t> enc) {
	vector<uint8_t> ret;
	AES_ctx ctx;

	ret.resize(enc.size());
	memcpy(ret.data(), enc.data(), enc.size());

	AES256_init_ctx_iv(&ctx, (uint8_t*)key.data(), iv.data());
	AES256_CBC_decrypt_buffer(&ctx, ret.data(), ret.size());

	return ret;
}

static void dump_linked_servers(const tds::options& opts, tds::tds& tds, unsigned int commit, span<std::byte> smk) {
	vector<linked_server> servs;

	// FIXME - also log user mappings (syslnklgns.lgnid != 0)
	// FIXME - also log linked servers using Kerberos passthrough

	{
		tds::tds dac(opts);

		{
			tds::query sq(dac, R"(SELECT sysservers.srvname, syslnklgns.name, syslnklgns.pwdhash, sysservers.srvproduct, sysservers.providername, sysservers.datasource, sysservers.providerstring
FROM master.sys.syslnklgns
JOIN master.sys.sysservers on syslnklgns.srvid = sysservers.srvid
WHERE LEN(syslnklgns.pwdhash) > 0 AND syslnklgns.lgnid = 0)");

			while (sq.fetch_row()) {
				servs.emplace_back((string)sq[0], (string)sq[1], sq[2].val, (string)sq[3], (string)sq[4], (string)sq[5], (string)sq[6]);
			}
		}
	}

	for (const auto& serv : servs) {
		auto j = json::object();

		j["srvname"] = serv.srvname;
		j["username"] = serv.name;

		if (!serv.srvproduct.empty())
			j["srvproduct"] = serv.srvproduct;

		if (!serv.providername.empty())
			j["providername"] = serv.providername;

		if (!serv.datasource.empty())
			j["datasource"] = serv.datasource;

		if (!serv.providerstring.empty())
			j["providerstring"] = serv.providerstring;

		auto hash = span(serv.pwdhash);
		auto plaintext = aes256_cbc_decrypt(smk, hash.subspan(4, 16), hash.subspan(20));

		if (plaintext.size() >= 8 && *(uint32_t*)plaintext.data() == 0xbaadf00d) {
			auto len = *(uint16_t*)&plaintext[6];
			auto u16sv = u16string_view((char16_t*)&plaintext[8], len / sizeof(char16_t));

			j["password"] = tds::utf16_to_utf8(u16sv);
		}

		auto fn = serv.srvname;

		// make NTFS-friendly

		for (auto& c : fn) {
			if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
				c = '-';
		}

		tds.run("INSERT INTO Restricted.GitFiles(id, filename, data) VALUES(?, ?, CONVERT(VARBINARY(MAX), ?))", commit, "linked_servers/" + fn + ".json", j.dump(3) + "\n");
	}
}

static string sid_to_string(span<const uint8_t> sid) {
	uint64_t first_part;
	string ret;

	if (sid.empty() || sid.front() != 0x01)
		return "(invalid SID)";

	sid = sid.subspan(1);

	if (sid.empty())
		return "(invalid SID)";

	auto num_parts = sid.front();

	sid = sid.subspan(1);

	if (sid.size() < 6 + (num_parts * sizeof(uint32_t)))
		return "(invalid SID)";

	first_part = (uint64_t)sid[0] << 40;
	first_part |= (uint64_t)sid[1] << 32;
	first_part |= sid[2] << 24;
	first_part |= sid[3] << 16;
	first_part |= sid[4] << 8;
	first_part |= sid[5];

	sid = sid.subspan(6);

	ret = fmt::format("S-1-{}", first_part);

	for (unsigned int i = 0; i < num_parts; i++) {
		auto num = *(uint32_t*)sid.data();

		ret += fmt::format("-{}", num);

		sid = sid.subspan(sizeof(uint32_t));
	}

	return ret;
}

static void dump_principals(const tds::options& opts, tds::tds& tds, unsigned int commit) {
	vector<principal> principals;

	{
		tds::tds dac(opts);

		{
			tds::query sq(dac, "SELECT name, sid, type, dbname, lang FROM master.sys.sysxlgns");

			while (sq.fetch_row()) {
				principals.emplace_back((string)sq[0], sq[1].val, (string)sq[2], (string)sq[3], (string)sq[4]);
			}
		}
	}

	for (auto& p : principals) {
		auto j = json::object();

		j["name"] = p.name;

		if (p.type == "C" || p.type == "M" || p.type == "G" || p.type == "U")
			j["sid"] = sid_to_string(p.sid);

		j["type"] = p.type;

		if (!p.dbname.empty())
			j["dbname"] = p.dbname;

		if (!p.lang.empty())
			j["lang"] = p.lang;

		auto fn = p.name;

		// make NTFS-friendly

		for (auto& c : fn) {
			if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
				c = '-';
		}

		tds.run("INSERT INTO Restricted.GitFiles(id, filename, data) VALUES(?, ?, CONVERT(VARBINARY(MAX), ?))", commit, "principals/" + fn + ".json", j.dump(3) + "\n");
	}
}

void dump_master(const string& db_server, unsigned int repo, span<std::byte> smk) {
	if (smk.size() == 16)
		throw runtime_error("3DES SMK not supported.");
	else if (smk.size() != 32)
		throw runtime_error("Invalid SMK length.");

	tds::options opts(db_server, db_username, db_password);

	// FIXME - different instances?

	opts.app_name = db_app;
	opts.port = 1434; // FIXME - find DAC port by querying server

	tds::tds tds(db_server, db_username, db_password, db_app);

	tds::trans trans(tds);

	unsigned int commit;

	{
		tds::query sq(tds, "INSERT INTO Restricted.Git(repo, username, description, dto) OUTPUT inserted.id VALUES(?, ?, 'Update', SYSDATETIMEOFFSET())",
					  repo, get_current_username());

		if (!sq.fetch_row())
			throw runtime_error("Error inserting entry into Restricted.Git.");

		if (sq[0].is_null)
			throw runtime_error("Returned commit ID was NULL.");

		commit = (unsigned int)sq[0];
	}

	// clear existing files
	tds.run("INSERT INTO Restricted.GitFiles(id) VALUES(?)", commit);

	dump_linked_servers(opts, tds, commit, smk);
	dump_principals(opts, tds, commit);

	trans.commit();
}
