#include <iostream>
#include <cstdlib>
#include <thread>
#include "msgs.h"
#include "sf_client.h"
#include "util.h"

using namespace web::http;
using namespace client;
using namespace std;

int main(int argc, const char** argv) {
  if (argc < 2) FATAL_ERROR("usage: main <apikeyfile>");
  string apikey = read_api_key_from_file(argv[1]);

  cout << "API Key: '" << apikey << "'\n";

  sf_client client(apikey);

  auto slevel = client.start_level("chock_a_block");
  if (slevel->tickers.size() != 1) FATAL_ERROR("tickers");
  if (slevel->venues.size() != 1) FATAL_ERROR("venues");

  string account = slevel->account;
  string venue = slevel->venues.front();
  string symbol = slevel->tickers.front();

  uint64_t shares_to_buy = 100000;

  while (shares_to_buy > 0) {
    auto obook = client.orderbook_for_venue(venue, symbol);

    cout << "obook: " << obook << "\n";

    if (obook->asks.empty()) {
      this_thread::sleep_for(chrono::seconds(2));
      continue;
    }

    order_request_t ort;
    ort.account = account;
    ort.venue = venue;
    ort.symbol = symbol;
    ort.req = obook->asks.front();
    ort.direction = BUY;
    ort.type = IOC;

    if (ort.req.qty > shares_to_buy)
      ort.req.qty = shares_to_buy;
    try {
      auto res = client.post_order(ort);
      cout << "order: " << res << endl;

      shares_to_buy -= res->status.filled_qty;
    } catch (exception& e) {
      cerr << "warning: " << e.what() << "\n";
    } catch (...) {
      cerr << "warning: unknown error\n";
    }

    this_thread::sleep_for(chrono::seconds(1));
  }
  return 0;
}
