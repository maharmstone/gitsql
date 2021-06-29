#include <git2.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "git.h"

using namespace std;

#ifdef _MSC_VER
#define noret __declspec(noreturn)
#else
#define noret __attribute__((noreturn))
#endif

noret
static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw runtime_error(string(func) + " failed (" + lg2err->message + ").");

	throw runtime_error(string(func) + " failed (error " + to_string(error) + ").");
}

static const auto jan1970 = chrono::duration_cast<chrono::seconds>(chrono::time_point{chrono::sys_days{1970y/chrono::January/1d}}.time_since_epoch());

GitSignature::GitSignature(const string& user, const string& email, const optional<tds::datetimeoffset>& dto) {
	unsigned int ret;

	if (dto.has_value()) {
		auto tp = (chrono::time_point<chrono::system_clock>)dto.value() + chrono::minutes{dto.value().offset};
		auto secs = chrono::duration_cast<chrono::seconds>(tp.time_since_epoch()) - jan1970;

		if ((ret = git_signature_new(&sig, user.c_str(), email.c_str(), secs.count(), dto.value().offset)))
			throw_git_error(ret, "git_signature_new");
	} else {
		if ((ret = git_signature_now(&sig, user.c_str(), email.c_str())))
			throw_git_error(ret, "git_signature_now");
	}
}

GitSignature::~GitSignature() {
	git_signature_free(sig);
}

GitTree::GitTree(const git_commit* commit) {
	unsigned int ret;

	if ((ret = git_commit_tree(&tree, commit)))
		throw_git_error(ret, "git_commit_tree");
}

GitTree::GitTree(const GitRepo& repo, const git_oid& oid) {
	unsigned int ret;

	if ((ret = git_tree_lookup(&tree, repo, &oid)))
		throw_git_error(ret, "git_tree_lookup");
}

GitTree::GitTree(const GitRepo& repo, const GitTreeEntry& gte) {
	unsigned int ret;

	if ((ret = git_tree_entry_to_object((git_object**)&tree, repo, gte)))
		throw_git_error(ret, "git_tree_entry_to_object");
}

GitTree::~GitTree() {
	git_tree_free(tree);
}

bool GitTree::entry_bypath(git_tree_entry** out, const string& path) {
	int ret = git_tree_entry_bypath(out, tree, path.c_str());

	if (ret != 0 && ret != GIT_ENOTFOUND)
		throw_git_error(ret, "git_tree_entry_bypath");

	return ret == 0;
}

GitRepo::GitRepo(const string& dir) {
	unsigned int ret;

	if ((ret = git_repository_open(&repo, dir.c_str())))
		throw_git_error(ret, "git_repository_open");
}

GitRepo::~GitRepo() {
	git_repository_free(repo);
}

bool GitRepo::reference_name_to_id(git_oid* out, const string& name) {
	auto ret = git_reference_name_to_id(out, repo, name.c_str());

	if (ret == 0)
		return true;
	else if (ret == GIT_ENOTFOUND)
		return false;
	else
		throw_git_error(ret, "git_reference_name_to_id");
}

void GitRepo::commit_lookup(git_commit** commit, const git_oid* oid) {
	unsigned int ret;

	if ((ret = git_commit_lookup(commit, repo, oid)))
		throw_git_error(ret, "git_commit_lookup");
}

git_oid GitRepo::commit_create(const GitSignature& author, const GitSignature& committer, const string& message, const GitTree& tree,
							   git_commit* parent) {
	unsigned int ret;
	git_oid id;

	if (parent) {
		if ((ret = git_commit_create_v(&id, repo, nullptr, author, committer, nullptr, message.c_str(), tree, 1, parent)))
			throw_git_error(ret, "git_commit_create_v");
	} else {
		if ((ret = git_commit_create_v(&id, repo, nullptr, author, committer, nullptr, message.c_str(), tree, 0)))
			throw_git_error(ret, "git_commit_create_v");
	}

	return id;
}

git_oid GitRepo::blob_create_frombuffer(const string_view& data) {
	unsigned int ret;
	git_oid blob;

	if ((ret = git_blob_create_frombuffer(&blob, repo, data.data(), data.length())))
		throw_git_error(ret, "git_blob_create_frombuffer");

	return blob;
}

git_oid GitRepo::tree_create_updated(const GitTree& baseline, size_t nupdates, const git_tree_update* updates) {
	unsigned int ret;
	git_oid oid;

	if ((ret = git_tree_create_updated(&oid, repo, baseline, nupdates, updates)))
		throw_git_error(ret, "git_tree_create_updated");

	return oid;
}

GitDiff::GitDiff(const GitRepo& repo, const GitTree& old_tree, const GitTree& new_tree, const git_diff_options* opts) {
	unsigned int ret;

	if ((ret = git_diff_tree_to_tree(&diff, repo, old_tree, new_tree, opts)))
		throw_git_error(ret, "git_diff_tree_to_tree");
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

	if ((ret = git_repository_index(&index, repo)))
		throw_git_error(ret, "git_repository_index");

	try {
		if ((ret = git_index_write_tree(&tree_id, index)))
			throw_git_error(ret, "git_index_write_tree");
	} catch (...) {
		git_index_free(index);
		throw;
	}

	git_index_free(index);

	return tree_id;
}

static void update_git_no_parent(GitRepo& repo, const GitSignature& sig, const string& description, const list<git_file>& files, const string& branch) {
	git_oid oid;

	GitTree empty_tree(repo, repo.index_tree_id());

	{
		git_tree_update* upd = new git_tree_update[files.size()];
		unsigned int nu = 0;

		for (const auto& f : files) {
			if (f.data.has_value()) {
				upd[nu].action = GIT_TREE_UPDATE_UPSERT;
				upd[nu].id = repo.blob_create_frombuffer(f.data.value());
			}

			upd[nu].filemode = GIT_FILEMODE_BLOB;
			upd[nu].path = f.filename.c_str();
			nu++;
		}

		if (nu == 0) {
			delete[] upd;
			return;
		}

		oid = repo.tree_create_updated(empty_tree, nu, upd);

		delete[] upd;
	}

	GitTree tree(repo, oid);

	auto commit_oid = repo.commit_create(sig, sig, description, tree);

	git_commit* commit; // FIXME - class wrapper for this

	repo.commit_lookup(&commit, &commit_oid);

	repo.branch_create(branch.c_str(), commit, true);

	git_commit_free(commit);
}

static void do_clear_all(const GitRepo& repo, const GitTree& tree, const string& prefix, list<git_file>& files) {
	auto c = tree.entrycount();

	for (size_t i = 0; i < c; i++) {
		GitTreeEntry gte(tree, i);
		string name = gte.name();

		if (gte.type() == GIT_OBJ_TREE) {
			GitTree subtree(repo, gte);

			do_clear_all(repo, subtree, prefix + name + "/", files);
		} else {
			bool found = false;

			for (auto& f : files) {
				if (f.filename == prefix + name) {
					found = true;
					break;
				}
			}

			if (!found)
				files.emplace_back(prefix + name, nullptr);
		}
	}
}

git_reference* GitRepo::branch_lookup(const std::string& branch_name, git_branch_t branch_type) {
	git_reference* ref = nullptr; // FIXME - would be better with class wrapper for this

	auto ret = git_branch_lookup(&ref, repo, branch_name.c_str(), branch_type);

	if (ret && ret != GIT_ENOTFOUND)
		throw_git_error(ret, "git_branch_lookup");

	return !ret ? ref : nullptr;
}

void GitRepo::branch_create(const string& branch_name, const git_commit* target, bool force) {
	git_reference* ref = nullptr;

	auto ret = git_branch_create(&ref, repo, branch_name.c_str(), target, force ? 1 : 0);

	if (ret)
		throw_git_error(ret, "git_branch_create");

	if (ref)
		git_reference_free(ref);
}

void GitRepo::reference_create(const string& name, const git_oid& id, bool force, const string& log_message) {
	git_reference* ref = nullptr;

	auto ret = git_reference_create(&ref, repo, name.c_str(), &id, force ? 1 : 0, log_message.c_str());

	if (ret)
		throw_git_error(ret, "git_reference_create");

	if (ref)
		git_reference_free(ref);
}

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, list<git_file>& files,
				bool clear_all, const optional<tds::datetimeoffset>& dto, const string& branch) {
	GitSignature sig(user, email, dto);

	git_commit* parent;
	git_oid parent_id;

	bool parent_found = repo.reference_name_to_id(&parent_id, branch.empty() ? "refs/heads/master" : "refs/heads/" + branch);

	if (!parent_found) {
		update_git_no_parent(repo, sig, description, files, branch);
		return;
	}

	repo.commit_lookup(&parent, &parent_id);

	GitTree parent_tree(parent);
	git_oid oid;

	if (clear_all)
		do_clear_all(repo, parent_tree, "", files);

	{
		git_tree_update* upd = new git_tree_update[files.size()];
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
				upd[nu].id = repo.blob_create_frombuffer(f.data.value());
			}

			upd[nu].filemode = GIT_FILEMODE_BLOB;
			upd[nu].path = f.filename.c_str();
			nu++;
		}

		if (nu == 0) {
			delete[] upd;
			return;
		}

		oid = repo.tree_create_updated(parent_tree, nu, upd);

		delete[] upd;
	}

	GitTree tree(repo, oid);

	{
		GitDiff diff(repo, parent_tree, tree, nullptr);

		if (diff.num_deltas() == 0) // no changes - avoid doing empty commit
			return;
	}

	auto commit_oid = repo.commit_create(sig, sig, description, tree, parent);
	git_commit* commit;

	repo.commit_lookup(&commit, &commit_oid);

	repo.reference_create(branch.empty() ? "refs/heads/master" : "refs/heads/" + branch,
						  commit_oid, true, "branch updated");
}

GitIndex::GitIndex(const GitRepo& repo) {
	unsigned int ret;

	if ((ret = git_repository_index(&index, repo)))
		throw_git_error(ret, "git_repository_index");
}

GitIndex::~GitIndex() {
	git_index_free(index);
}

void GitIndex::write_tree(git_oid* oid) {
	unsigned int ret;

	if ((ret = git_index_write_tree(oid, index)))
		throw_git_error(ret, "git_index_write_tree");
}

void GitIndex::add_bypath(const string& fn) {
	unsigned int ret;

	if ((ret = git_index_add_bypath(index, fn.c_str())))
		throw_git_error(ret, "git_index_add_bypath");
}

void GitIndex::remove_bypath(const string& fn) {
	unsigned int ret;

	if ((ret = git_index_remove_bypath(index, fn.c_str())))
		throw_git_error(ret, "git_index_remove_bypath");
}

void GitIndex::clear() {
	unsigned int ret;

	if ((ret = git_index_clear(index)))
		throw_git_error(ret, "git_index_clear");
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

git_otype GitTreeEntry::type() {
	return git_tree_entry_type(gte);
}

GitTree::GitTree(const GitRepo& repo, const string& rev) {
	unsigned int ret;

	if ((ret = git_revparse_single((git_object**)&tree, repo, rev.c_str())))
		throw_git_error(ret, "git_revparse_single");
}

GitBlob::GitBlob(const GitTree& tree, const string& path) {
	unsigned int ret;

	if ((ret = git_object_lookup_bypath(&obj, (git_object*)(git_tree*)tree, path.c_str(), GIT_OBJ_BLOB)))
		throw_git_error(ret, "git_object_lookup_bypath");
}

GitBlob::~GitBlob() {
	git_object_free(obj);
}

GitBlob::operator string() const {
	return string((char*)git_blob_rawcontent((git_blob*)obj), git_blob_rawsize((git_blob*)obj));
}

void GitRepo::checkout_head(const git_checkout_options* opts) {
	unsigned int ret;

	if ((ret = git_checkout_head(repo, opts)))
		throw_git_error(ret, "git_checkout_head");
}

bool GitRepo::branch_is_head(const std::string& name) {
	int ret;
	git_reference* ref;

	ret = git_reference_lookup(&ref, repo, ("refs/heads/" + name).c_str());
	if (ret)
		throw_git_error(ret, "git_reference_lookup");

	ret = git_branch_is_head(ref);

	if (ret < 0) {
		git_reference_free(ref); // FIXME - will this overwrite error?
		throw_git_error(ret, "git_branch_is_head");
	}

	git_reference_free(ref);

	return ret;
}

bool GitRepo::is_bare() {
	return git_repository_is_bare(repo);
}
