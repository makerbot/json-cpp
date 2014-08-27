OPTION(JSONCPP_LIB_BUILD_SHARED "Build jsoncpp_lib as a shared library." OFF)
IF(JSONCPP_LIB_BUILD_SHARED)
  SET(JSONCPP_LIB_TYPE SHARED)
  ADD_DEFINITIONS( -DJSON_DLL_BUILD )
ELSE()
  SET(JSONCPP_LIB_TYPE STATIC)
ENDIF()


SET( JSONCPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/jsoncpp )

SET( PUBLIC_HEADERS
    ${JSONCPP_INCLUDE_DIR}/json/config.h
    ${JSONCPP_INCLUDE_DIR}/json/forwards.h
    ${JSONCPP_INCLUDE_DIR}/json/features.h
    ${JSONCPP_INCLUDE_DIR}/json/value.h
    ${JSONCPP_INCLUDE_DIR}/json/reader.h
    ${JSONCPP_INCLUDE_DIR}/json/writer.h
    ${JSONCPP_INCLUDE_DIR}/json/assertions.h
    ${JSONCPP_INCLUDE_DIR}/json/version.h
    ${JSONCPP_INCLUDE_DIR}/json/autolink.h
    ${JSONCPP_INCLUDE_DIR}/json/json.h )

SOURCE_GROUP( "Public API" FILES ${PUBLIC_HEADERS} )

ADD_LIBRARY( jsoncpp_lib ${JSONCPP_LIB_TYPE}
    ${PUBLIC_HEADERS}
    json_tool.h
    json_reader.cpp
    json_batchallocator.h
    json_valueiterator.inl
    json_internalarray.inl
    json_internalmap.inl
    json_value.cpp
    json_writer.cpp
    version.h.in )
SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES OUTPUT_NAME jsoncpp )
SET_TARGET_PROPERTIES( jsoncpp_lib PROPERTIES
    VERSION ${JSON_CPP_VERSION}
    SOVERSION ${JSON_CPP_VERSION} )

# Install instructions for this target
INSTALL( TARGETS jsoncpp_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib )
