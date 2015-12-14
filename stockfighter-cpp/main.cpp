#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cpprest/http_client.h>

using namespace web::http;
using namespace client;

using namespace std;

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

  http_client client("https://api.stockfighter.io");

  auto res = client.request("GET", "/ob/api/venues").get();

  auto body = res.extract_utf8string(true).get();

  cout << body << "\n===END OF MESSAGE===\n";

  return 0;
}
