#pragma once

#include "util/data_structure/data_types.h"



// @brief Initializes the crash handler and sets up custom signal handlers
//        for critical signals such as SIGSEGV, SIGABRT, SIGFPE, SIGILL, and SIGBUS.
//        When a crash occurs, the handler captures a backtrace, resolves symbol names,
//        attempts source file and line resolution, and logs detailed crash information.
// @note On Linux, this uses `sigaction` with `SA_SIGINFO` to intercept crash signals.
// @return Returns `true` if all crash signal handlers were successfully installed,
//         otherwise returns `false`.
b8 init_crash_handler();


// @brief Shuts down the crash handler and restores the original signal handlers.
//        After calling this function, crash signals will no longer be intercepted,
//        and the default system behavior for those signals is restored.
void shutdown_crash_handler();
