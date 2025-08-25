#pragma once

#include "util/breakpoint.h"
#include "util/core_config.h"
#include "util/data_structure/data_types.h"


typedef enum log_type {
    LOG_TYPE_TRACE,
    LOG_TYPE_DEBUG,
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR,
    LOG_TYPE_FATAL,
} log_type;


//
b8 init_logger(const b8 log_to_console, const char* log_dir, const char* log_file_name, const b8 use_append_mode);

#define INIT_LOGGER_DEFAULTS() init_logger(false, "./logs", "general.log", false)

//
b8 shutdown_logger();

// 
void log_message(log_type type, const char* file_name, const char* function_name, const int line, const char* format, ...);




// This enables the different log levels (FATAL + ERROR are always on)
//  0 = FATAL + ERROR
//  1 = FATAL + ERROR + WARN
//  2 = FATAL + ERROR + WARN + INFO
//  3 = FATAL + ERROR + WARN + INFO + DEBUG
//  4 = FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define LOG_LEVEL_ENABLED           			4




#define LOG_Fatal(message, ...)                                         { log_message(LOG_TYPE_FATAL, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#define LOG_Error(message, ...)                                         { log_message(LOG_TYPE_ERROR, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }

#if LOG_LEVEL_ENABLED > 0
    #define LOG_Warn(message, ...)                                      { log_message(LOG_TYPE_WARN, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Warn(message, ...)                                      { }
#endif

#if LOG_LEVEL_ENABLED > 1
    #define LOG_Info(message, ...)                                      { log_message(LOG_TYPE_INFO, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Info(message, ...)                                      { }
#endif

#if LOG_LEVEL_ENABLED > 2
    #define LOG_Debug(message, ...)                                     { log_message(LOG_TYPE_DEBUG, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Debug(message, ...)                                     { }
#endif


#if LOG_LEVEL_ENABLED > 3
    #define LOG_Trace(message, ...)                                     { log_message(LOG_TYPE_TRACE, __FILE__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__); }
#else
    #define LOG_Trace(message, ...)                                     { }
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
            LOG(Error, failure_msg)                                     \
            BREAK_POINT();                                              \
        }
    #define ASSERT_S(expr, success_msg, failure_msg)                    \
        if (!(expr)) {                                                  \
            LOG(Error, "Assert failed for [%s]", ##expr)                \
            BREAK_POINT();                                              \
        }
#else
    #define ASSERT(expr, success_msg, failure_msg)                      if (!(expr)) { BREAK_POINT(); }
    #define ASSERT_s(expr, success_msg, failure_msg)                    if (!(expr)) { BREAK_POINT(); }
#endif
