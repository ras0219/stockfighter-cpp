#include <fstream>
#include <iostream>
#include <cstdlib>
#include <thread>
#include "msgs.h"
#include "sf_client.h"

using namespace web::http;
using namespace client;
using namespace std;
using namespace literals;

void fatal(const char* msg) {
  cerr << msg << "\n";
  abort();
}

int main(int argc, const char** argv) {
  if (argc < 2) fatal("usage: main <apikeyfile>");
  ifstream f(argv[1]);
  if (!f) fatal("error: ifstream()");
  string apikey;
  if (!getline(f, apikey)) fatal("error: getline()");

  cout << "API Key: '" << apikey << "'\n";

  sf_client client(apikey);

  string level = "chock_a_block";

  auto slevel = client.start_level(level);
  if (slevel->tickers.size() != 1) fatal("error: tickers");
  if (slevel->venues.size() != 1) fatal("error: venues");

  cout << "=== level start ===" << endl;

  this_thread::sleep_for(3s);
  
  string account = slevel->account;
  string venue = slevel->venues.front();
  string symbol = slevel->tickers.front();

  vector<uint64_t> outstanding_orders;

  auto obook = client.orderbook_for_venue(venue, symbol);

  vector<request_t> asks = move(obook->asks);
  vector<request_t> bids = move(obook->bids);

  uint64_t best_ask = 0;
  uint64_t best_bid = 0;

  uint64_t default_spread = 75;
  
  if (asks.empty() && bids.empty()) {
    // do nothing
  } else if (asks.empty()) {
    best_ask = asks.front().price;
    best_bid = best_ask - default_spread;
  } else if (bids.empty()) {
    best_bid = bids.front().price;
    best_ask = best_bid + default_spread;
  } else {
    best_ask = asks.front().price;
    best_bid = bids.front().price;
  }

  cout << "best ask: " << best_ask << endl;
  cout << "best bid: " << best_bid << endl;


  
  

  // order_request_t ort;
  //   ort.account = account;
  //   ort.venue = venue;
  //   ort.symbol = symbol;
  //   ort.req = obook->asks.front();
  //   ort.direction = BUY;
  //   ort.type = IOC;

  //   if (ort.req.qty > shares_to_buy)
  //     ort.req.qty = shares_to_buy;
  //   try {
  //     auto res = client.post_order(ort);
  //     cout << "order: " << res << endl;

  //     shares_to_buy -= res->filled_qty;
  //   } catch (exception& e) {
  //     cerr << "warning: " << e.what() << "\n";
  //   } catch (...) {
  //     cerr << "warning: unknown error\n";
  //   }

  //   this_thread::sleep_for(1s);
  // }
  return 0;
}
