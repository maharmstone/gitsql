#pragma once

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
