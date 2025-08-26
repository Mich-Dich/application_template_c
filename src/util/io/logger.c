
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h> 

#include "util/data_structure/dynamic_string.h"
#include "util/system.h"

#include "logger.h"


// ============================================================================================================================================
// format helper
// ============================================================================================================================================

inline static const char *short_filename(const char *path) {

    const char *s1 = strrchr(path, '/');
    const char *s2 = strrchr(path, '\\');
    const char *last = NULL;

    if (s1 && s2) 
        last = (s1 > s2) ? s1 : s2;
    else
        last = s1 ? s1 : s2;

    return last ? last + 1 : path;
}


static const char* severity_names[] = {
    "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};


const char* log_level_to_string(log_type t) {

    if ((int)t < 0 || (size_t)t >= sizeof(severity_names) / sizeof(severity_names[0]))
        return "UNK";
    return severity_names[(int)t];
}


static const char* console_color_table[] = {
    "\033[90m",                 // TRACE - gray
    "\033[94m",                 // DEBUG - blue
    "\033[92m",                 // INFO  - green
    "\033[93m",                 // WARN  - yellow
    "\033[91m",                 // ERROR - red
    "\x1B[1m\x1B[37m\x1B[41m"   // FATAL - bold white on red
};
static const char*              console_rest = "\033[0m";

static const char*              default_format = "[$B$T $L] $E $P:$G $C$Z";

static char*                    s_format_current = NULL;


// ============================================================================================================================================
// thread label map (simple linked list)
// ============================================================================================================================================

typedef struct thread_label_node {
    u64                         thread_id;
    char*                       label;
    struct thread_label_node*   next;
} thread_label_node;

static thread_label_node*       s_thread_labels = NULL;

static pthread_mutex_t          s_general_mutex = PTHREAD_MUTEX_INITIALIZER;


void register_thread_label(u64 thread_id, const char* label) {

    // TODO: only print this to the log file
    // printf("registering thread [%ul] under [%s]\n", thread_id, label);

    pthread_mutex_lock(&s_general_mutex);
    struct thread_label_node* current = s_thread_labels;
    while (current) {
        if (current->thread_id == thread_id) {
            free(current->label);
            current->label = strdup(label ? label : "");
            pthread_mutex_unlock(&s_general_mutex);
            return;
        }
        current = current->next;
    }
    // not found, append
    struct thread_label_node* node = malloc(sizeof(*node));
    node->thread_id = thread_id;
    node->label = strdup(label ? label : "");
    node->next = s_thread_labels;
    s_thread_labels = node;
    pthread_mutex_unlock(&s_general_mutex);
}


const char* lookup_thread_label(u64 thread_id) {

    struct thread_label_node* n = s_thread_labels;
    while (n) {
        if (n->thread_id == thread_id)
            return n->label;
        n = n->next;
    }
    return NULL;
}


// ============================================================================================================================================
// data
// ============================================================================================================================================

static b8 s_log_to_console = false;

static char* s_log_file_path = NULL;

static char s_log_msg_buffer[32000] = {0};


// ============================================================================================================================================
// private functions
// ============================================================================================================================================

bool flush_log_msg_buffer(const char* log_msg) {
    
    if (s_log_file_path == NULL)
        return false;

    FILE* fp = fopen(s_log_file_path, "a");
    if (fp == NULL)
        BREAK_POINT();          // logger failed to create log file

    // TODO: dump content of buffer
    fprintf(fp, "%s", s_log_msg_buffer);

    if (log_msg)                            // log provided message if exist
        fprintf(fp, "%s", log_msg);

    fclose(fp);
    return true;
}


// ============================================================================================================================================
// LOGGER
// ============================================================================================================================================


b8 init_logger(const char* log_msg_format, const b8 log_to_console, const char* log_dir, const char* log_file_name, const b8 use_append_mode) {

    s_log_to_console = log_to_console;
    set_format(log_msg_format);

    const char* exec_path = get_executable_path();
    if (exec_path == NULL)
        return false;

    char file_path[256];
    memset(file_path, '\0', sizeof(file_path));
    snprintf(file_path, sizeof(file_path), "%s/%s", exec_path, log_dir);
    // printf("log path: %s\n", file_path);

    if (!mkdir(file_path, 0777))        // create dir
        if (errno != EEXIST)            // OK if exist
            BREAK_POINT();

    memset(file_path, '\0', sizeof(file_path));
    snprintf(file_path, sizeof(file_path), "%s/%s/%s.log", exec_path, log_dir, log_file_name);
    s_log_file_path = strdup(file_path);
    // printf("log file path: %s\n", file_path);

    system_time st = get_system_time();
    FILE* fp = fopen(s_log_file_path, (use_append_mode) ? "a" : "w");
    ASSERT_SS(fp)

    fprintf(fp, "=====================================================================================================\n");
    fprintf(fp, "Log initalized at [%04d/%02d/%02d %02d:%02d:%02d] with format: %s\n", st.year, st.month, st.day, st.hour, st.minute, st.second, s_format_current);
    fprintf(fp, "-----------------------------------------------------------------------------------------------------\n");

    fclose(fp);

    return true;
}


b8 shutdown_logger() {

    system_time st = get_system_time();

    dyn_str out;
    ds_init(&out);

    ds_append_str(&out, "-----------------------------------------------------------------------------------------------------\n");
    ds_append_fmt(&out, "Closing Log at [%04d/%02d/%02d %02d:%02d:%02d]\n", st.year, st.month, st.day, st.hour, st.minute, st.second);
    ds_append_str(&out, "=====================================================================================================\n");

    ASSERT_SS(flush_log_msg_buffer(out.buf))

    ds_free(&out);

    return true;
}


void set_format(const char* new_format) {

    pthread_mutex_lock(&s_general_mutex);
    free(s_format_current);
    if (new_format)
        s_format_current = strdup(new_format);
    else
        s_format_current = strdup(default_format);
    pthread_mutex_unlock(&s_general_mutex);
}


// ============================================================================================================================================
// message formatter
// ============================================================================================================================================

// ----------------- main formatter - expects the message text to be already formatted -----------------
void process_log_message_v(log_type type, u64 thread_id, const char* file_name, const char* function_name, int line, const char* formatted_message) {
    
    if (!formatted_message)
        formatted_message = "";

    system_time st = get_system_time();

    // build formatted output according to s_format_current
    pthread_mutex_lock(&s_general_mutex);
    const char* fmt = s_format_current ? s_format_current : default_format;

    dyn_str out;
    ds_init(&out);

    size_t fmt_len = strlen(fmt);
    for (size_t i = 0; i < fmt_len; ++i) {
        char c = fmt[i];
        if (c == '$') {
            if (i + 1 >= fmt_len) break;
            char cmd = fmt[++i];
            switch (cmd) {
                case 'B': ds_append_str(&out, console_color_table[(int)type]); break;                           // color begin
                case 'E': ds_append_str(&out, console_rest); break;                                             // color end
                case 'C': ds_append_str(&out, formatted_message); break;                                        // message content
                case 'L': ds_append_str(&out, log_level_to_string(type)); break;                                // severity
                case 'Z': ds_append_char(&out, '\n'); break;                                                    // newline
                case 'Q': {                                                                                     // thread id or label
                    const char* label = lookup_thread_label(thread_id);
                    if (label)  ds_append_str(&out, label);
                    else        ds_append_fmt(&out, "%lu", thread_id);
                } break;
                case 'F': ds_append_str(&out, function_name ? function_name : ""); break;                       // function
                case 'P': ds_append_str(&out, function_name); break;                                            // short function
                case 'A': ds_append_str(&out, file_name ? file_name : ""); break;                               // file
                case 'I': ds_append_str(&out, short_filename(file_name ? file_name : "")); break;               // short file
                case 'G': ds_append_fmt(&out, "%d", line); break;                                               // line
                
                case 'T': ds_append_fmt(&out, "%02d:%02d:%02d", st.hour, st.minute, st.second); break;          // time component
                case 'H': ds_append_fmt(&out, "%02d", st.hour); break;                                          // time component
                case 'M': ds_append_fmt(&out, "%02d", st.minute); break;                                        // time component
                case 'S': ds_append_fmt(&out, "%02d", st.second); break;                                        // time component
                case 'J': ds_append_fmt(&out, "%03d", st.millisec); break;                                      // time component

                case 'N': ds_append_fmt(&out, "%04d/%02d/%02d", st.year, st.month, st.day); break;              // date component
                case 'Y': ds_append_fmt(&out, "%04d", st.year); break;                                          // date component
                case 'O': ds_append_fmt(&out, "%02d", st.month); break;                                         // date component
                case 'D': ds_append_fmt(&out, "%02d", st.day); break;                                           // date component

                default:                                                                                        // unknown %% - treat literally (append '$' and the char)
                    ds_append_char(&out, '$');
                    ds_append_char(&out, cmd);
                    break;
            }
        } else {
            ds_append_char(&out, c);
        }
    }

    // ensure final message ends with newline
    if (out.len == 0 || out.buf[out.len - 1] != '\n') ds_append_char(&out, '\n');


    // route to stdout or stderr depending on severity
    if (s_log_to_console) {
        if ((int)type < LOG_TYPE_WARN) {
            fputs(out.buf, stdout);
            fflush(stdout);
        } else {
            fputs(out.buf, stderr);
            fflush(stderr);
        }
    }


    const size_t msg_length = strlen(out.buf);
    const size_t remaining_buffer_size = sizeof(s_log_msg_buffer) - strlen(s_log_msg_buffer) -1;

    if (remaining_buffer_size > msg_length)
        strcat(s_log_msg_buffer, out.buf);              // save because ensured size
    else
        flush_log_msg_buffer(out.buf);      // flush all buffered messages and current message


    ds_free(&out);
    pthread_mutex_unlock(&s_general_mutex);
}

//
void log_message(log_type type, u64 thread_id, const char* file_name, const char* function_name, const int line, const char* message, ...) {

    if (strlen(message) == 0) return;           // skip all empty log messages
    
    // Format the user message first (variable args)
    va_list ap;
    va_start(ap, message);

    // create a dynamic local buffer for the user's message
    int needed = vsnprintf(NULL, 0, message, ap);
    va_end(ap);

    dyn_str msg;
    ds_init(&msg);
    if (needed > 0) {
        ds_ensure(&msg, (size_t)needed);
        va_start(ap, message);
        vsnprintf(msg.buf, msg.cap, message, ap);
        msg.len = (size_t)needed;
        msg.buf[msg.len] = '\0';
        va_end(ap);
    }

    // call the formatter that understands s_format_current
    process_log_message_v(type, thread_id, file_name, function_name, line, msg.buf);

    ds_free(&msg);
}
