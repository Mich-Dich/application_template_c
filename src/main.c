
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h> 

#include "util/io/logger.h"


int main(void) {

    init_logger("[$B$T.$J $L$E][$B$Q $I $P:$G$E] $C", true, "logs", "application", false);
    LOGGER_REGISTER_THREAD_LABEL("main")
    
    LOG(Trace, "This is a trace log");
    LOG(Debug, "Debugging value x = %d", 42);
    LOG(Info,  "Initialization complete");
    LOG(Warn,  "Warning!");
    LOG(Error, "Failed some random task");
    LOG(Fatal, "System crash: %s", "rebooting...");

    LOG(Trace, "Before breakpoint");
    // BREAK_POINT();
    LOG(Trace, "After breakpoint (if execution continued)");

    return 0;
}
