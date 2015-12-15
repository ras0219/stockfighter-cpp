#pragma once

#include <string>
#include <memory>

#include "msgs.h"

namespace web { namespace http { namespace client { class http_client; }}}

class sf_client {
public:
  sf_client(std::string apikey);
  sf_client(std::string apikey, std::string host);
  ~sf_client();

  optional<gm_start_level_t> start_level(const std::string& level);
  optional<gm_start_level_t> restart_level(uint64_t id);

  optional<unit> api_heartbeat();
  optional<symbolslist_t> venue_stocks(const std::string& venue);
  optional<orderbook_t> orderbook_for_venue(const std::string& venue, const std::string& symbol);
  optional<order_response_t> post_order(const order_request_t& order);
  optional<order_response_t> cancel_order(const std::string& venue, const std::string& symbol, uint64_t id);

  bool tracing;
  
private:
  std::string m_apikey;
  std::unique_ptr<web::http::client::http_client> m_client;
};


