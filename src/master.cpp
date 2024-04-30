#include "gitsql.h"
#include "aes.h"
#include "git.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

extern string db_username, db_password;

struct user_pass {
	user_pass() = default;

	user_pass(string_view username, span<const uint8_t> pwdhash) :
			 username(username), pwdhash(pwdhash.begin(), pwdhash.end()) {
	}

	string username;
	vector<uint8_t> pwdhash;
};

struct linked_server {
	linked_server(unsigned int srvid, string_view srvname, string_view srvproduct,
				  string_view providername, string_view datasource, string_view providerstring,
				  string_view catalog) :
				  srvid(srvid), srvname(srvname), srvproduct(srvproduct),
				  providername(providername), datasource(datasource), providerstring(providerstring),
				  catalog(catalog) {
	}

	unsigned int srvid;
	string srvname;
	string srvproduct;
	string providername;
	string datasource;
	string providerstring;
	string catalog;
	user_pass up;
	map<string, user_pass> mappings;
};

struct linked_server_logins {
	linked_server_logins(unsigned int srvid, string_view name, span<const uint8_t> pwdhash,
						 unsigned int lgnid, string_view local_name) :
						 srvid(srvid), up(name, pwdhash), lgnid(lgnid), local_name(local_name) {
	}

	unsigned int srvid;
	user_pass up;
	unsigned int lgnid;
	string local_name;
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

static optional<string> decrypt_pwdhash(span<const uint8_t> hash, span<const std::byte> smk) {
	auto plaintext = aes256_cbc_decrypt(smk, hash.subspan(4, 16), hash.subspan(20));

	if (plaintext.size() < 8 || *(uint32_t*)plaintext.data() != 0xbaadf00d)
		return nullopt;

	auto len = *(uint16_t*)&plaintext[6];
	auto u16sv = u16string_view((char16_t*)&plaintext[8], len / sizeof(char16_t));

	return tds::utf16_to_utf8(u16sv);
}

static void dump_linked_servers(const tds::options& opts, git_update& gu, span<const std::byte> smk) {
	vector<linked_server> servs;
	vector<linked_server_logins> logins;

	// FIXME - log how undefined logins will be handled

	{
		tds::tds dac(opts);

		{
			tds::query sq(dac, "SELECT srvid, srvname, srvproduct, providername, datasource, providerstring, catalog FROM master.sys.sysservers WHERE srvid != 0");

			while (sq.fetch_row()) {
				servs.emplace_back((unsigned int)sq[0], (string)sq[1], (string)sq[2], (string)sq[3],
								   (string)sq[4], (string)sq[5], (string)sq[6]);
			}
		}

		{
			tds::query sq(dac, R"(SELECT syslnklgns.srvid, syslnklgns.name, syslnklgns.pwdhash, syslnklgns.lgnid, sysxlgns.name
FROM master.sys.syslnklgns
LEFT JOIN master.sys.sysxlgns ON sysxlgns.id = syslnklgns.lgnid
WHERE syslnklgns.pwdhash IS NOT NULL)");

			while (sq.fetch_row()) {
				logins.emplace_back((unsigned int)sq[0], (string)sq[1], sq[2].val, (unsigned int)sq[3], (string)sq[4]);
			}
		}
	}

	for (const auto& l : logins) {
		optional<reference_wrapper<linked_server>> servrw;

		for (auto& s : servs) {
			if (s.srvid == l.srvid) {
				servrw = s;
				break;
			}
		}

		if (!servrw)
			continue;

		auto& serv = servrw.value().get();

		if (l.lgnid == 0)
			serv.up = l.up;
		else
			serv.mappings.try_emplace(l.local_name, l.up);
	}

	for (const auto& serv : servs) {
		auto j = json::object();

		j["srvname"] = serv.srvname;

		if (!serv.up.username.empty())
			j["username"] = serv.up.username;

		if (!serv.srvproduct.empty())
			j["srvproduct"] = serv.srvproduct;

		if (!serv.providername.empty())
			j["providername"] = serv.providername;

		if (!serv.datasource.empty())
			j["datasource"] = serv.datasource;

		if (!serv.providerstring.empty())
			j["providerstring"] = serv.providerstring;

		if (!serv.catalog.empty())
			j["catalog"] = serv.catalog;

		if (!serv.up.pwdhash.empty()) {
			auto pw = decrypt_pwdhash(serv.up.pwdhash, smk);

			if (pw)
				j["password"] = pw.value();
		}

		if (!serv.mappings.empty()) {
			auto m = json::object();

			for (const auto& mp : serv.mappings) {
				auto v = json::object();

				v["username"] = mp.second.username;

				auto pw = decrypt_pwdhash(mp.second.pwdhash, smk);

				if (pw)
					v["password"] = pw.value();

				m[mp.first] = v;
			}

			j["mappings"] = m;
		}

		auto fn = serv.srvname;

		// make NTFS-friendly

		for (auto& c : fn) {
			if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
				c = '-';
		}

		gu.add_file("linked_servers/" + fn + ".json", j.dump(3) + "\n");
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
	first_part |= (uint64_t)sid[2] << 24;
	first_part |= (uint64_t)sid[3] << 16;
	first_part |= (uint64_t)sid[4] << 8;
	first_part |= sid[5];

	sid = sid.subspan(6);

	ret = format("S-1-{}", first_part);

	for (unsigned int i = 0; i < num_parts; i++) {
		auto num = *(uint32_t*)sid.data();

		ret += format("-{}", num);

		sid = sid.subspan(sizeof(uint32_t));
	}

	return ret;
}

static void dump_principals(const tds::options& opts, git_update& gu) {
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

		gu.add_file("principals/" + fn + ".json", j.dump(3) + "\n");
	}
}

static void dump_extended_stored_procedures(const tds::options& opts, git_update& gu) {
	vector<pair<string, string>> xps;

	{
		tds::tds dac(opts); // we don't actually need DAC for this

		tds::query sq(dac, "SELECT name, dll_name FROM master.sys.extended_procedures");

		while (sq.fetch_row()) {
			xps.emplace_back((string)sq[0], (string)sq[1]);
		}
	}

	for (const auto& [name, dll_name] : xps) {
		string sql = "EXEC sp_addextendedproc " + tds::value{name}.to_literal() + ", " + tds::value{dll_name}.to_literal() + ";\n";

		gu.add_file("extended_stored_procedures/" + name + ".sql", sql);
	}
}

void dump_master(string_view db_server, unsigned int repo_num, span<const std::byte> smk) {
	if (smk.size() == 16)
		throw runtime_error("3DES SMK not supported.");
	else if (smk.size() != 32)
		throw runtime_error("Invalid SMK length.");

	string repo_dir, master_server, branch;

	{
		tds::tds tds(db_server, db_username, db_password, db_app);
		tds::query sq(tds, "SELECT dir, server, branch FROM master.dbo.git_repo WHERE id = ?", repo_num);

		if (!sq.fetch_row())
			throw formatted_error("Repo {} not found in master.dbo.git_repo.", repo_num);

		repo_dir = (string)sq[0];
		master_server = (string)sq[1];
		branch = (string)sq[2];
	}

	if (master_server.empty())
		throw formatted_error("No server specified for repo {}.", repo_num);

	git_libgit2_init();
	git_libgit2_opts(GIT_OPT_ENABLE_STRICT_OBJECT_CREATION, false);
	git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, 0);

	GitRepo repo(repo_dir);

	tds::options opts("", db_username, db_password);

	// FIXME - different instances?

	opts.server = master_server;
	opts.app_name = db_app;
	opts.port = 1434; // FIXME - find DAC port by querying server

	git_update gu(repo);

	gu.start();

	dump_linked_servers(opts, gu, smk);
	dump_principals(opts, gu);
	dump_extended_stored_procedures(opts, gu);

	string name, email;

	get_current_user_details(name, email);

	gu.stop();

	update_git(repo, name, email, "Update", gu.files2, true, nullopt, branch);
}
