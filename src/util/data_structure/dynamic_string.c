
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdint.h>

#include "dynamic_string.h"


void ds_init(dyn_str* s) {
    
    s->cap = 4096;
    s->data = malloc(s->cap);
    s->len = 0;
    if (s->data) s->data[0] = '\0';
}


void ds_init_s(dyn_str* s, size_t needed_size) {
    
    s->cap = needed_size + 64;      // use provided size + small safety buffer
    s->data = malloc(s->cap);
    s->len = 0;
    if (s->data) s->data[0] = '\0';
}


void ds_free(dyn_str* s) {
    
    free(s->data);
    s->data = NULL;
    s->len = s->cap = 0;
}


void ds_ensure(dyn_str* s, size_t extra) {
    
    if (!s->data)
        ds_init(s);
    
    const size_t need = s->len + extra + 1;
    if (need > s->cap) {
        while (s->cap < need)
            s->cap *= 2;

        s->data = realloc(s->data, s->cap);
    }
}


void ds_append_str(dyn_str* s, const char* text) {
    
    if (!text)
        return;
    
    const size_t tlen = strlen(text);
    ds_ensure(s, tlen);
    memcpy(s->data + s->len, text, tlen);
    s->len += tlen;
    s->data[s->len] = '\0';
}


void ds_append_char(dyn_str* s, char c) {

    ds_ensure(s, 1);
    s->data[s->len++] = c;
    s->data[s->len] = '\0';
}


void ds_append_fmt(dyn_str* s, const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    // first determine required size
    va_list ap2;
    va_copy(ap2, ap);
    const int needed = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (needed > 0) {
        ds_ensure(s, (size_t)needed);
        vsnprintf(s->data + s->len, s->cap - s->len, fmt, ap);
        s->len += (size_t)needed;
    }
    va_end(ap);
}


void ds_iterate_lines(const dyn_str *ds, void (*callback)(const char *line, size_t len, void *user_data), void *user_data) {

    const char *start = ds->data;
    const char *end = ds->data + ds->len;
    while (start < end) {

        const char *newline = memchr(start, '\n', end - start);
        const size_t line_len = (newline)? (size_t)(newline - start) : (size_t)(end - start);
        callback(start, line_len, user_data);                   // Call the callback with the line
        start = newline ? newline + 1 : end;                    // Move to the next line (skip the newline if present)
    }
}


void ds_remove_range(dyn_str* ds, size_t pos, size_t len) {

    if (pos >= ds->len) return;
    if (pos + len > ds->len) len = ds->len - pos;
    
    memmove(ds->data + pos, ds->data + pos + len, ds->len - pos - len + 1);
    ds->len -= len;
}


void ds_insert_str(dyn_str* ds, size_t pos, const char* str) {

    const size_t str_len = strlen(str);
    if (ds->len + str_len >= ds->cap) {                 // Ensure enough capacity
        ds->cap = (ds->len + str_len) * 2;
        ds->data = realloc(ds->data, ds->cap);
    }
    
    memmove(ds->data + pos + str_len, ds->data + pos, ds->len - pos + 1);       // Make space for the new string
    memcpy(ds->data + pos, str, str_len);                                       // Copy the new string
    ds->len += str_len;
}
