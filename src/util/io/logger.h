#pragma once

#include <pthread.h>

#include "util/breakpoint.h"
#include "util/core_config.h"
#include "util/data_structure/data_types.h"


typedef enum {
    LOG_TYPE_TRACE = 0,
    LOG_TYPE_DEBUG,
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR,
    LOG_TYPE_FATAL,
} log_type;


//
b8 init_logger(const char* log_msg_format, const b8 log_to_console, const char* log_dir, const char* log_file_name, const b8 use_append_mode);

//
b8 shutdown_logger();

// 
void log_message(log_type type, u64 thread_id, const char* file_name, const char* function_name, const int line, const char* message, ...);

// The format of log-messages can be customized with the following tags
// @note to format all following log-messages use: set_format()
// @note e.g. set_format("$B[$T] $L [$F] $C$E")
//
// @param $T time                    hh:mm:ss
// @param $H hour                    hh
// @param $M minute                  mm
// @param $S secund                  ss
// @param $J millisecused            jjj
//      
// @param $N data                    yyyy:mm:dd
// @param $Y data year               yyyy
// @param $O data month              mm
// @param $D data day                dd
//
// @param $M thread                  Thread_id: 137575225550656 or a label if provided
// @param $F function name           application::main, math::foo
// @param $P only function name      main, foo
// @param $A file name               /home/workspace/test_cpp/src/main.cpp  /home/workspace/test_cpp/src/project.cpp
// @param $I only file name          main.cpp
// @param $G line                    1, 42
//
// @param $L log-level               add used log severity: [TRACE], [DEBUG] ... [FATAL]
// @param $B color begin             from here the color begins
// @param $E color end               from here the color will be reset
// @param $C text                    the message the user wants to print
// @param $Z new line                add a new line in the message format
void set_format(const char* new_format);

//
void register_thread_label(u64 thread_id, const char* label);

#define LOGGER_REGISTER_THREAD_LABEL(label)     register_thread_label((u64)pthread_self(), label);






// This enables the different log levels (FATAL + ERROR are always on)
//  0 = FATAL + ERROR
//  1 = FATAL + ERROR + WARN
//  2 = FATAL + ERROR + WARN + INFO
//  3 = FATAL + ERROR + WARN + INFO + DEBUG
//  4 = FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define LOG_LEVEL_ENABLED           			4




#define LOG_Fatal(message, ...)                                         { log_message(LOG_TYPE_FATAL, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#define LOG_Error(message, ...)                                         { log_message(LOG_TYPE_ERROR, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }

#if LOG_LEVEL_ENABLED > 0
    #define LOG_Warn(message, ...)                                      { log_message(LOG_TYPE_WARN, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Warn(message, ...)                                      { }
#endif

#if LOG_LEVEL_ENABLED > 1
    #define LOG_Info(message, ...)                                      { log_message(LOG_TYPE_INFO, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Info(message, ...)                                      { }
#endif

#if LOG_LEVEL_ENABLED > 2
    #define LOG_Debug(message, ...)                                     { log_message(LOG_TYPE_DEBUG, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Debug(message, ...)                                     { }
#endif


#if LOG_LEVEL_ENABLED > 3
    #define LOG_Trace(message, ...)                                     { log_message(LOG_TYPE_TRACE, (u64)pthread_self(), __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
    #define LOG_INIT                                                    LOG(Trace, "init")
    #define LOG_SHUTDOWN                                                LOG(Trace, "shutdown")
#else
    #define LOG_Trace(message, ...)                                     { }
    #define LOG_INIT
    #define LOG_SHUTDOWN
#endif

#define LOG(severity, message, ...)                                     LOG_##severity(message, ##__VA_ARGS__)


#if ENABLE_LOGGING_FOR_VALIDATION
    #define VALIDATE(expr, return_cmd, success_msg, failure_msg)        \
        if (expr) { LOG(Trace, success_msg) }                           \
        else {                                                          \
            LOG(Warn, failure_msg)                                      \
            return_cmd;                                                 \
        }

    #define VALIDATE_S(expr, return_cmd)                                \
        if (!(expr)) {                                                  \
            LOG(Error, "Validation failed for [%s]", ##expr)            \
            return_cmd;                                                 \
        }
#else
    #define VALIDATE(expr, return_cmd, success_msg, failure_msg)        if (!(expr)) { return_cmd; }
    #define VALIDATE_s(expr, return_cmd, success_msg, failure_msg)      if (!(expr)) { return_cmd; }
#endif



#if ENABLE_LOGGING_FOR_ASSERTS
    #define ASSERT(expr, success_msg, failure_msg)                      \
        if (expr) { LOG(Trace, success_msg) }                           \
        else {                                                          \
            LOG(Fatal, failure_msg)                                     \
            BREAK_POINT();                                              \
        }
    #define ASSERT_S(expr)                                              \
        if (!(expr)) {                                                  \
            LOG(Fatal, "Assert failed for [%s]", ##expr)                \
            BREAK_POINT();                                              \
        }
#else
    #define ASSERT(expr, success_msg, failure_msg)                      if (!(expr)) { BREAK_POINT(); }
    #define ASSERT_s(expr)                                              if (!(expr)) { BREAK_POINT(); }
#endif

// extra short version, does not log anything just test an expression
#define ASSERT_SS(expr)                                                 if (!(expr)) { BREAK_POINT(); }