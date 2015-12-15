#include <iostream>
#include <cstdlib>
#include <thread>
#include "msgs.h"
#include "sf_client.h"
#include "util.h"

using namespace web::http;
using namespace client;
using namespace std;
using namespace literals;

void fatal(const char* msg) {
  cerr << msg << "\n";
  abort();
}

int main(int argc, const char** argv) {
  if (argc < 2) FATAL_ERROR("usage: main <apikeyfile>");
  string apikey = read_api_key_from_file(argv[1]);

  cout << "API Key: '" << apikey << "'\n";

  sf_client client(apikey);

  client.tracing = true;

  auto slevel = [&](){
    if (argc >= 3) {
      // instance id provided
      return client.restart_level(atoi(argv[2]));
    } else {
      return client.start_level("sell_side");
    }
  }();
  if (slevel->tickers.size() != 1) FATAL_ERROR("tickers");
  if (slevel->venues.size() != 1) FATAL_ERROR("venues");

  cout << "=== level start: "<<slevel<<" ===" << endl;

  this_thread::sleep_for(3s);
  
  string account = slevel->account;
  string venue = slevel->venues.front();
  string symbol = slevel->tickers.front();

  int64_t max_position = 1000;
  uint64_t total_buys = 0;
  uint64_t total_sells = 0;
  
  uint64_t total_cash_sent = 0;
  uint64_t total_cash_recv = 0;

  vector<order_response_t> outstanding_orders;

  while (1) {

    for (auto&& o : outstanding_orders) {
      auto canc = client.cancel_order(venue, symbol, o.id);

      if (canc->status.direction == BUY) {
        total_buys += canc->status.filled_qty;
        for (auto&& fill : canc->status.fills)
          total_cash_sent += fill.req.price * fill.req.qty;
      }
      if (canc->status.direction == SELL) {
        total_sells += canc->status.filled_qty;
        for (auto&& fill : canc->status.fills)
          total_cash_recv += fill.req.price * fill.req.qty;
      }
    }
    outstanding_orders.clear();

    cout << "trades to date: B:" << total_buys << " S:" << total_sells << endl;
    cout << "cash:           B:" << total_cash_sent << " S:" << total_cash_recv << endl;
    
    auto obook = client.orderbook_for_venue(venue, symbol);
    cout << "obook: " << obook << endl;
    vector<request_t> asks = move(obook->asks);
    vector<request_t> bids = move(obook->bids);

    uint64_t best_ask = 0;
    uint64_t best_bid = 0;

    uint64_t default_spread = 100;
  
    if (asks.empty() && bids.empty()) {
      // sleep and try again
      cout << "no offers. sleeping." << endl;
      this_thread::sleep_for(1s);
      continue;
    } else if (bids.empty()) {
      best_ask = asks.front().price;
      best_bid = best_ask - default_spread;
    } else if (asks.empty()) {
      best_bid = bids.front().price;
      best_ask = best_bid + default_spread;
    } else {
      best_ask = asks.front().price;
      best_bid = bids.front().price;
    }

    cout << "best ask: " << best_ask << endl;
    cout << "best bid: " << best_bid << endl;

    int64_t cur_position = int64_t(total_buys) - int64_t(total_sells);

    try {
      order_request_t ort;
      ort.account = account;
      ort.venue = venue;
      ort.symbol = symbol;
      ort.req.price = best_bid + 5;
      ort.req.qty = (max_position - cur_position)/2;
      ort.direction = BUY;
      ort.type = LIMIT;

      cout << "PLACING ORDER." << endl;
      auto res = client.post_order(ort);
      cout << "RESP: " << res << endl;
      outstanding_orders.push_back(*res);
    } catch (exception& e) { cerr << "error: " << e.what() << endl; }

    try {
      order_request_t ort;
      ort.account = account;
      ort.venue = venue;
      ort.symbol = symbol;
      ort.req.price = best_ask - 5;
      ort.req.qty = (max_position + cur_position)/2;
      ort.direction = SELL;
      ort.type = LIMIT;

      cout << "PLACING ORDER." << endl;
      auto res = client.post_order(ort);
      cout << "RESP: " << res << endl;
      outstanding_orders.push_back(*res);
    } catch (exception& e) { cerr << "error: " << e.what() << endl; }

    this_thread::sleep_for(2s);
    cout << "looping.\n";
  }  
  return 0;
}
