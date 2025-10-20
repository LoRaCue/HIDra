# Generate version information from GitVersion
find_program(GITVERSION_EXECUTABLE gitversion)
if(GITVERSION_EXECUTABLE)
    # Get version components from GitVersion
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Major
        OUTPUT_VARIABLE FIRMWARE_VERSION_MAJOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Minor
        OUTPUT_VARIABLE FIRMWARE_VERSION_MINOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Patch
        OUTPUT_VARIABLE FIRMWARE_VERSION_PATCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable CommitsSinceVersionSource
        OUTPUT_VARIABLE FIRMWARE_VERSION_BUILD
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable SemVer
        OUTPUT_VARIABLE FIRMWARE_VERSION_SEMVER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable FullSemVer
        OUTPUT_VARIABLE FIRMWARE_VERSION_FULL
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Sha
        OUTPUT_VARIABLE FIRMWARE_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable BranchName
        OUTPUT_VARIABLE FIRMWARE_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    # Fallback values if GitVersion not available
    set(FIRMWARE_VERSION_MAJOR "0")
    set(FIRMWARE_VERSION_MINOR "0")
    set(FIRMWARE_VERSION_PATCH "1")
    set(FIRMWARE_VERSION_BUILD "0")
    set(FIRMWARE_VERSION_SEMVER "0.0.1-dev")
    set(FIRMWARE_VERSION_FULL "0.0.1-dev")
    set(FIRMWARE_GIT_COMMIT "unknown")
    set(FIRMWARE_GIT_BRANCH "unknown")
    message(WARNING "GitVersion not found, using fallback version information")
endif()

# Set additional build metadata
string(TIMESTAMP FIRMWARE_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)
set(FIRMWARE_VERSION_STRING "${FIRMWARE_VERSION_MAJOR}.${FIRMWARE_VERSION_MINOR}.${FIRMWARE_VERSION_PATCH}")
set(FIRMWARE_BUILD_CONFIG "${CMAKE_BUILD_TYPE}")
set(FIRMWARE_ESP_IDF_VERSION "${IDF_VER}")

# Generate version.h from template
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hidra_slave VERSION ${FIRMWARE_VERSION_STRING})
