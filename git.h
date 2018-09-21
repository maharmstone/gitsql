#pragma once

#include <git2.h>
#include <string>
#include <time.h>
#include "mercurysql.h"

using namespace std;

class GitRepo;
class GitDiff;
class GitIndex;
class GitTreeEntry;

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
	friend class GitTreeEntry;

public:
	GitTree(const git_commit* commit);
	GitTree(const GitRepo& repo, const git_oid& oid);
	GitTree(const GitRepo& repo, const GitTreeEntry& gte);
	~GitTree();
	size_t entrycount();

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
	friend GitIndex;

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

class GitIndex {
public:
	GitIndex(const GitRepo& repo);
	~GitIndex();
	void write_tree(git_oid* oid);
	void add_bypath(const string& fn);
	void remove_bypath(const string& fn);
	void clear();

private:
	git_index* index = nullptr;
};

class GitTreeEntry {
public:
	GitTreeEntry(GitTree& tree, size_t idx);
	string name();
	git_otype type();

	friend class GitTree;

private:
	const git_tree_entry* gte;

	operator const git_tree_entry*() const {
		return gte;
	}
};

typedef struct {
	string filename;
	nullable<string> data;
} git_file;

void update_git(GitRepo& repo, const string& user, const string& email, const string& description, time_t dt, signed int offset, const vector<git_file>& files);
