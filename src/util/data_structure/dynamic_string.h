
#include "util/data_structure/data_types.h"

typedef struct {
    char*   buf;
    size_t  len;
    size_t  cap;
} dyn_str;


//
void ds_init(dyn_str* s);

//
void ds_free(dyn_str* s);

//
void ds_ensure(dyn_str* s, size_t extra);

//
void ds_append_str(dyn_str* s, const char* text);

//
void ds_append_char(dyn_str* s, char c);

//
void ds_append_fmt(dyn_str* s, const char* fmt, ...);
