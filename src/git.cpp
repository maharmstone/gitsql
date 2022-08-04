#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <zlib.h>
#include "git.h"
#include "gitsql.h"

#ifndef _WIN32
#include <fcntl.h>
#endif

using namespace std;

class git_exception : public exception {
public:
	git_exception(int error, string_view func) {
		auto lg2err = git_error_last();

		if (lg2err && lg2err->message)
			msg = string(func) + " failed (" + lg2err->message + ")";
		else
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
	unsigned int ret;
	git_signature* tmp = nullptr;

	if (dto.has_value()) {
		auto tp = (chrono::time_point<chrono::system_clock>)dto.value();
		auto secs = chrono::duration_cast<chrono::seconds>(tp.time_since_epoch()) - jan1970;

		if ((ret = git_signature_new(&tmp, user.c_str(), email.c_str(), secs.count(), dto.value().offset)))
			throw git_exception(ret, "git_signature_new");
	} else {
		if ((ret = git_signature_now(&tmp, user.c_str(), email.c_str())))
			throw git_exception(ret, "git_signature_now");
	}

	sig.reset(tmp);
}

GitTree::GitTree(const git_commit* commit) {
	unsigned int ret;

	if ((ret = git_commit_tree(&tree, commit)))
		throw git_exception(ret, "git_commit_tree");
}

GitTree::GitTree(const GitRepo& repo, const git_oid& oid) {
	unsigned int ret;

	if ((ret = git_tree_lookup(&tree, repo.repo.get(), &oid)))
		throw git_exception(ret, "git_tree_lookup");
}

GitTree::GitTree(const GitRepo& repo, const GitTreeEntry& gte) {
	unsigned int ret;

	if ((ret = git_tree_entry_to_object((git_object**)&tree, repo.repo.get(), gte)))
		throw git_exception(ret, "git_tree_entry_to_object");
}

GitTree::~GitTree() {
	git_tree_free(tree);
}

bool GitTree::entry_bypath(git_tree_entry** out, const string& path) {
	int ret = git_tree_entry_bypath(out, tree, path.c_str());

	if (ret != 0 && ret != GIT_ENOTFOUND)
		throw git_exception(ret, "git_tree_entry_bypath");

	return ret == 0;
}

GitRepo::GitRepo(const string& dir) {
	git_repository* tmp = nullptr;
	unsigned int ret;

	if ((ret = git_repository_open(&tmp, dir.c_str())))
		throw git_exception(ret, "git_repository_open");

	repo.reset(tmp);
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
	unsigned int ret;
	git_commit* tmp = nullptr;

	if ((ret = git_commit_lookup(&tmp, repo.get(), oid)))
		throw git_exception(ret, "git_commit_lookup");

	return git_commit_ptr{tmp};
}

git_oid GitRepo::commit_create(const GitSignature& author, const GitSignature& committer, const string& message, const GitTree& tree,
							   git_commit* parent) {
	unsigned int ret;
	git_oid id;

	if (parent) {
		if ((ret = git_commit_create_v(&id, repo.get(), nullptr, author.sig.get(), committer.sig.get(), nullptr, message.c_str(), tree, 1, parent)))
			throw git_exception(ret, "git_commit_create_v");
	} else {
		if ((ret = git_commit_create_v(&id, repo.get(), nullptr, author.sig.get(), committer.sig.get(), nullptr, message.c_str(), tree, 0)))
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
	char fn[39];

	file /= "objects";

	sprintf(fn, "%02x", oid.id[0]);
	file /= fn;

	create_directory(file);

	sprintf(fn, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			oid.id[1], oid.id[2], oid.id[3], oid.id[4], oid.id[5], oid.id[6], oid.id[7], oid.id[8],
			oid.id[9], oid.id[10], oid.id[11], oid.id[12], oid.id[13], oid.id[14], oid.id[15],
			oid.id[16], oid.id[17], oid.id[18], oid.id[19]);
	file /= fn;

	return file;
}

git_oid GitRepo::blob_create_from_buffer(string_view data) {
	unsigned int ret;
	git_oid blob;
	unique_ptr<git_odb, decltype(&git_odb_free)> odb(nullptr, git_odb_free);

	if ((ret = git_odb_hash(&blob, data.data(), data.length(), GIT_OBJECT_BLOB)))
		throw git_exception(ret, "git_odb_hash");

	{
		git_odb* tmp;

		if ((ret = git_repository_odb(&tmp, repo.get())))
			throw git_exception(ret, "git_repository_odb");

		odb.reset(tmp);
	}

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

	string header = "blob " + to_string(data.length());
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
			strm.avail_in = (unsigned int)data.length();
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

git_oid GitRepo::tree_create_updated(const GitTree& baseline, size_t nupdates, const git_tree_update* updates) {
	unsigned int ret;
	git_oid oid;

	if ((ret = git_tree_create_updated(&oid, repo.get(), baseline, nupdates, updates)))
		throw git_exception(ret, "git_tree_create_updated");

	return oid;
}

GitDiff::GitDiff(const GitRepo& repo, const GitTree& old_tree, const GitTree& new_tree, const git_diff_options* opts) {
	unsigned int ret;

	if ((ret = git_diff_tree_to_tree(&diff, repo.repo.get(), old_tree, new_tree, opts)))
		throw git_exception(ret, "git_diff_tree_to_tree");
}

GitDiff::~GitDiff() {
	git_diff_free(diff);
}

size_t GitDiff::num_deltas() {
	return git_diff_num_deltas(diff);
}

git_oid GitRepo::index_tree_id() const {
	unsigned int ret;
	git_index* index;
	git_oid tree_id;

	if ((ret = git_repository_index(&index, repo.get())))
		throw git_exception(ret, "git_repository_index");

	if ((ret = git_index_write_tree(&tree_id, index))) {
		auto exc = git_exception(ret, "git_index_write_tree");
		git_index_free(index);
		throw exc;
	}

	git_index_free(index);

	return tree_id;
}

static void update_git_no_parent(GitRepo& repo, const GitSignature& sig, const string& description, const list<git_file>& files, const string& branch) {
	git_oid oid;

	GitTree empty_tree(repo, repo.index_tree_id());

	{
		vector<git_tree_update> upd(files.size());
		unsigned int nu = 0;

		for (const auto& f : files) {
			if (f.data.has_value()) {
				upd[nu].action = GIT_TREE_UPDATE_UPSERT;
				upd[nu].id = repo.blob_create_from_buffer(f.data.value());
			}

			upd[nu].filemode = GIT_FILEMODE_BLOB;
			upd[nu].path = f.filename.c_str();
			nu++;
		}

		if (nu == 0)
			return;

		oid = repo.tree_create_updated(empty_tree, nu, upd.data());
	}

	GitTree tree(repo, oid);

	auto commit_oid = repo.commit_create(sig, sig, description, tree);

	auto commit = repo.commit_lookup(&commit_oid);

	repo.branch_create(branch.empty() ? "master" : branch, commit.get(), true);
}

static void do_clear_all(const GitRepo& repo, const GitTree& tree, const string& prefix,
						 list<git_file>& files, unordered_set<string>& filenames) {
	auto c = tree.entrycount();

	for (size_t i = 0; i < c; i++) {
		GitTreeEntry gte(tree, i);
		string name = prefix + gte.name();

		if (gte.type() == GIT_OBJECT_TREE) {
			GitTree subtree(repo, gte);

			do_clear_all(repo, subtree, name + "/", files, filenames);
		} else {
			if (!filenames.contains(name)) {
				files.emplace_back(name, nullptr);
				filenames.emplace(name);
			}
		}
	}
}

git_reference* GitRepo::branch_lookup(const std::string& branch_name, git_branch_t branch_type) {
	git_reference* ref = nullptr; // FIXME - would be better with class wrapper for this

	auto ret = git_branch_lookup(&ref, repo.get(), branch_name.c_str(), branch_type);

	if (ret && ret != GIT_ENOTFOUND)
		throw git_exception(ret, "git_branch_lookup");

	return !ret ? ref : nullptr;
}

void GitRepo::branch_create(const string& branch_name, const git_commit* target, bool force) {
	git_reference* ref = nullptr;

	auto ret = git_branch_create(&ref, repo.get(), branch_name.c_str(), target, force ? 1 : 0);

	if (ret)
		throw git_exception(ret, "git_branch_create");

	if (ref)
		git_reference_free(ref);
}

void GitRepo::reference_create(const string& name, const git_oid& id, bool force, const string& log_message) {
	git_reference* ref = nullptr;

	auto ret = git_reference_create(&ref, repo.get(), name.c_str(), &id, force ? 1 : 0, log_message.c_str());

	if (ret)
		throw git_exception(ret, "git_reference_create");

	if (ref)
		git_reference_free(ref);
}

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, list<git_file>& files,
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
		vector<git_tree_update> upd(files.size());
		unsigned int nu = 0;

		for (const auto& f : files) {
			if (!f.data.has_value()) {
				git_tree_entry* out;

				if (!parent_tree.entry_bypath(&out, f.filename))
					continue;

				git_tree_entry_free(out);

				upd[nu].action = GIT_TREE_UPDATE_REMOVE;
			} else {
				upd[nu].action = GIT_TREE_UPDATE_UPSERT;
				upd[nu].id = repo.blob_create_from_buffer(f.data.value());
			}

			upd[nu].filemode = GIT_FILEMODE_BLOB;
			upd[nu].path = f.filename.c_str();
			nu++;
		}

		if (nu == 0)
			return;

		oid = repo.tree_create_updated(parent_tree, nu, upd.data());
	}

	GitTree tree(repo, oid);

	{
		GitDiff diff(repo, parent_tree, tree, nullptr);

		if (diff.num_deltas() == 0) // no changes - avoid doing empty commit
			return;
	}

	auto commit_oid = repo.commit_create(sig, sig, description, tree, parent.get());

	repo.reference_create(branch.empty() ? "refs/heads/master" : "refs/heads/" + branch,
						  commit_oid, true, "branch updated");
}

GitIndex::GitIndex(const GitRepo& repo) {
	unsigned int ret;

	if ((ret = git_repository_index(&index, repo.repo.get())))
		throw git_exception(ret, "git_repository_index");
}

GitIndex::~GitIndex() {
	git_index_free(index);
}

void GitIndex::write_tree(git_oid* oid) {
	unsigned int ret;

	if ((ret = git_index_write_tree(oid, index)))
		throw git_exception(ret, "git_index_write_tree");
}

void GitIndex::add_bypath(const string& fn) {
	unsigned int ret;

	if ((ret = git_index_add_bypath(index, fn.c_str())))
		throw git_exception(ret, "git_index_add_bypath");
}

void GitIndex::remove_bypath(const string& fn) {
	unsigned int ret;

	if ((ret = git_index_remove_bypath(index, fn.c_str())))
		throw git_exception(ret, "git_index_remove_bypath");
}

void GitIndex::clear() {
	unsigned int ret;

	if ((ret = git_index_clear(index)))
		throw git_exception(ret, "git_index_clear");
}

size_t GitTree::entrycount() const {
	return git_tree_entrycount(tree);
}

GitTreeEntry::GitTreeEntry(const GitTree& tree, size_t idx) {
	gte = git_tree_entry_byindex(tree, idx);

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
	unsigned int ret;

	if ((ret = git_revparse_single((git_object**)&tree, repo.repo.get(), rev.c_str())))
		throw git_exception(ret, "git_revparse_single");
}

GitBlob::GitBlob(const GitTree& tree, const string& path) {
	unsigned int ret;

	if ((ret = git_object_lookup_bypath(&obj, (git_object*)(git_tree*)tree, path.c_str(), GIT_OBJECT_BLOB)))
		throw git_exception(ret, "git_object_lookup_bypath");
}

GitBlob::~GitBlob() {
	git_object_free(obj);
}

GitBlob::operator string() const {
	return string((char*)git_blob_rawcontent((git_blob*)obj), git_blob_rawsize((git_blob*)obj));
}

void GitRepo::checkout_head(const git_checkout_options* opts) {
	unsigned int ret;

	if ((ret = git_checkout_head(repo.get(), opts)))
		throw git_exception(ret, "git_checkout_head");
}

bool GitRepo::branch_is_head(const std::string& name) {
	int ret;
	git_reference* ref;

	ret = git_reference_lookup(&ref, repo.get(), ("refs/heads/" + name).c_str());
	if (ret == GIT_ENOTFOUND)
		return false;
	else if (ret)
		throw git_exception(ret, "git_reference_lookup");

	ret = git_branch_is_head(ref);

	if (ret < 0) {
		auto exc = git_exception(ret, "git_branch_is_head");
		git_reference_free(ref);
		throw exc;
	}

	git_reference_free(ref);

	return ret;
}

bool GitRepo::is_bare() {
	return git_repository_is_bare(repo.get());
}
