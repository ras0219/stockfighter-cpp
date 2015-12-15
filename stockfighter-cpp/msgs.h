#pragma once

#include <cstdint>
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
    if (ok == ERROR) throw std::runtime_error(err);
    return &data;
  }
  T const* operator->() const {
    if (ok == ERROR) throw std::runtime_error(err);
    return &data;
  }
  T& operator*() {
    if (ok == ERROR) throw std::runtime_error(err);
    return data;
  }
  const T& operator*() const {
    if (ok == ERROR) throw std::runtime_error(err);
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

struct request_t {
  uint64_t price;
  uint64_t qty;
};

struct orderbook_t {
  std::vector<request_t> bids;
  std::vector<request_t> asks;

  std::string timestamp;
};
enum direction_t {
  BUY,
  SELL
};
enum order_type_t {
  LIMIT,
  MARKET,
  FOK,
  IOC
};
enum open_t {
  OPEN,
  CLOSED
};

const char* stringify(order_type_t t);

struct order_request_t {
  std::string account;
  std::string venue;
  std::string symbol;
  request_t req;
  direction_t direction;
  order_type_t type;
};

struct fill_t {
  request_t req;
  std::string timestamp;
};

struct order_status_t {
  uint64_t original_qty;
  uint64_t outstanding_qty;
  uint64_t filled_qty;

  std::vector<fill_t> fills;
  open_t open;

  direction_t direction;

  std::string timestamp;
};

struct order_response_t {
  uint64_t id;

  order_status_t status;
};

struct gm_start_level_t {
  std::string account;
  uint64_t instance;

  std::string instructions;
  std::string order_types;
  uint64_t seconds_per_day;

  std::vector<std::string> tickers;
  std::vector<std::string> venues;
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

std::ostream& operator<<(std::ostream& os, unit const&);
std::ostream& operator<<(std::ostream& os, symboldef_t const& msg);
std::ostream& operator<<(std::ostream& os, symbolslist_t const& msg);
std::ostream& operator<<(std::ostream& os, request_t const& req);
std::ostream& operator<<(std::ostream& os, orderbook_t const& msg);
std::ostream& operator<<(std::ostream& os, order_response_t const& msg);
std::ostream& operator<<(std::ostream& os, order_status_t const& msg);
std::ostream& operator<<(std::ostream& os, gm_start_level_t const& msg);
