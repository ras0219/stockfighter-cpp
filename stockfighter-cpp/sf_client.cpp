#include "sf_client.h"
#include <cpprest/http_client.h>

#define TRACE if (tracing) {                    \
    cerr << "\n|trace|" << __FILE__ << ":" << __LINE__ << "|\n"; }

using namespace web;
using namespace http;
using namespace std;
using namespace web::http;

sf_client::sf_client(std::string apikey)
  : sf_client(move(apikey), "https://api.stockfighter.io")
{}

sf_client::sf_client(string apikey, string host)
  : m_apikey(move(apikey)), m_client(make_unique<web::http::client::http_client>(host))
{
  tracing = false;
}

sf_client::~sf_client() {}

// This is a parser based on return type deduction.
// I am not sure how I feel about this... but it does make for very elegant code.
struct parser {
  const json::value& val;

  parser at(size_t sz) const {
    return parser{val.at(sz)};
  }
  parser at(const std::string& key) const {
    return parser{val.at(key)};
  }

  explicit operator string() const {
    return val.as_utf8string().c_str();
  }

  operator unit() const { return unit(); }
  
  operator uint64_t() const {
    return val.as_number().to_uint64();
  }

  operator vector<string>() const {
    vector<string> ret;
    if (val.is_null())
      return ret;
    for (auto&& e : val.as_array())
      ret.push_back(string(parser{e}));

    return ret;
  }
  
  template<class T>
  operator vector<T>() const {
    vector<T> ret;
    if (val.is_null())
      return ret;
    for (auto&& e : val.as_array())
      ret.push_back(parser{e});

    return ret;
  }

  operator ok_t() const {
    return val.as_bool() ? OK : ERROR;
  }
  
  operator fill_t() const {
    fill_t ret;
    ret.req.price = at("price");
    ret.req.qty = at("qty");
    ret.timestamp = string(at("ts"));
    return ret;
  }

  operator direction_t() const {
    return val.as_utf8string() == "buy" ? BUY : SELL;
  }

  operator open_t() const {
    return val.as_bool() ? OPEN : CLOSED;
  }

  operator request_t() const {
      request_t ret;
      ret.price = at("price");
      ret.qty = at("qty");
      return ret;
  }

  operator order_response_t() const {
    order_response_t ret;
    ret.id = at("id");
    ret.status = *this;
    return ret;
  }
  
  operator order_status_t() const {
    order_status_t ret;
    ret.original_qty = at("originalQty");
    ret.outstanding_qty = at("qty");
    ret.filled_qty = at("totalFilled");
    ret.direction = at("direction");
    ret.open = at("open");
    ret.timestamp = string(at("ts"));

    // fills
    ret.fills = at("fills");

    return ret;
  }

  operator orderbook_t() const {
      orderbook_t ret;
      ret.asks = at("asks");
      ret.bids = at("bids");
      ret.timestamp = string(at("ts"));
      return ret;
  }

  template<class T>
  operator optional<T>() const {
    optional<T> ret;
    ret.ok = at("ok");
    if (ret.ok == ERROR) {
      ret.err = string(at("error"));
      return ret;
    }

    ret.data = *this;
    return ret;
  }

  operator symboldef_t() const {
    symboldef_t ret;
    ret.symbol = string(at("symbol"));
    ret.name = string(at("name"));
    return ret;
  }

  operator symbolslist_t() const {
    symbolslist_t ret;
    ret.symbols = at("symbols");
    return ret;
  }
  
  operator gm_start_level_t() const {
    gm_start_level_t ret;
    ret.account = string(at("account"));
    ret.instance = at("instanceId");
    // TODO instructions
    ret.seconds_per_day = at("secondsPerTradingDay");
    ret.tickers = at("tickers");
    ret.venues = at("venues");
    return ret;
  }
};

optional<unit> sf_client::api_heartbeat() {
  TRACE;
  optional<unit> ret;

  auto res = m_client->request(methods::GET, "/ob/api/heartbeat").get();
  auto doc = res.extract_json().get();
  TRACE;
  return parser{doc};
}

optional<symbolslist_t> sf_client::venue_stocks(const std::string& venue) {
  TRACE;
  optional<symbolslist_t> ret;

  auto res = m_client->request(methods::GET, "/ob/api/venues/"+venue+"/stocks").get();
  auto doc = res.extract_json().get();

  TRACE;
  return parser{doc};
}

void parse_into(const json::array& arr, std::vector<request_t>& reqs) {
  for (auto&& req : arr) {
    auto& reqobj = req.as_object();
    reqs.push_back(request_t{
	reqobj.at("price").as_number().to_uint64(),
	  reqobj.at("qty").as_number().to_uint64()});
  }
}

optional<orderbook_t> sf_client::orderbook_for_venue(const std::string& venue, const std::string& stock) {
  TRACE;
  optional<orderbook_t> ret;
  
  auto res = m_client->request(methods::GET, "/ob/api/venues/"+venue+"/stocks/"+stock).get();
  auto doc = res.extract_json().get();

  //auto& obj = doc.as_object();
  //ret.ok = obj["ok"].as_bool() ? OK : ERROR;
  //switch (ret.ok) {
  //case ERROR:
  //  ret.err = obj["error"].as_utf8string();
  //  break;
  //case OK: {
  //  if (!obj["bids"].is_null()) parse_into(obj["bids"].as_array(), ret.data.bids);
  //  if (!obj["asks"].is_null()) parse_into(obj["asks"].as_array(), ret.data.asks);
  //  ret.data.timestamp = obj["ts"].as_string();
  //  break;
  //}
  //  }

  TRACE;
  return parser{doc};
  }

json::value order_to_json(const order_request_t& order) {
  std::vector<std::pair<utility::string_t, json::value>> fields;
  fields.emplace_back(U("account"), json::value::string(order.account));
  fields.emplace_back(U("venue"), json::value::string(order.venue));
  fields.emplace_back(U("stock"), json::value::string(order.symbol));
  fields.emplace_back(U("price"), json::value::number(order.req.price));
  fields.emplace_back(U("qty"), json::value::number(order.req.qty));
  fields.emplace_back(U("direction"), json::value::string(order.direction == BUY ? "buy" : "sell"));
  fields.emplace_back(U("orderType"), json::value::string(stringify(order.type)));
  return json::value::object(move(fields));
  }

template<class F>
vector<result_of_t<F(const json::value&)>> map(const json::value& v, F f) {
  using v_t = vector<result_of_t<F(const json::value&)>>;
  v_t ret;

  if (v.is_null())
    return ret;

  for (auto&& e : v.as_array())
    ret.push_back(f(e));
  
  return ret;
}

optional<order_response_t> sf_client::post_order(const order_request_t& order) {
  TRACE;
  optional<order_response_t> ret;
  auto uri = "/ob/api/venues/"+order.venue+"/stocks/"+order.symbol+"/orders";

  http_request req(methods::POST);
  req.set_request_uri(uri);
  req.headers().add("X-Starfighter-Authorization", m_apikey);
  req.set_body(order_to_json(order));

  auto res = m_client->request(req).get();
  auto doc = res.extract_json().get();

  TRACE;
  return parser{doc};
}

optional<order_response_t> sf_client::cancel_order(const std::string& venue,
                                                   const std::string& symbol,
                                                   uint64_t id) {
  TRACE;
  optional<order_response_t> ret;
  auto uri = "/ob/api/venues/"+venue+"/stocks/"+symbol+"/orders/"+to_string(id);

  cerr << "uri:" << uri << endl;
  
  http_request req(methods::DEL);
  req.set_request_uri(uri);
  req.headers().add("X-Starfighter-Authorization", m_apikey);

  auto res = m_client->request(req).get();
  auto doc = res.extract_json().get();

  TRACE;
  return parser{doc};
}

optional<gm_start_level_t> sf_client::start_level(const std::string& level) {
  TRACE;
  optional<gm_start_level_t> ret;

  auto uri = "/gm/levels/" + level;

  http_request req(methods::POST);
  req.set_request_uri(uri);
  req.headers().add("X-Starfighter-Authorization", m_apikey);

  auto res = m_client->request(req).get();
  auto doc = res.extract_json().get();

  TRACE;
  return parser{doc};
}

optional<gm_start_level_t> sf_client::restart_level(uint64_t id) {
  TRACE;
  optional<gm_start_level_t> ret;

  auto uri = "/gm/instances/" + to_string(id) + "/restart";

  http_request req(methods::POST);
  req.set_request_uri(uri);
  req.headers().add("X-Starfighter-Authorization", m_apikey);

  auto res = m_client->request(req).get();
  auto doc = res.extract_json().get();

  TRACE;
  return parser{doc};
}
