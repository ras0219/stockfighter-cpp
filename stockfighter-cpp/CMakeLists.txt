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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-std=c++14)
endif()

include_directories(${Casablanca_INCLUDE_DIRS})
link_libraries(${Casablanca_LIBRARIES} stdc++)

add_library(sf STATIC msgs.cpp msgs.h sf_client.cpp sf_client.h)
link_libraries(sf)

add_executable(level2 level2.cpp)
add_executable(level3 level3.cpp)
