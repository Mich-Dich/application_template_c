
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
    s->buf = malloc(s->cap);
    s->len = 0;
    if (s->buf) s->buf[0] = '\0';
}


void ds_init_s(dyn_str* s, size_t needed_size) {
    
    s->cap = needed_size + 64;      // use provided size + small safety buffer
    s->buf = malloc(s->cap);
    s->len = 0;
    if (s->buf) s->buf[0] = '\0';
}


void ds_free(dyn_str* s) {
    
    free(s->buf);
    s->buf = NULL;
    s->len = s->cap = 0;
}


void ds_ensure(dyn_str* s, size_t extra) {
    
    if (!s->buf)
        ds_init(s);
    
    size_t need = s->len + extra + 1;
    if (need > s->cap) {
        while (s->cap < need)
            s->cap *= 2;

        s->buf = realloc(s->buf, s->cap);
    }
}


void ds_append_str(dyn_str* s, const char* text) {
    
    if (!text)
        return;
    
    size_t tlen = strlen(text);
    ds_ensure(s, tlen);
    memcpy(s->buf + s->len, text, tlen);
    s->len += tlen;
    s->buf[s->len] = '\0';
}


void ds_append_char(dyn_str* s, char c) {

    ds_ensure(s, 1);
    s->buf[s->len++] = c;
    s->buf[s->len] = '\0';
}


void ds_append_fmt(dyn_str* s, const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    // first determine required size
    va_list ap2;
    va_copy(ap2, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (needed > 0) {
        ds_ensure(s, (size_t)needed);
        vsnprintf(s->buf + s->len, s->cap - s->len, fmt, ap);
        s->len += (size_t)needed;
    }
    va_end(ap);
}
