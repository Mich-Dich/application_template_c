
#include <stdio.h>

#include "util/io/logger.h"

int main(int argc, char const *argv[]) {

    LOG(Trace, "This is a trace log");
    LOG(Debug, "Debugging value x = %d", 42);
    LOG(Info,  "Initialization complete");
    LOG(Warn,  "Low memory warning!");
    LOG(Error, "Failed to open file");
    LOG(Fatal, "System crash: %s", "rebooting...");

    printf("Before breakpoint\n");
    BREAK_POINT();
    printf("After breakpoint (if execution continued)\n");

    return 0;
}
