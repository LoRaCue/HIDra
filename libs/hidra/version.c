#include <stdio.h>
#include "version.h"

void firmware_print_version_info(void)
{
    printf("HIDra Firmware Version: %s\n", HIDRA_VERSION_FULL);
    printf("Git Commit: %s\n", HIDRA_GIT_COMMIT);
    printf("Git Branch: %s\n", HIDRA_GIT_BRANCH);
    printf("Build Timestamp: %s\n", HIDRA_BUILD_TIMESTAMP);
    printf("Build Configuration: %s\n", HIDRA_BUILD_CONFIG);
}
