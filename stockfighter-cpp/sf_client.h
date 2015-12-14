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

  optional<unit> api_heartbeat();
  optional<symbolslist_t> venue_stocks(const std::string& venue);

private:
  std::string m_apikey;
  std::unique_ptr<web::http::client::http_client> m_client;
};


