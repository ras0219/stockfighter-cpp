cmake_minimum_required(VERSION 2.8)
project(stockfighter-cpp)

set(BUILD_SHARED_LIBS 0)
set(BUILD_TESTS 0 CACHE BOOL "")
set(BUILD_SAMPLES 0 CACHE BOOL "")
set(CPPREST_EXCLUDE_WEBSOCKETS 1 CACHE BOOL "Disable websockets functionality")
add_definitions(-D_NO_ASYNCRTIMP)
add_subdirectory(
  ../cpprestsdk/Release
  cpprestsdk
  EXCLUDE_FROM_ALL)

get_directory_property(
  Casablanca_LIBRARIES
  DIRECTORY ../cpprestsdk/Release
  DEFINITION Casablanca_LIBRARIES)
get_directory_property(
  Casablanca_INCLUDE_DIRS
  DIRECTORY ../cpprestsdk/Release
  DEFINITION Casablanca_INCLUDE_DIRS)

add_executable(main main.cpp)
target_include_directories(main PUBLIC ${Casablanca_INCLUDE_DIRS})
target_link_libraries(main ${Casablanca_LIBRARIES})
