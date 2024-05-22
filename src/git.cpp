#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <zlib.h>
#include "git.h"
#include "gitsql.h"

#ifndef _WIN32
#include <fcntl.h>
#endif

// FIXME - remove this once CI lets us
#ifndef __cpp_lib_out_ptr
template<typename T, typename Deleter>
class out_ptr {
public:
	out_ptr(std::unique_ptr<T*, Deleter>& up) : up(up) {
		ptr = up.get();
	}

	~out_ptr() {
		if (ptr != up.get())
			up.reset(ptr);
	}

	operator T**() {
		return &ptr;
	}

private:
	T* ptr;
	std::unique_ptr<T*, Deleter>& up;
};
#endif

using namespace std;

class git_exception : public exception {
public:
	git_exception(int error, string_view func) {
		auto lg2err = git_error_last();

		if (lg2err && lg2err->message) {
			msg = string(func) + " failed (" + lg2err->message;

			while (!msg.empty() && (msg.back() == '\r' || msg.back() == '\n')) {
				msg.pop_back();
			}

			msg += ")";
		} else
			msg = string(func) + " failed (error " + to_string(error) + ")";
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

private:
	string msg;
};

static constexpr auto jan1970 = chrono::duration_cast<chrono::seconds>(chrono::time_point{chrono::sys_days{1970y/chrono::January/1d}}.time_since_epoch());

GitSignature::GitSignature(const string& user, const string& email, const optional<tds::datetimeoffset>& dto) {
	if (dto.has_value()) {
		auto tp = (chrono::time_point<chrono::system_clock>)dto.value();
		auto secs = chrono::duration_cast<chrono::seconds>(tp.time_since_epoch()) - jan1970;

		if (auto ret = git_signature_new(out_ptr(sig), user.c_str(), email.c_str(), secs.count(), dto.value().offset))
			throw git_exception(ret, "git_signature_new");
	} else {
		if (auto ret = git_signature_now(out_ptr(sig), user.c_str(), email.c_str()))
			throw git_exception(ret, "git_signature_now");
	}
}

GitTree::GitTree(const git_commit* commit) {
	if (auto ret = git_commit_tree(out_ptr(tree), commit))
		throw git_exception(ret, "git_commit_tree");
}

GitTree::GitTree(const GitRepo& repo, const git_oid& oid) {
	if (auto ret = git_tree_lookup(out_ptr(tree), repo.repo.get(), &oid))
		throw git_exception(ret, "git_tree_lookup");
}

GitTree::GitTree(const GitRepo& repo, const GitTreeEntry& gte) {
	git_object* tmp = nullptr;

	if (auto ret = git_tree_entry_to_object(&tmp, repo.repo.get(), gte))
		throw git_exception(ret, "git_tree_entry_to_object");

	tree.reset((git_tree*)tmp);
}

git_tree_entry_ptr GitTree::entry_bypath(const string& path) {
	git_tree_entry_ptr gte;

	int ret = git_tree_entry_bypath(out_ptr(gte), tree.get(), path.c_str());

	if (ret == GIT_ENOTFOUND)
		return nullptr;
	else if (ret)
		throw git_exception(ret, "git_tree_entry_bypath");

	return gte;
}

GitRepo::GitRepo(const string& dir) {
	if (auto ret = git_repository_open(out_ptr(repo), dir.c_str()))
		throw git_exception(ret, "git_repository_open");
}

bool GitRepo::reference_name_to_id(git_oid* out, const string& name) {
	auto ret = git_reference_name_to_id(out, repo.get(), name.c_str());

	if (ret == 0)
		return true;
	else if (ret == GIT_ENOTFOUND)
		return false;
	else
		throw git_exception(ret, "git_reference_name_to_id");
}

git_commit_ptr GitRepo::commit_lookup(const git_oid* oid) {
	git_commit_ptr commit;

	if (auto ret = git_commit_lookup(out_ptr(commit), repo.get(), oid))
		throw git_exception(ret, "git_commit_lookup");

	return commit;
}

git_oid GitRepo::commit_create(const GitSignature& author, const GitSignature& committer, const string& message, const GitTree& tree,
							   git_commit* parent) {
	git_oid id;

	if (parent) {
		if (auto ret = git_commit_create_v(&id, repo.get(), nullptr, author.sig.get(), committer.sig.get(), nullptr, message.c_str(), tree.tree.get(), 1, parent))
			throw git_exception(ret, "git_commit_create_v");
	} else {
		if (auto ret = git_commit_create_v(&id, repo.get(), nullptr, author.sig.get(), committer.sig.get(), nullptr, message.c_str(), tree.tree.get(), 0))
			throw git_exception(ret, "git_commit_create_v");
	}

	return id;
}

#ifdef _WIN32
static void rename_open_file(HANDLE h, const filesystem::path& fn) {
	vector<uint8_t> buf;
	auto dest = fn.u16string();

	buf.resize(offsetof(FILE_RENAME_INFO, FileName) + ((dest.length() + 1) * sizeof(char16_t)));

	auto fri = (FILE_RENAME_INFO*)buf.data();

	fri->ReplaceIfExists = true;
	fri->RootDirectory = nullptr;
	fri->FileNameLength = (DWORD)(dest.length() * sizeof(char16_t));
	memcpy(fri->FileName, dest.data(), fri->FileNameLength);
	fri->FileName[dest.length()] = 0;

	if (!SetFileInformationByHandle(h, FileRenameInfo, fri, (DWORD)buf.size()))
		throw last_error("SetFileInformationByHandle", GetLastError());
}
#endif

static filesystem::path get_object_filename(const filesystem::path& repopath, const git_oid& oid) {
	filesystem::path file = repopath;

	file /= "objects";

	file /= format("{:02x}", oid.id[0]);

	create_directory(file);

	file /= format("{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
						oid.id[1], oid.id[2], oid.id[3], oid.id[4], oid.id[5], oid.id[6], oid.id[7], oid.id[8],
						oid.id[9], oid.id[10], oid.id[11], oid.id[12], oid.id[13], oid.id[14], oid.id[15],
						oid.id[16], oid.id[17], oid.id[18], oid.id[19]);

	return file;
}

git_oid GitRepo::blob_create_from_buffer(span<const uint8_t> data) {
	git_oid blob;
	git_odb_ptr odb;

	if (auto ret = git_odb_hash(&blob, data.data(), data.size(), GIT_OBJECT_BLOB))
		throw git_exception(ret, "git_odb_hash");

	if (auto ret = git_repository_odb(out_ptr(odb), repo.get()))
		throw git_exception(ret, "git_repository_odb");

	if (git_odb_exists(odb.get(), &blob) == 1)
		return blob;

	string repopath = git_repository_path(repo.get()); // FIXME - should be Unicode on Windows

#ifdef _WIN32
	for (auto& c : repopath) {
		if (c == '/')
			c = '\\';
	}

	filesystem::path tmpfile = repopath;
	tmpfile /= "newblob";

	unique_handle h{CreateFileW((WCHAR*)tmpfile.u16string().c_str(), FILE_WRITE_DATA | DELETE, 0, nullptr, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, nullptr)};

	if (h.get() == INVALID_HANDLE_VALUE)
		throw last_error("CreateFile(" + tmpfile.string() + ")", GetLastError());
#else
	unique_handle h{open(repopath.c_str(), O_TMPFILE | O_RDWR, 0644)};

	if (h.get() == -1)
		throw errno_error("open", errno);
#endif

	string header = "blob " + to_string(data.size());
	header.push_back(0);

	z_stream strm;
	int err;
	uint8_t zbuf[4096];
	bool writing_header = true;

	memset(&strm, 0, sizeof(strm));

	err = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	if (err != Z_OK)
		throw formatted_error("deflateInit failed (error {})", err);

	strm.next_in = (unsigned char*)header.data();
	strm.avail_in = (unsigned int)header.length();
	strm.next_out = zbuf;
	strm.avail_out = sizeof(zbuf);

	do {

		err = deflate(&strm, writing_header ? Z_NO_FLUSH : Z_FINISH);

		if (err != Z_OK && err != Z_STREAM_END) {
			deflateEnd(&strm);
			throw formatted_error("deflate returned {}", err);
		}

		if (strm.avail_out != sizeof(zbuf)) {
#ifdef _WIN32
			DWORD written;

			if (!WriteFile(h.get(), zbuf, sizeof(zbuf) - strm.avail_out, &written, nullptr)) {
				deflateEnd(&strm);
				throw last_error("WriteFile(" + tmpfile.string() + ")", GetLastError());
			}
#else
			do {
				auto ret = write(h.get(), zbuf, sizeof(zbuf) - strm.avail_out);

				if (ret == -1) {
					if (errno == EINTR)
						continue;

					throw errno_error("write", errno);
				}

				if (ret != (decltype(ret))(sizeof(zbuf) - strm.avail_out))
					throw runtime_error("short write");

				break;
			} while (true);
#endif

			strm.next_out = zbuf;
			strm.avail_out = sizeof(zbuf);
		}

		if (strm.avail_in == 0 && writing_header) {
			writing_header = false;
			strm.next_in = (unsigned char*)data.data();
			strm.avail_in = (unsigned int)data.size();
		}
	} while (err != Z_STREAM_END);

	deflateEnd(&strm);

	auto blobfile = get_object_filename(repopath, blob);

#ifdef _WIN32
	rename_open_file(h.get(), blobfile);
#else
	string procfile = "/proc/self/fd/" + to_string(h.get());

	if (linkat(AT_FDCWD, procfile.c_str(), AT_FDCWD, blobfile.string().c_str(), AT_SYMLINK_FOLLOW) == -1)
		throw errno_error("linkat", errno);
#endif

	return blob;
}

git_oid GitRepo::tree_create_updated(const GitTree& baseline, span<const git_tree_update> updates) {
	git_oid oid;

	if (auto ret = git_tree_create_updated(&oid, repo.get(), baseline.tree.get(), updates.size(), updates.data()))
		throw git_exception(ret, "git_tree_create_updated");

	return oid;
}

git_oid GitRepo::index_tree_id() const {
	git_index_ptr index;
	git_oid tree_id;

	if (auto ret = git_repository_index(out_ptr(index), repo.get()))
		throw git_exception(ret, "git_repository_index");

	if (auto ret = git_index_write_tree(&tree_id, index.get()))
		throw git_exception(ret, "git_index_write_tree");

	return tree_id;
}

static void update_git_no_parent(GitRepo& repo, const GitSignature& sig, const string& description, const list<git_file2>& files, const string& branch) {
	git_oid oid;

	GitTree empty_tree(repo, repo.index_tree_id());

	{
		struct gtwctx {
			vector<git_tree_update> upd;
			list<string> strs;
		};

		gtwctx ctx;

		git_tree_walk(empty_tree.tree.get(), GIT_TREEWALK_PRE, [](const char* root, const git_tree_entry* entry, void* payload) {
			auto& ctx = *(gtwctx*)payload;

			if (strcmp(root, ""))
				return 0;

			ctx.strs.emplace_back(git_tree_entry_name(entry));

			ctx.upd.emplace_back();
			ctx.upd.back().action = GIT_TREE_UPDATE_REMOVE;
			ctx.upd.back().path = ctx.strs.back().c_str();

			return 0;
		}, &ctx);

		if (!ctx.upd.empty()) {
			oid = repo.tree_create_updated(empty_tree, ctx.upd);

			empty_tree = GitTree(repo, oid);
		}
	}

	{
		vector<git_tree_update> upd;

		upd.reserve(files.size());

		for (const auto& f : files) {
			upd.emplace_back();

			auto& u = upd.back();

			if (f.oid.has_value()) {
				u.action = GIT_TREE_UPDATE_UPSERT;
				u.id = f.oid.value();
			}

			u.filemode = GIT_FILEMODE_BLOB;
			u.path = f.filename.c_str();
		}

		if (upd.empty())
			return;

		oid = repo.tree_create_updated(empty_tree, upd);
	}

	GitTree tree(repo, oid);

	auto commit_oid = repo.commit_create(sig, sig, description, tree);

	auto commit = repo.commit_lookup(&commit_oid);

	repo.branch_create(branch.empty() ? "master" : branch, commit.get(), true);
}

static void do_clear_all(const GitRepo& repo, const GitTree& tree, const string& prefix,
						 list<git_file2>& files, unordered_set<string>& filenames) {
	auto c = tree.entrycount();

	for (size_t i = 0; i < c; i++) {
		GitTreeEntry gte(tree, i);
		string name = prefix + gte.name();

		if (gte.type() == GIT_OBJECT_TREE) {
			GitTree subtree(repo, gte);

			do_clear_all(repo, subtree, name + "/", files, filenames);
		} else {
			if (!filenames.contains(name)) {
				files.emplace_back(name, nullopt);
				filenames.emplace(name);
			}
		}
	}
}

git_reference_ptr GitRepo::branch_lookup(const std::string& branch_name, git_branch_t branch_type) {
	git_reference_ptr ref;

	auto ret = git_branch_lookup(out_ptr(ref), repo.get(), branch_name.c_str(), branch_type);

	if (ret == GIT_ENOTFOUND)
		return nullptr;
	else if (ret)
		throw git_exception(ret, "git_branch_lookup");

	return ref;
}

void GitRepo::branch_create(const string& branch_name, const git_commit* target, bool force) {
	git_reference_ptr ref;

	if (auto ret = git_branch_create(out_ptr(ref), repo.get(), branch_name.c_str(), target, force ? 1 : 0))
		throw git_exception(ret, "git_branch_create");
}

void GitRepo::reference_create(const string& name, const git_oid& id, bool force, const string& log_message) {
	git_reference_ptr ref;

	if (auto ret = git_reference_create(out_ptr(ref), repo.get(), name.c_str(), &id, force ? 1 : 0, log_message.c_str()))
		throw git_exception(ret, "git_reference_create");
}

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, list<git_file2>& files,
				bool clear_all, const optional<tds::datetimeoffset>& dto, const string& branch) {
	GitSignature sig(user, email, dto);

	git_oid parent_id;

	bool parent_found = repo.reference_name_to_id(&parent_id, branch.empty() ? "refs/heads/master" : "refs/heads/" + branch);

	if (!parent_found) {
		update_git_no_parent(repo, sig, description, files, branch);
		return;
	}

	auto parent = repo.commit_lookup(&parent_id);

	GitTree parent_tree(parent.get());
	git_oid oid;

	if (clear_all) {
		unordered_set<string> filenames;

		for (const auto& f : files) {
			filenames.emplace(f.filename);
		}

		do_clear_all(repo, parent_tree, "", files, filenames);
	}

	{
		vector<git_tree_update> upd;

		upd.reserve(files.size());

		for (const auto& f : files) {
			upd.emplace_back();

			auto& u = upd.back();

			if (!f.oid.has_value()) {
				auto gte = parent_tree.entry_bypath(f.filename);

				if (!gte) {
					upd.pop_back();
					continue;
				}

				u.action = GIT_TREE_UPDATE_REMOVE;
			} else {
				u.action = GIT_TREE_UPDATE_UPSERT;
				u.id = f.oid.value();
			}

			u.filemode = GIT_FILEMODE_BLOB;
			u.path = f.filename.c_str();
		}

		if (upd.empty())
			return;

		oid = repo.tree_create_updated(parent_tree, upd);
	}

	if (!memcmp(&oid, git_tree_id(parent_tree.tree.get()), sizeof(git_oid))) // no changes - avoid doing empty commit
		return;

	GitTree tree(repo, oid);

	auto commit_oid = repo.commit_create(sig, sig, description, tree, parent.get());

	repo.reference_create(branch.empty() ? "refs/heads/master" : "refs/heads/" + branch,
						  commit_oid, true, "branch updated");
}

GitIndex::GitIndex(const GitRepo& repo) {
	if (auto ret = git_repository_index(out_ptr(index), repo.repo.get()))
		throw git_exception(ret, "git_repository_index");
}

void GitIndex::write_tree(git_oid* oid) {
	if (auto ret = git_index_write_tree(oid, index.get()))
		throw git_exception(ret, "git_index_write_tree");
}

void GitIndex::add_bypath(const string& fn) {
	if (auto ret = git_index_add_bypath(index.get(), fn.c_str()))
		throw git_exception(ret, "git_index_add_bypath");
}

void GitIndex::remove_bypath(const string& fn) {
	if (auto ret = git_index_remove_bypath(index.get(), fn.c_str()))
		throw git_exception(ret, "git_index_remove_bypath");
}

void GitIndex::clear() {
	if (auto ret = git_index_clear(index.get()))
		throw git_exception(ret, "git_index_clear");
}

size_t GitTree::entrycount() const {
	return git_tree_entrycount(tree.get());
}

GitTreeEntry::GitTreeEntry(const GitTree& tree, size_t idx) {
	gte = git_tree_entry_byindex(tree.tree.get(), idx);

	if (!gte)
		throw runtime_error("git_tree_entry_byindex returned NULL.");
}

string GitTreeEntry::name() {
	return git_tree_entry_name(gte);
}

git_object_t GitTreeEntry::type() {
	return git_tree_entry_type(gte);
}

GitTree::GitTree(const GitRepo& repo, const string& rev) {
	git_object* tmp = nullptr;

	if (auto ret = git_revparse_single(&tmp, repo.repo.get(), rev.c_str()))
		throw git_exception(ret, "git_revparse_single");

	tree.reset((git_tree*)tmp);
}

GitBlob::GitBlob(const GitTree& tree, const string& path) {
	if (auto ret = git_object_lookup_bypath(out_ptr(obj), (git_object*)tree.tree.get(), path.c_str(), GIT_OBJECT_BLOB))
		throw git_exception(ret, "git_object_lookup_bypath");
}

GitBlob::operator string() const {
	return string((char*)git_blob_rawcontent((git_blob*)obj.get()), git_blob_rawsize((git_blob*)obj.get()));
}

void GitRepo::checkout_head(const git_checkout_options* opts) {
	if (auto ret = git_checkout_head(repo.get(), opts))
		throw git_exception(ret, "git_checkout_head");
}

bool GitRepo::branch_is_head(const std::string& name) {
	git_reference_ptr ref;

	auto ret = git_reference_lookup(out_ptr(ref), repo.get(), ("refs/heads/" + name).c_str());

	if (ret == GIT_ENOTFOUND)
		return false;
	else if (ret)
		throw git_exception(ret, "git_reference_lookup");

	ret = git_branch_is_head(ref.get());

	if (ret < 0)
		throw git_exception(ret, "git_branch_is_head");

	return ret;
}

bool GitRepo::is_bare() {
	return git_repository_is_bare(repo.get());
}

string GitRepo::branch_upstream_remote(const string& refname) {
	string remote;
	git_buf buf = {};

	auto ret = git_branch_upstream_remote(&buf, repo.get(), refname.c_str());

	if (ret == GIT_ENOTFOUND)
		return "";

	if (ret)
		throw git_exception(ret, "git_branch_upstream_remote");

	try {
		remote.assign(string_view(buf.ptr, buf.size));
	} catch (...) {
		git_buf_dispose(&buf);
		throw;
	}

	git_buf_dispose(&buf);

	return remote;
}

GitRemote GitRepo::remote_lookup(const string& name) {
	git_remote* r;

	if (auto ret = git_remote_lookup(&r, repo.get(), name.c_str()); ret)
		throw git_exception(ret, "git_remote_lookup");

	return r;
}

void GitRemote::push(const std::vector<std::string>& refspecs, const git_push_options& opts) {
	vector<const char*> v;
	git_strarray specs;

	v.reserve(refspecs.size());

	for (const auto& s : refspecs) {
		v.push_back(s.c_str());
	}

	specs.strings = (char**)v.data();
	specs.count = refspecs.size();

	if (auto ret = git_remote_push(r.get(), &specs, &opts); ret)
		throw git_exception(ret, "git_remote_push");
}

static vector<pair<string, string>> load_ssh_keys() {
	vector<pair<string, string>> keys;

	static const string key_names[] = {
		"id_rsa",
		"id_ecdsa",
		"id_ecdsa_sk",
		"id_ed25519",
		"id_ed25519_sk",
		"id_xmss",
		"id_dsa"
	};

#ifdef _WIN32
	const char* home = getenv("USERPROFILE");
#else
	const char* home = getenv("HOME");
#endif

	if (!home)
		return {};

	auto sshdir = filesystem::path{home} / ".ssh";

	if (!filesystem::exists(sshdir))
		return {};

	for (const auto& n : key_names) {
		auto fn = sshdir / n;

		if (!filesystem::exists(fn))
			continue;

		auto fnpub = sshdir / (n + ".pub");

		if (!filesystem::exists(fnpub))
			continue;

		string private_key, public_key;

		private_key.resize(file_size(fn));

		{
			ifstream f(fn, ios::binary);

			if (!f.is_open()) {
				cerr << "Could not open " << fn.string() << " for reading." << endl;
				continue;
			}

			if (!f.read(private_key.data(), private_key.size())) {
				cerr << "Could not read " << fn.string() << "." << endl;
				continue;
			}
		}

		public_key.resize(file_size(fnpub));

		{
			ifstream f(fnpub, ios::binary);

			if (!f.is_open()) {
				cerr << "Could not open " << fnpub.string() << " for reading." << endl;
				continue;
			}

			if (!f.read(public_key.data(), public_key.size())) {
				cerr << "Could not read " << fnpub.string() << "." << endl;
				continue;
			}
		}

		keys.emplace_back(make_pair(public_key, private_key));
	}

	return keys;
}

void GitRepo::try_push(const string& ref) {
	auto remote = branch_upstream_remote(ref);

	if (remote.empty())
		return;

	auto r = remote_lookup(remote);

	git_push_options options = GIT_PUSH_OPTIONS_INIT;

	// FIXME - way of saying to use specific key? Or use libssh to parse ~/.ssh/config file?

	auto keys = load_ssh_keys();

	if (keys.empty())
		throw runtime_error("No SSH keys loaded.");

	struct options_payload {
		optional<string> status;
		vector<pair<string, string>> keys;
		unsigned int key_num = 0;
	} p;

	swap(p.keys, keys);

	options.callbacks.payload = &p;

	options.callbacks.credentials = [](git_credential** out, const char*, const char* username_from_url,
									   unsigned int allowed_types, void* payload) -> int {
		if (!(allowed_types & GIT_CREDENTIAL_SSH_MEMORY))
			return GIT_PASSTHROUGH;

		auto& p = *(options_payload*)payload;

		if (p.key_num == p.keys.size())
			return GIT_EAUTH;

		auto ret = git_credential_ssh_key_memory_new(out, username_from_url, p.keys[p.key_num].first.c_str(),
													 p.keys[p.key_num].second.c_str(), "");

		p.key_num++;

		return ret;
	};

	options.callbacks.certificate_check = [](git_cert*, int, const char*, void*) -> int {
		// FIXME - parse ~/.ssh/known_hosts?
		return 0;
	};

	options.callbacks.push_update_reference = [](const char*, const char* status, void* payload) -> int {
		auto& p = *(options_payload*)payload;

		if (status)
			p.status = status;

		return 0;
	};

	r.push({ ref }, options);

	if (p.status.has_value())
		throw runtime_error("push failed: " + p.status.value());
}

void git_update::add_file(string_view filename, string_view data) {
	{
		lock_guard lg(lock);

		files.emplace_back(filename, data);
	}

	cv.notify_one();
}

void git_update::run(stop_token st) noexcept {
	try {
		do {
			decltype(files) local_files;

			{
				unique_lock ul(lock);

				cv.wait(ul, st, [&]{ return !files.empty(); });

				local_files.splice(local_files.end(), files);
			}

			if (local_files.empty() && st.stop_requested())
				break;

			while (!local_files.empty()) {
				auto f = move(local_files.front());

				local_files.pop_front();

				if (f.data.has_value())
					files2.emplace_back(f.filename, repo.blob_create_from_buffer(f.data.value()));
				else
					files2.emplace_back(f.filename, nullopt);
			}
		} while (true);
	} catch (...) {
		teptr = current_exception();
	}
}

void git_update::start() {
	t = jthread([](stop_token st, git_update* gu) {
		gu->run(st);
	}, this);
}

void git_update::stop() {
	t.request_stop();
	t.join();

	if (teptr)
		rethrow_exception(teptr);
}

