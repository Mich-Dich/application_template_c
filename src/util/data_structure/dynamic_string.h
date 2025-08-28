
#include "util/data_structure/data_types.h"

typedef struct {
    char*   data;   // Pointer to the dynamically allocated string buffer.
    size_t  len;    // Current length of the string (excluding null terminator).
    size_t  cap;    // Allocated capacity of the buffer.
} dyn_str;



// @brief Initializes a dynamic string to an empty state.
//        Allocates no memory initially; the string will start with len = 0.
void ds_init(dyn_str* s);


// @brief Initializes a dynamic string with a preallocated buffer.
// @param needed_size The minimum initial capacity to allocate.
void ds_init_s(dyn_str* s, size_t needed_size);


// @brief Frees the memory used by the dynamic string.
//        After this call, the string will be in an uninitialized state.
void ds_free(dyn_str* s);


// @brief Ensures that the dynamic string has enough capacity to hold
//        at least `extra` more characters beyond its current length.
//        If necessary, reallocates the internal buffer.
void ds_ensure(dyn_str* s, size_t extra);


// @brief Appends a C-string to the end of the dynamic string.
// @param text The null-terminated string to append.
void ds_append_str(dyn_str* s, const char* text);


// @brief Appends a single character to the dynamic string.
void ds_append_char(dyn_str* s, char c);


// @brief Appends a formatted string to the dynamic string.
//        Works like printf-style formatting.
// @param fmt A format string (printf-style).
// @param ... The arguments to format.
void ds_append_fmt(dyn_str* s, const char* fmt, ...);


// @brief Iterates over lines in a dyn_str.
// @param ds   Pointer to the dyn_str.
// @param callback  Function called for each line, receives (line_start, line_length, user_data).
void ds_iterate_lines(const dyn_str *ds, void (*callback)(const char *line, size_t len, void *user_data), void *user_data);


void ds_remove_range(dyn_str* ds, size_t pos, size_t len);


void ds_insert_str(dyn_str* ds, size_t pos, const char* str);
