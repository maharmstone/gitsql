#pragma once

#include <git2.h>
#include <string>
#include <time.h>
#include "mercurysql.h"

using namespace std;

class GitRepo;
class GitDiff;

class GitSignature {
	friend class GitRepo;

public:
	GitSignature(const string& user, const string& email);
	GitSignature::GitSignature(const string& user, const string& email, time_t dt, signed int offset);
	~GitSignature();

private:
	git_signature* sig = nullptr;

	operator git_signature*() const {
		return sig;
	}
};

class GitTree {
	friend class GitRepo;
	friend class GitDiff;

public:
	GitTree(const git_commit* commit);
	GitTree(const GitRepo& repo, const git_oid& oid);
	~GitTree();

private:
	git_tree* tree;

	operator git_tree*() const {
		return tree;
	}
};

class GitDiff {
public:
	GitDiff(const GitRepo& repo, const GitTree& old_tree, const GitTree& new_tree, const git_diff_options* opts = nullptr);
	~GitDiff();
	size_t num_deltas();

private:
	git_diff* diff;
};

class GitRepo {
	friend GitTree;
	friend GitDiff;

public:
	GitRepo(const string& dir);
	~GitRepo();
	void reference_name_to_id(git_oid* out, const string& name);
	void commit_lookup(git_commit** commit, const git_oid* oid);
	git_oid commit_create(const string& update_ref, const GitSignature& author, const GitSignature& committer, const string& message, const GitTree& tree, git_commit* parent);
	git_oid blob_create_frombuffer(const string& data);
	git_oid tree_create_updated(const GitTree& baseline, size_t nupdates, const git_tree_update* updates);

private:
	git_repository* repo = nullptr;

	operator git_repository*() const {
		return repo;
	}
};

typedef struct {
	string filename;
	nullable<string> data;
} git_file;

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, time_t dt, signed int offset, const vector<git_file>& files);
