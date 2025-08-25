
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "logger.h"


static const char* log_level_to_string(log_type type) {
    switch (type) {
        case LOG_TYPE_TRACE: return "TRACE";
        case LOG_TYPE_DEBUG: return "DEBUG";
        case LOG_TYPE_INFO:  return "INFO";
        case LOG_TYPE_WARN:  return "WARN";
        case LOG_TYPE_ERROR: return "ERROR";
        case LOG_TYPE_FATAL: return "FATAL";
        default:             return "UNKNOWN";
    }
}
static b8 s_log_to_console = false;


b8 init_logger(const b8 log_to_console, const char* log_dir, const char* log_file_name, const b8 use_append_mode) {

    s_log_to_console = log_to_console;
    // TODO: make sure log file exists [log_dir/main_log_file_name.log]
    // TODO: clear old file if exists and [use_append_mode]
}


b8 shutdown_logger() {

}


void log_message(log_type type, const char* file_name, const char* function_name, const int line, const char* format, ...) {

    // Get current time
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);

    const char* color_start = "";
    const char* color_end = "\033[0m";
    switch (type) {
        case LOG_TYPE_TRACE: color_start = "\033[90m"; break; // Gray
        case LOG_TYPE_DEBUG: color_start = "\033[94m"; break; // Blue
        case LOG_TYPE_INFO:  color_start = "\033[92m"; break; // Green
        case LOG_TYPE_WARN:  color_start = "\033[93m"; break; // Yellow
        case LOG_TYPE_ERROR: color_start = "\033[91m"; break; // Red
        case LOG_TYPE_FATAL: color_start = "\x1B[1m\x1B[37m\x1B[41m"; break; // Magenta
        default: break;
    }

    fprintf(stderr, "%s[%s] %-5s%s ", color_start, time_buf, log_level_to_string(type), color_end);     // Print log header

    // Handle variable arguments
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}
