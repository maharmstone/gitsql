#include <git2.h>
#include <string>
#include "git.h"

using namespace std;

static void throw_git_error(int error, const char* func) {
	const git_error *lg2err;

	if ((lg2err = giterr_last()) && lg2err->message)
		throw runtime_error(string(func) + " failed (" + lg2err->message + ").");

	throw runtime_error(string(func) + " failed.");
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

GitTree::~GitTree() {
	git_tree_free(tree);
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

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, time_t dt, signed int offset, const vector<git_file>& files) {
	GitSignature sig(user, email, dt, offset);

	git_commit* parent;
	git_oid parent_id;

	repo.reference_name_to_id(&parent_id, "HEAD");

	repo.commit_lookup(&parent, &parent_id);

	GitTree parent_tree(parent);
	git_oid oid;

	{
		git_tree_update* upd = new git_tree_update[files.size()];

		for (unsigned int i = 0; i < files.size(); i++) {
			if (files[i].data.is_null())
				upd[i].action = GIT_TREE_UPDATE_REMOVE;
			else {
				upd[i].action = GIT_TREE_UPDATE_UPSERT;
				upd[i].id = repo.blob_create_frombuffer(files[i].data);
			}

			upd[i].filemode = GIT_FILEMODE_BLOB;
			upd[i].path = files[i].filename.c_str();
		}

		oid = repo.tree_create_updated(parent_tree, files.size(), upd);

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