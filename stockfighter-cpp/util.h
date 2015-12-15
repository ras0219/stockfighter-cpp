#pragma once

#include <string>

#define FATAL_ERROR(FMT, ...) fatal_error(__LINE__, __FILE__, FMT, ##__VA_ARGS__)

std::string read_api_key_from_file(const std::string& filename);
void fatal_error(int lineno, const char* srcfile, const char* fmt, ...);
