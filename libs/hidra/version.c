#include "version.h"
#include <stdio.h>

const char* hidra_get_version(void) {
    return HIDRA_VERSION_SEMVER;
}

const char* hidra_get_version_full(void) {
    return HIDRA_VERSION_FULL;
}

const char* hidra_get_build_info(void) {
    static char build_info[256];
    snprintf(build_info, sizeof(build_info), 
             "HIDra %s (Build: %s, Commit: %.8s, Branch: %s)",
             HIDRA_VERSION_SEMVER,
             HIDRA_BUILD_TIMESTAMP,
             HIDRA_GIT_COMMIT,
             HIDRA_GIT_BRANCH);
    return build_info;
}

void hidra_print_version_info(void) {
    printf("=== HIDra Library Version Information ===\n");
    printf("Version: %s\n", HIDRA_VERSION_SEMVER);
    printf("Full Version: %s\n", HIDRA_VERSION_FULL);
    printf("Build Time: %s\n", HIDRA_BUILD_TIMESTAMP);
    printf("Git Commit: %s\n", HIDRA_GIT_COMMIT);
    printf("Git Branch: %s\n", HIDRA_GIT_BRANCH);
    printf("Build Config: %s\n", HIDRA_BUILD_CONFIG);
    printf("==========================================\n");
}
