#include <iostream>
#include <cstdlib>
#include <thread>
#include "msgs.h"
#include "sf_client.h"
#include "util.h"

using namespace web::http;
using namespace client;
using namespace std;

void fatal(const char* msg) {
  cerr << msg << "\n";
  abort();
}

int main(int argc, const char** argv) {
  if (argc < 2) FATAL_ERROR("usage: main <apikeyfile>");
  string apikey = read_api_key_from_file(argv[1]);

  cout << "API Key: '" << apikey << "'\n";

  sf_client client(apikey);

  //client.tracing = true;

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

  string account = slevel->account;
  string venue = slevel->venues.front();
  string symbol = slevel->tickers.front();

  uint64_t total_buys = 0;
  uint64_t total_sells = 0;
  
  vector<order_response_t> outstanding_orders;

  while (1) {

    for (auto&& o : outstanding_orders) {
      auto canc = client.cancel_order(venue, symbol, o.id);

      if (canc->status.direction == BUY)
        total_buys += canc->status.filled_qty;
      if (canc->status.direction == SELL)
        total_sells += canc->status.filled_qty;
    }
    outstanding_orders.clear();

    cout << "trades to date: B:" << total_buys << " S:" << total_sells << endl;
    
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
      this_thread::sleep_for(chrono::seconds(1));
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

    {
      order_request_t ort;
      ort.account = account;
      ort.venue = venue;
      ort.symbol = symbol;
      ort.req.price = best_bid + 5;
      ort.req.qty = 20;
      ort.direction = BUY;
      ort.type = LIMIT;

      cout << "PLACING ORDER." << endl;
      auto res = client.post_order(ort);
      cout << "RESP: " << res << endl;
      outstanding_orders.push_back(*res);
    }    

    {
      order_request_t ort;
      ort.account = account;
      ort.venue = venue;
      ort.symbol = symbol;
      ort.req.price = best_ask - 5;
      ort.req.qty = 20;
      ort.direction = SELL;
      ort.type = LIMIT;

      cout << "PLACING ORDER." << endl;
      auto res = client.post_order(ort);
      cout << "RESP: " << res << endl;
      outstanding_orders.push_back(*res);
    }    

    this_thread::sleep_for(chrono::seconds(3));
    cout << "looping.\n";
  }  
  return 0;
}
