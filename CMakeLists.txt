# chosen arbitrarily
cmake_minimum_required(VERSION 2.8.9)

project(json-cpp)

include_directories(include/jsoncpp)

add_library(
  json-cpp
  SHARED

  src/lib_json/json_reader.cpp
  src/lib_json/json_value.cpp
  src/lib_json/json_writer.cpp)
