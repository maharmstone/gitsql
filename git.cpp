#include <git2.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "git.h"

using namespace std;

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw runtime_error(string(func) + " failed (" + lg2err->message + ").");

	throw runtime_error(string(func) + " failed (error " + to_string(error) + ").");
}

GitSignature::GitSignature(const string& user, const string& email) {
	unsigned int ret;

	if ((ret = git_signature_now(&sig, user.c_str(), email.c_str())))
		throw_git_error(ret, "git_signature_now");
}

GitSignature::GitSignature(const string& user, const string& email, time_t dt, signed int offset) {
	unsigned int ret;

	if ((ret = git_signature_new(&sig, user.c_str(), email.c_str(), dt, offset)))
		throw_git_error(ret, "git_signature_new");
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

void GitRepo::reference_name_to_id(git_oid* out, const string& name) {
	unsigned int ret;

	if ((ret = git_reference_name_to_id(out, repo, name.c_str())))
		throw_git_error(ret, "git_reference_name_to_id");
}

void GitRepo::commit_lookup(git_commit** commit, const git_oid* oid) {
	unsigned int ret;

	if ((ret = git_commit_lookup(commit, repo, oid)))
		throw_git_error(ret, "git_commit_lookup");
}

git_oid GitRepo::commit_create(const string& update_ref, const GitSignature& author, const GitSignature& committer, const string& message, const GitTree& tree, git_commit* parent) {
	unsigned int ret;
	git_oid id;

	if ((ret = git_commit_create_v(&id, repo, update_ref.c_str(), author, committer, nullptr, message.c_str(), tree, 1, parent)))
		throw_git_error(ret, "git_commit_create_v");

	return id;
}

git_oid GitRepo::blob_create_frombuffer(const string& data) {
	unsigned int ret;
	git_oid blob;

	if ((ret = git_blob_create_frombuffer(&blob, repo, data.c_str(), data.length())))
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

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, time_t dt, signed int offset, const list<git_file>& files) {
	GitSignature sig(user, email, dt, offset);

	git_commit* parent;
	git_oid parent_id;

	repo.reference_name_to_id(&parent_id, "HEAD");

	repo.commit_lookup(&parent, &parent_id);

	GitTree parent_tree(parent);
	git_oid oid;

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

	repo.commit_create("HEAD", sig, sig, description, tree, parent);
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

size_t GitTree::entrycount() {
	return git_tree_entrycount(tree);
}

GitTreeEntry::GitTreeEntry(GitTree& tree, size_t idx) {
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
