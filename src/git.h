#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <git2.h>
#include <string>
#include <list>
#include <optional>
#include <filesystem>
#include <time.h>
#include <tdscpp.h>

class GitRepo;
class GitDiff;
class GitIndex;
class GitTreeEntry;

class git_commit_deleter {
public:
	typedef git_commit* pointer;

	void operator()(git_commit* commit) {
		git_commit_free(commit);
	}
};

using git_commit_ptr = std::unique_ptr<git_commit*, git_commit_deleter>;

class git_repository_deleter {
public:
	typedef git_repository* pointer;

	void operator()(git_repository* repo) {
		git_repository_free(repo);
	}
};

using git_repository_ptr = std::unique_ptr<git_repository*, git_repository_deleter>;

class git_signature_deleter {
public:
	typedef git_signature* pointer;

	void operator()(git_signature* sig) {
		git_signature_free(sig);
	}
};

using git_signature_ptr = std::unique_ptr<git_signature*, git_signature_deleter>;

class git_tree_deleter {
public:
	typedef git_tree* pointer;

	void operator()(git_tree* tree) {
		git_tree_free(tree);
	}
};

using git_tree_ptr = std::unique_ptr<git_tree*, git_tree_deleter>;

class GitSignature {
public:
	GitSignature(const std::string& user, const std::string& email, const std::optional<tds::datetimeoffset>& dto = std::nullopt);

	git_signature_ptr sig;
};

class GitTree {
public:
	GitTree(const git_commit* commit);
	GitTree(const GitRepo& repo, const git_oid& oid);
	GitTree(const GitRepo& repo, const std::string& rev);
	GitTree(const GitRepo& repo, const GitTreeEntry& gte);
	size_t entrycount() const;
	bool entry_bypath(git_tree_entry** out, const std::string& path);

	git_tree_ptr tree;
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
public:
	GitRepo(const std::string& dir);
	bool reference_name_to_id(git_oid* out, const std::string& name);
	git_commit_ptr commit_lookup(const git_oid* oid);
	git_oid commit_create(const GitSignature& author, const GitSignature& committer, const std::string& message, const GitTree& tree,
						  git_commit* parent = nullptr);
	git_oid blob_create_from_buffer(std::string_view data);
	git_oid tree_create_updated(const GitTree& baseline, size_t nupdates, const git_tree_update* updates);
	git_oid index_tree_id() const;
	void checkout_head(const git_checkout_options* opts = nullptr);
	git_reference* branch_lookup(const std::string& branch_name, git_branch_t branch_type);
	void branch_create(const std::string& branch_name, const git_commit* target, bool force);
	void reference_create(const std::string& name, const git_oid& id, bool force, const std::string& log_message);
	bool branch_is_head(const std::string& name);
	bool is_bare();

	git_repository_ptr repo;
};

class GitIndex {
public:
	GitIndex(const GitRepo& repo);
	~GitIndex();
	void write_tree(git_oid* oid);
	void add_bypath(const std::string& fn);
	void remove_bypath(const std::string& fn);
	void clear();

private:
	git_index* index = nullptr;
};

class GitTreeEntry {
public:
	GitTreeEntry(const GitTree& tree, size_t idx);
	std::string name();
	git_object_t type();

	friend class GitTree;

private:
	const git_tree_entry* gte;

	operator const git_tree_entry*() const {
		return gte;
	}
};

class GitBlob {
public:
	GitBlob(const GitTree& tree, const std::string& path);
	~GitBlob();
	operator std::string() const;

private:
	git_object* obj;
};

struct git_file {
	git_file(const std::string& filename, const tds::value& data) : filename(filename) {
		if (!data.is_null) {
			this->data = "";
			this->data.value().resize(data.val.size());
			memcpy(this->data.value().data(), data.val.data(), data.val.size());
		}
	}

	std::string filename;
	std::optional<std::string> data;
};

#ifdef _WIN32

class handle_closer {
public:
	typedef HANDLE pointer;

	void operator()(HANDLE h) {
		if (h == INVALID_HANDLE_VALUE)
			return;

		CloseHandle(h);
	}
};

typedef std::unique_ptr<HANDLE, handle_closer> unique_handle;

#else

class unique_handle {
public:
	unique_handle() : fd(0) {
	}

	explicit unique_handle(int fd) : fd(fd) {
	}

	unique_handle(unique_handle&& that) noexcept {
		fd = that.fd;
		that.fd = 0;
	}

	unique_handle(const unique_handle&) = delete;
	unique_handle& operator=(const unique_handle&) = delete;

	unique_handle& operator=(unique_handle&& that) noexcept {
		if (fd > 0)
			close(fd);

		fd = that.fd;
		that.fd = 0;

		return *this;
	}

	~unique_handle() {
		if (fd <= 0)
			return;

		close(fd);
	}

	explicit operator bool() const noexcept {
		return fd != 0;
	}

	void reset(int new_fd = 0) noexcept {
		if (fd > 0)
			close(fd);

		fd = new_fd;
	}

	int get() const noexcept {
		return fd;
	}

private:
	int fd;
};

#endif

void update_git(GitRepo& repo, const std::string& user, const std::string& email, const std::string& description,
				std::list<git_file>& files, bool clear_all = false, const std::optional<tds::datetimeoffset>& dto = std::nullopt,
				const std::string& branch = "");
