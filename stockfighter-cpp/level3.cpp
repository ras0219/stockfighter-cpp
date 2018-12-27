#include <iostream>
#include <cstdlib>
#include <thread>
#include <algorithm>
#include <deque>
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
start_level:
  auto slevel = [&](){
      if (argc >= 3) {
          // instance id provided
          return client.restart_level(atoi(argv[2]));
      } else {
          return client.start_level("sell_side");
      }
  }();

  if (slevel.ok == ERROR && slevel.err == "This level has been locked by a different server.  (Sorry if this happened accidentally as a result of a server crash -- it will clear up in an hour.)")
  {
      this_thread::sleep_for(chrono::seconds(10));
      goto start_level;
  }

  if (slevel->tickers.size() != 1) FATAL_ERROR("tickers");
  if (slevel->venues.size() != 1) FATAL_ERROR("venues");

  cout << "=== level start: "<<slevel<<" ===" << endl;

  deque<int64_t> max_price;
  deque<int64_t> min_price;

  static const size_t window_size = 80;

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

    auto net_stock = int64_t(total_buys) - int64_t(total_sells);
    auto net_cash = int64_t(total_cash_recv) - int64_t(total_cash_sent);

    cout << "trades to date: B:" << total_buys << " S:" << total_sells << " N:" << net_stock << endl;
    cout << "cash:           B:" << total_cash_sent << " S:" << total_cash_recv << " N:" << net_cash << endl;
    cout << "break-even price: " << -(float(net_cash) / float(net_stock)) << endl;
    
    auto obook = client.orderbook_for_venue(venue, symbol);
    cout << "obook: " << obook << endl;
    vector<request_t> asks = move(obook->asks);
    vector<request_t> bids = move(obook->bids);

    //asks.erase(remove_if(asks.begin(), asks.end(), [](request_t const& r) { return r.qty < 100; }), asks.end());
    //bids.erase(remove_if(bids.begin(), bids.end(), [](request_t const& r) { return r.qty < 100; }), bids.end());

    uint64_t best_ask = 0;
    uint64_t best_bid = 0;

//    uint64_t default_spread = 200;

    if (asks.empty() && bids.empty()) {
      // sleep and try again
      cout << "no offers. sleeping." << endl;
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    } else if (bids.empty()) {
        max_price.push_back(asks.front().price);
        min_price.push_back(asks.front().price);
    } else if (asks.empty()) {
        max_price.push_back(bids.front().price);
        min_price.push_back(bids.front().price);
    } else {
        max_price.push_back(asks.front().price);
        min_price.push_back(bids.front().price);
    }

    cout << "best ask: " << best_ask << endl;
    cout << "best bid: " << best_bid << endl;

    auto min_price_window = *std::min_element(min_price.begin(), min_price.end());
    auto max_price_window = *std::max_element(max_price.begin(), max_price.end());
    if (min_price.size() == window_size)
    {
        min_price.pop_front();
        max_price.pop_front();

        int64_t cur_position = int64_t(total_buys) - int64_t(total_sells);
    
        try {
          order_request_t ort;
          ort.account = account;
          ort.venue = venue;
          ort.symbol = symbol;
          ort.req.price = (max_price_window + min_price_window) / 2 * (0.95 - 0.5 * cur_position / max_position);
          ort.req.qty = (max_position - cur_position)/20;
          ort.direction = BUY;
          ort.type = LIMIT;

          cout << "PLACING ORDER: " << ort << endl;
          auto res = client.post_order(ort);
          cout << "RESP: " << res << endl;
          outstanding_orders.push_back(*res);
        } catch (exception& e) { cerr << "error: " << e.what() << endl; }

        try {
          order_request_t ort;
          ort.account = account;
          ort.venue = venue;
          ort.symbol = symbol;
          ort.req.price = (max_price_window + min_price_window) / 2 * (1.08 - 0.5 * cur_position / max_position);
          ort.req.qty = (max_position + cur_position)/20;
          ort.direction = SELL;
          ort.type = LIMIT;

          cout << "PLACING ORDER: " << ort << endl;
          auto res = client.post_order(ort);
          cout << "RESP: " << res << endl;
          outstanding_orders.push_back(*res);
        } catch (exception& e) { cerr << "error: " << e.what() << endl; }

    }
    this_thread::sleep_for(chrono::seconds(1));
    cout << "looping.\n";
  }  
  return 0;
}
