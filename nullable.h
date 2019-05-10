#pragma once

#include <string>

template<class T>
class nullable_base {
public:
	nullable_base(const T& s) {
		t = s;
		null = false;
	}

	nullable_base(nullptr_t) {
		null = true;
	}

	nullable_base() {
		null = true;
	}

	operator T() const {
		if (null)
			return T();
		else
			return t;
	}

	bool is_null() const {
		return null;
	}

protected:
	T t;
	bool null = false;
};

template<class T>
class nullable : public nullable_base<T> {
};

class binary_string {
public:
	binary_string(std::string s) {
		this->s = s;
	}

	std::string s;
};

class TDSField;

template<>
class nullable<std::string> : public nullable_base<std::string> {
public:
	nullable(const std::string& s) : nullable_base(s) { }

	nullable(nullptr_t) : nullable_base(nullptr) { }

	nullable() : nullable_base() { }

	nullable(const TDSField& f);
};

template<>
class nullable<unsigned long long> : public nullable_base<unsigned long long> {
public:
	nullable(const unsigned long long& s) : nullable_base(s) { }

	nullable(nullptr_t) : nullable_base(nullptr) { }

	nullable() : nullable_base() { }

	nullable(const TDSField& f);
};
