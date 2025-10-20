# Generate version information from GitVersion
find_program(GITVERSION_EXECUTABLE gitversion)
if(GITVERSION_EXECUTABLE)
    # Get version components from GitVersion
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Major
        OUTPUT_VARIABLE HIDRA_VERSION_MAJOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Minor
        OUTPUT_VARIABLE HIDRA_VERSION_MINOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Patch
        OUTPUT_VARIABLE HIDRA_VERSION_PATCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable CommitsSinceVersionSource
        OUTPUT_VARIABLE HIDRA_VERSION_BUILD
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable SemVer
        OUTPUT_VARIABLE HIDRA_VERSION_SEMVER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable FullSemVer
        OUTPUT_VARIABLE HIDRA_VERSION_FULL
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable Sha
        OUTPUT_VARIABLE HIDRA_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GITVERSION_EXECUTABLE} /output json /showvariable BranchName
        OUTPUT_VARIABLE HIDRA_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    # Fallback values if GitVersion not available
    set(HIDRA_VERSION_MAJOR "0")
    set(HIDRA_VERSION_MINOR "0")
    set(HIDRA_VERSION_PATCH "1")
    set(HIDRA_VERSION_BUILD "0")
    set(HIDRA_VERSION_SEMVER "0.0.1-dev")
    set(HIDRA_VERSION_FULL "0.0.1-dev")
    set(HIDRA_GIT_COMMIT "unknown")
    set(HIDRA_GIT_BRANCH "unknown")
    message(WARNING "GitVersion not found, using fallback version information")
endif()

# Set additional build metadata
string(TIMESTAMP HIDRA_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)
set(HIDRA_VERSION_STRING "${HIDRA_VERSION_MAJOR}.${HIDRA_VERSION_MINOR}.${HIDRA_VERSION_PATCH}")
set(HIDRA_BUILD_CONFIG "${CMAKE_BUILD_TYPE}")

# Generate version.h from template
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

# Register component with version support
idf_component_register(
    SRCS "hidra.c" "version.c"
    INCLUDE_DIRS "." "../../protocol" "${CMAKE_CURRENT_BINARY_DIR}"
    REQUIRES driver
)

# Set component version for ESP-IDF component manager
idf_component_set_property(hidra COMPONENT_VERSION ${HIDRA_VERSION_SEMVER})
