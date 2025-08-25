
#include <time.h>
#include <sys/time.h>
#include <libgen.h> 
#include <stdio.h>
#include <unistd.h>   // for readlink
#include <limits.h>   // for PATH_MAX
#include <string.h>   // for strncpy

#include "system.h"


system_time get_system_time() {

    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    struct tm tm_local;
    localtime_r(&t, &tm_local);

    system_time out;
    out.year = tm_local.tm_year + 1900;
    out.month = tm_local.tm_mon + 1;
    out.day = tm_local.tm_mday;
    out.hour = tm_local.tm_hour;
    out.minute = tm_local.tm_min;
    out.second = tm_local.tm_sec;
    out.millisec = (int)(tv.tv_usec / 1000);
    return out;
}

//
const char* get_executable_path() {

    static char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        perror("readlink");
        return NULL;
    }
    path[len] = '\0'; // add null terminator

    // dirname may modify the string, so we copy it first
    static char dir[PATH_MAX];
    strncpy(dir, path, sizeof(dir));
    return dirname(dir);
}