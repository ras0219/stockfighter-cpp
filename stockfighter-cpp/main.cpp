#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cpprest/http_client.h>

using namespace web::http;


using namespace std;

void fatal(const char* msg) {
  cerr << msg << "\n";
  abort();
}

int main() {
  ifstream f("~/.stockfighter");
  if (!f) fatal("error: ifstream()");
  string apikey;
  if (!getline(f, apikey)) fatal("error: getline()");

  

  printf("Hello, world!\n");
  return 0;
}
