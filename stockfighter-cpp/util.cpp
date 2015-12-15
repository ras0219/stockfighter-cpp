#include "util.h"
#include <cstdio>
#include <cstdarg>
#include <fstream>

using namespace std;

void fatal_error(int lineno, const char* srcfile, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  fprintf(stderr, "%s:%d: fatal error: ", srcfile, lineno);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  
  va_end(args);
  terminate();
}

std::string read_api_key_from_file(const std::string& filename) {
  string apikey;

  ifstream f(filename);
  if (!f) FATAL_ERROR("ifstream(%s)", filename.c_str());
  if (!getline(f, apikey)) FATAL_ERROR("getline()");

  return apikey;
}
