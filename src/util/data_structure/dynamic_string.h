
#include "util/data_structure/data_types.h"

typedef struct {
    char*   buf;
    size_t  len;
    size_t  cap;
} dyn_str;



// @brief Initializes a dynamic string structure with a default capacity.
//        Allocates an initial buffer and sets length to zero.
// @param s Pointer to the dynamic string structure to initialize.
void ds_init(dyn_str* s);


// @brief Initializes a dynamic string structure with the provided capacity.
//        Allocates an initial buffer and sets length to zero.
// @param s Pointer to the dynamic string structure to initialize.
// @param needed_size capacity needed for string
void ds_init_s(dyn_str* s, size_t needed_size);


// @brief Frees the memory used by a dynamic string and resets its state.
// @param s Pointer to the dynamic string structure to free.
void ds_free(dyn_str* s);


// @brief Ensures that the dynamic string has enough capacity to hold
//        the specified additional number of characters. If necessary,
//        the buffer is reallocated with a larger capacity.
// @param s Pointer to the dynamic string structure.
// @param extra The number of additional bytes needed (not including the null terminator).
void ds_ensure(dyn_str* s, size_t extra);


// @brief Appends a null-terminated string to the dynamic string buffer.
//        Automatically resizes the buffer if necessary.
// @param s Pointer to the dynamic string structure.
// @param text The null-terminated string to append. Ignored if NULL.
void ds_append_str(dyn_str* s, const char* text);


// @brief Appends a single character to the dynamic string buffer.
//        Automatically resizes the buffer if necessary.
// @param s Pointer to the dynamic string structure.
// @param c The character to append.
void ds_append_char(dyn_str* s, char c);


// @brief Appends a formatted string to the dynamic string buffer.
//        Works like `printf`, automatically resizing the buffer as needed.
// @param s Pointer to the dynamic string structure.
// @param fmt The format string (printf-style).
// @param ... Additional arguments corresponding to the format specifiers.
void ds_append_fmt(dyn_str* s, const char* fmt, ...);
