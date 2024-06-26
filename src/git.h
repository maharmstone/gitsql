#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <git2.h>
#include <string>
#include <list>
#include <optional>
#include <filesystem>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <time.h>
#include <tdscpp.h>

class GitRepo;
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

class git_index_deleter {
public:
	typedef git_index* pointer;

	void operator()(git_index* index) {
		git_index_free(index);
	}
};

using git_index_ptr = std::unique_ptr<git_index*, git_index_deleter>;

class git_reference_deleter {
public:
	typedef git_reference* pointer;

	void operator()(git_reference* ref) {
		git_reference_free(ref);
	}
};

using git_reference_ptr = std::unique_ptr<git_reference*, git_reference_deleter>;

class git_object_deleter {
public:
	typedef git_object* pointer;

	void operator()(git_object* obj) {
		git_object_free(obj);
	}
};

using git_object_ptr = std::unique_ptr<git_object*, git_object_deleter>;

class git_tree_entry_deleter {
public:
	typedef git_tree_entry* pointer;

	void operator()(git_tree_entry* gte) {
		git_tree_entry_free(gte);
	}
};

using git_tree_entry_ptr = std::unique_ptr<git_tree_entry*, git_tree_entry_deleter>;

class git_remote_deleter {
public:
	using pointer = git_remote*;

	void operator()(git_remote* r) {
		git_remote_free(r);
	}
};

using git_remote_ptr = std::unique_ptr<git_remote*, git_remote_deleter>;

class git_odb_deleter {
public:
	using pointer = git_odb*;

	void operator()(git_odb* o) {
		git_odb_free(o);
	}
};

using git_odb_ptr = std::unique_ptr<git_odb*, git_odb_deleter>;

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
	git_tree_entry_ptr entry_bypath(const std::string& path);

	git_tree_ptr tree;
};

class GitRemote {
public:
	GitRemote(git_remote* r) : r(r) { }
	void push(const std::vector<std::string>& refspecs, const git_push_options& opts);

	git_remote_ptr r;
};

class GitRepo {
public:
	GitRepo(const std::string& dir);
	bool reference_name_to_id(git_oid* out, const std::string& name);
	git_commit_ptr commit_lookup(const git_oid* oid);
	git_oid commit_create(const GitSignature& author, const GitSignature& committer, const std::string& message, const GitTree& tree,
						  git_commit* parent = nullptr);
	git_oid blob_create_from_buffer(std::span<const uint8_t> data);
	git_oid tree_create_updated(const GitTree& baseline, std::span<const git_tree_update> updates);
	git_oid index_tree_id() const;
	void checkout_head(const git_checkout_options* opts = nullptr);
	git_reference_ptr branch_lookup(const std::string& branch_name, git_branch_t branch_type);
	void branch_create(const std::string& branch_name, const git_commit* target, bool force);
	void reference_create(const std::string& name, const git_oid& id, bool force, const std::string& log_message);
	bool branch_is_head(const std::string& name);
	bool is_bare();
	std::string branch_upstream_remote(const std::string& refname);
	GitRemote remote_lookup(const std::string& name);
	void try_push(const std::string& ref);

	git_oid blob_create_from_buffer(std::string_view data) {
		return blob_create_from_buffer(std::span((uint8_t*)data.data(), data.size()));
	}

	git_repository_ptr repo;
};

class GitIndex {
public:
	GitIndex(const GitRepo& repo);
	void write_tree(git_oid* oid);
	void add_bypath(const std::string& fn);
	void remove_bypath(const std::string& fn);
	void clear();

	git_index_ptr index;
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
	operator std::string() const;

	git_object_ptr obj;
};

struct git_file {
	git_file(std::string_view filename, const tds::value& data) : filename(filename) {
		if (!data.is_null) {
			this->data = "";
			this->data.value().resize(data.val.size());
			memcpy(this->data.value().data(), data.val.data(), data.val.size());
		}
	}

	std::string filename;
	std::optional<std::string> data;
};

struct git_file2 {
	git_file2(std::string_view filename, const std::optional<git_oid>& oid) :
		filename(filename), oid(oid) {
	}

	std::string filename;
	std::optional<git_oid> oid;
};

struct git_update {
	git_update(GitRepo& repo) : repo(repo) { }

	void add_file(std::string_view filename, std::string_view data);
	void run(std::stop_token st) noexcept;
	void start();
	void stop();

	GitRepo& repo;
	std::mutex lock;
	std::condition_variable_any cv;
	std::list<git_file> files;
	std::list<git_file2> files2;
	std::exception_ptr teptr;
	std::jthread t;
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
				std::list<git_file2>& files, bool clear_all = false, const std::optional<tds::datetimeoffset>& dto = std::nullopt,
				const std::string& branch = "");
