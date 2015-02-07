
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doxybuild)
    set_target_properties(
        doxybuild
        PROPERTIES
        EXCLUDE_FROM_DEFAULT_BUILD TRUE)

    add_custom_command(
        TARGET doxybuild
        COMMAND python "${PROJECT_SOURCE_DIR}/doxybuild.py"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM)

    add_dependencies(docs doxybuild)

    install_docs(
        "${PROJECT_SOURCE_DIR}/dist/doxygen"
        _PREFIX jsoncpp
        DEPENDS doxybuild)
endif()

install_docs(
    "${PROJECT_SOURCE_DIR}/doc/readme.txt"
    PREFIX jsoncpp)
