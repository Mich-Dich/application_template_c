
#include "util/crash_handler.h"
#include "util/io/logger.h"
#include "application.h"


int main(int argc, char *argv[]) {

    ASSERT_SS(init_logger("[$B$T.$J $L$E][$B$Q $I $P:$G$E] $C", true, "logs", "application", false))            // logger should be external to application
    ASSERT_SS(init_crash_handler())
    LOGGER_REGISTER_THREAD_LABEL("main")

    VALIDATE(init_application(argc, argv), shutdown_logger(); return 1, "", "Failed to init the application")
    run_application();
    shutdown_application();

    shutdown_crash_handler();
    shutdown_logger();
    return 0;
}
