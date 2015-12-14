#include "msgs.h"
#include <cstdlib>

using namespace std;

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
  for (auto&& sym : msg.symbols) {
    os << sym << "\n";
  }
  os << "}";
  return os;
}

std::ostream& operator<<(std::ostream& os, unit const&) { return os << "unit{}"; }
