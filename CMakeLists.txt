# Copyright 2014 Makerbot Industries

# chosen arbitrarily
cmake_minimum_required(VERSION 2.8.9)

project(json-cpp CXX)

set(JSONCPP_VERSION 0.0.0)

# Install locations
set(CONFIG_INSTALL_DESTINATION cmake)
set(LIB_INSTALL_DESTINATION lib)
set(BIN_INSTALL_DESTINATION bin)
set(INCLUDE_INSTALL_DESTINATION include)

include_directories(include/jsoncpp)

# This is the macro used in headers to indicate export of a function/class
set(JSONCPP_API JSON_API)

if(WIN32)
  add_definitions("-D${JSONCPP_API}=__declspec(dllexport)")
else()
  add_definitions(
    "-fvisibility=hidden"
    "-D${JSONCPP_API}=__attribute__ ((visibility (\"default\")))")
endif()

add_library(
  json
  SHARED

  "src/lib_json/json_reader.cpp"
  "src/lib_json/json_value.cpp"
  "src/lib_json/json_writer.cpp")

# for configure_package_config_file() and write_basic_package_version_file()
include(CMakePackageConfigHelpers)

# Names of files for exporting
set(JSONCPP_CONFIG "${CMAKE_BINARY_DIR}/JsonCppConfig.cmake")
set(JSONCPP_CONFIG_TEMPLATE "JsonCppConfig.cmake.in")
set(JSONCPP_CONFIG_VERSION "${CMAKE_BINARY_DIR}/JsonCppConfigVersion.cmake")
set(JSONCPP_TARGETS_FILE "JsonCppTargets")

# Create the config file from the config template
configure_package_config_file(
  "${JSONCPP_CONFIG_TEMPLATE}"
  "${JSONCPP_CONFIG}"
  INSTALL_DESTINATION ${CONFIG_INSTALL_DESTINATION}
  PATH_VARS
    CONFIG_INSTALL_DESTINATION
    INCLUDE_INSTALL_DESTINATION
    JSONCPP_TARGETS_FILE
)

# Create the version file
write_basic_package_version_file(
  "${JSONCPP_CONFIG_VERSION}"
  VERSION ${JSONCPP_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Install The library, exporting the targets to the targets file
install(
  TARGETS json
  EXPORT ${JSONCPP_TARGETS_FILE}
  RUNTIME DESTINATION ${BIN_INSTALL_DESTINATION}
  LIBRARY DESTINATION ${LIB_INSTALL_DESTINATION}
  ARCHIVE DESTINATION ${LIB_INSTALL_DESTINATION}
)

# Install the config & target files for import
install(
  FILES "${JSONCPP_CONFIG}" "${JSONCPP_CONFIG_VERSION}"
  DESTINATION ${CONFIG_INSTALL_DESTINATION}
)

install(
  EXPORT ${JSONCPP_TARGETS_FILE}
  NAMESPACE JSONCPP::
  DESTINATION ${CONFIG_INSTALL_DESTINATION}
)

# Install the include files
install(
  DIRECTORY include/jsoncpp
  DESTINATION ${INCLUDE_INSTALL_DESTINATION}
)
