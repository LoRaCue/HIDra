# Generate version information from GitVersion
find_program(GITVERSION_EXECUTABLE gitversion)
if(GITVERSION_EXECUTABLE)
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable SemVer
        OUTPUT_VARIABLE APP_VERSION_SEMVER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable FullSemVer
        OUTPUT_VARIABLE APP_VERSION_FULL
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Sha
        OUTPUT_VARIABLE APP_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable BranchName
        OUTPUT_VARIABLE APP_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(APP_VERSION_SEMVER "0.0.1-dev")
    set(APP_VERSION_FULL "0.0.1-dev")
    set(APP_GIT_COMMIT "unknown")
    set(APP_GIT_BRANCH "unknown")
endif()

string(TIMESTAMP APP_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

# Generate version.h from template
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

cmake_minimum_required(VERSION 3.16)

# Add libs directory to component search path
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../libs")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hidra_master_test VERSION ${APP_VERSION_SEMVER})
