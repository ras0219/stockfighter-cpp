#include "sf_client.h"
#include <cpprest/http_client.h>

using namespace std;

sf_client::sf_client(std::string apikey)
  : sf_client(move(apikey), "https://api.stockfighter.io")
{}

sf_client::sf_client(string apikey, string host)
  : m_apikey(move(apikey)), m_client(make_unique<web::http::client::http_client>(host))
{}

sf_client::~sf_client() {}

optional<unit> sf_client::api_heartbeat() {
  optional<unit> ret;

  auto res = m_client->request("GET", "/ob/api/heartbeat").get();
  auto doc = res.extract_json().get();
  auto& obj = doc.as_object();
  ret.ok = obj["ok"].as_bool() ? OK : ERROR;
  switch (ret.ok) {
  case OK: break;
  case ERROR:
    ret.err = obj["error"].as_string();
    break;
  }

  return ret;
}


optional<symbolslist_t> sf_client::venue_stocks(const std::string& venue) {
  optional<symbolslist_t> ret;

  auto res = m_client->request("GET", "/ob/api/venues/"+venue+"/stocks").get();
  auto doc = res.extract_json().get();
  auto& obj = doc.as_object();
  ret.ok = obj["ok"].as_bool() ? OK : ERROR;
  switch (ret.ok) {
  case ERROR:
    ret.err = obj["error"].as_string();
    break;
  case OK: {
    auto& arr = obj["symbols"].as_array();
    for (auto&& sym : arr) {
      auto& symobj = sym.as_object();
      ret.data.symbols.push_back(symboldef_t{
	  symobj["symbol"].as_string(),
	    symobj["name"].as_string()
	    });
    }
    break;
  }
  }

  return ret;
}
