#pragma once

typedef enum log_type {
    LOG_TYPE_TRACE,
    LOG_TYPE_DEBUG,
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR,
    LOG_TYPE_FATAL,
} log_type;


void log_message(log_type type, const char* format, ...);


#define LOG_Trace(message, ...)         log_message(LOG_TYPE_TRACE, message, ##__VA_ARGS__);
#define LOG_Debug(message, ...)         log_message(LOG_TYPE_DEBUG, message, ##__VA_ARGS__);
#define LOG_Info(message, ...)          log_message(LOG_TYPE_INFO,  message, ##__VA_ARGS__);
#define LOG_Warn(message, ...)          log_message(LOG_TYPE_WARN,  message, ##__VA_ARGS__);
#define LOG_Error(message, ...)         log_message(LOG_TYPE_ERROR, message, ##__VA_ARGS__);
#define LOG_Fatal(message, ...)         log_message(LOG_TYPE_FATAL, message, ##__VA_ARGS__);

#define LOG(severity, message, ...)     LOG_##severity(message, ##__VA_ARGS__)
