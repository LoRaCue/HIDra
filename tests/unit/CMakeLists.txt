# Generate version information from GitVersion for tests
find_program(GITVERSION_EXECUTABLE gitversion)
if(GITVERSION_EXECUTABLE)
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable SemVer
        OUTPUT_VARIABLE TEST_VERSION_SEMVER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(TEST_VERSION_SEMVER "0.0.1")
endif()

cmake_minimum_required(VERSION 3.16)

set(COMPONENTS main)
set(EXTRA_COMPONENT_DIRS "../../libs" "../../protocol")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hidra_unit_tests VERSION ${TEST_VERSION_SEMVER})
