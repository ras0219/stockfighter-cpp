#include "sf_client.h"
#include "util.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    cout << "Usage: " << argv[0] << " <api-key-file> <level-name>\n";
    exit(-1);
  }

  string apikey = read_api_key_from_file(argv[1]);
  sf_client client(apikey);

  cout << client.start_level(argv[2]) << endl;
  
  return 0;
}
