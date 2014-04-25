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

add_library(
  json
  SHARED

  "src/lib_json/json_reader.cpp"
  "src/lib_json/json_value.cpp"
  "src/lib_json/json_writer.cpp"
)

mb_generate_config_files()
mb_set_api_macro("JSON_API")
mb_set_api_visibility()

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
