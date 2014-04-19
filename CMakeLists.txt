# Copyright 2014 Makerbot Industries

# chosen arbitrarily
cmake_minimum_required(VERSION 2.8.9)

project(JsonCpp CXX)

set(${PROJECT_NAME}_VERSION 0.0.0)

# Common paths & utils
find_package(
  MakerBot
  CONFIG
  REQUIRED
  NO_CMAKE_BUILDS_PATH
)

include_directories(include/jsoncpp)

# This is the macro used in headers to indicate export of a function/class
set(API_DEFINE JSON_API)

if(WIN32)
  add_definitions("-D${API_DEFINE}=__declspec(dllexport)")
else()
  add_definitions(
    "-fvisibility=hidden"
    "-D${API_DEFINE}=__attribute__ ((visibility (\"default\")))")
endif()

add_library(
  json
  SHARED

  "src/lib_json/json_reader.cpp"
  "src/lib_json/json_value.cpp"
  "src/lib_json/json_writer.cpp"
)

# Generates the Config file other projects use to find this project
find_package(
  MakerBotConfigGen
  CONFIG
  REQUIRED
  NO_CMAKE_BUILDS_PATH
)

# Install The library, exporting the targets to the targets file
install(
  TARGETS json
  EXPORT ${TARGETS_FILE}
  RUNTIME DESTINATION ${BIN_INSTALL_DESTINATION}
  LIBRARY DESTINATION ${LIB_INSTALL_DESTINATION}
  ARCHIVE DESTINATION ${LIB_INSTALL_DESTINATION}
)

install(
  EXPORT ${TARGETS_FILE}
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CONFIG_INSTALL_DESTINATION}
)

# Install the include files
install(
  DIRECTORY include/jsoncpp
  DESTINATION ${INCLUDE_INSTALL_DESTINATION}
)
