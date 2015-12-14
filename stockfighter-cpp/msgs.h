#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

enum ok_t {
  OK,
  ERROR
};

struct unit {};

template<class T>
struct optional {
  ok_t ok;
  std::string err;

  T* operator->() {
    if (ok == ERROR) throw std::runtime_error("dereference bad object");
    return data;
  }
  T const* operator->() const {
    if (ok == ERROR) throw std::runtime_error("dereference bad object");
    return data;
  }

  T data;
};

struct symboldef_t {
  std::string symbol;
  std::string name;
};

struct symbolslist_t {
  std::vector<symboldef_t> symbols;
};

std::ostream& operator<<(std::ostream& os, ok_t const& ok);

template<class T>
std::ostream& operator<<(std::ostream& os, optional<T> const& obj) {
  switch (obj.ok) {
  case OK: return os << obj.data;
  case ERROR: return os << "ERR{\"" << obj.err << "\"}";
  default: std::abort();
  }
}

std::ostream& operator<<(std::ostream& os, symboldef_t const& msg);
std::ostream& operator<<(std::ostream& os, symbolslist_t const& msg);
std::ostream& operator<<(std::ostream& os, unit const&);
