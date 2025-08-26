
#include "util/crash_handler.h"
#include "util/io/logger.h"
#include "application.h"


int main(int argc, char *argv[]) {

    ASSERT_SS(init_logger("[$B$T.$J $L$E][$B$Q $I $P:$G$E] $C", true, "logs", "application", false))            // logger should be external to application
    ASSERT_SS(init_crash_handler())
    LOGGER_REGISTER_THREAD_LABEL("main")


    // Example crash triggers (for testing):
    // *(int*)0 = 0;           // SIGSEGV
    abort();                // SIGABRT
    // int x = 1 / 0;          // SIGFPE

    shutdown_crash_handler();
    shutdown_logger();
    return 0;
}
