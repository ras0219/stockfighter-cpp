#include "msgs.h"
#include <cstdlib>
#include <iomanip>

using namespace std;

template<class T>
void serialize(ostream& os, const vector<T>& v) {
  for (auto&& t : v) {
    os << t << "\n";
  }
}

const char* stringify(order_type_t t) {
  switch (t) {
  case LIMIT: return "limit";
  case MARKET: return "market";
  case FOK: return "fill-or-kill";
  case IOC: return "immediate-or-cancel";
  default:
    abort();
  }
}

ostream& operator<<(ostream& os, ok_t const& ok) {
  switch (ok) {
  case OK: return os << "OK";
  case ERROR: return os << "ERR";
  default: abort();
  }
}

std::ostream& operator<<(std::ostream& os, symboldef_t const& msg) {
  os << "symboldef_t{\"" << msg.symbol << "\", \"" << msg.name << "\"}";
  return os;
}
std::ostream& operator<<(std::ostream& os, symbolslist_t const& msg) {
  os << "symbolslist_t{\n";
  serialize(os, msg.symbols);
  os << "}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const unit&) { return os << "unit{}"; }

std::ostream& operator<<(std::ostream& os, request_t const& req) {
  return os << "request_t{p" << req.price << ",q" << req.qty << "}";
}

std::ostream& operator<<(std::ostream& os, const orderbook_t& msg) {
  os << "orderbook_t{\n";
  os << "bids{\n";
  serialize(os, msg.bids);
  os << "}, asks{\n";
  serialize(os, msg.asks);
  os << "}, timestamp{" << msg.timestamp << "}\n}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const order_status_t& msg) {
  os << "order_status_t{original_qty{"
     << msg.original_qty << "}, outstanding_qty{"
     << msg.outstanding_qty << "}, filled_qty{"
     << msg.filled_qty << "}, timestamp{"
     << msg.timestamp << "}}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const order_response_t& msg) {
  os << "order_response_t{id{"
     << msg.id << "},"
     << msg.status
     << "}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const gm_start_level_t& msg) {
  os << "gm_start_level_t{\n"
     << "account{" << msg.account << "},\n"
     << "instance{" << msg.instance << "},\n"
     << "seconds{" << msg.seconds_per_day << "},\n"
     << "tickers{";
  serialize(os, msg.tickers);
  os << "},\n"
     << "venues{";
  serialize(os, msg.venues);
  os << "}}";
}
