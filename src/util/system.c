
#include <time.h>
#include <sys/time.h>
#include <libgen.h> 
#include <stdio.h>
#include <unistd.h>   // for readlink
#include <limits.h>   // for PATH_MAX
#include <string.h>   // for strncpy
#include <errno.h>
#include <stdint.h>

#include "system.h"



f64 get_precise_time() {

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);                    // CLOCK_MONOTONIC is steady, not affected by system clock changes
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}


void precise_sleep(const f64 seconds) {

    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Calculate the end time
    struct timespec end = start;
    end.tv_sec += (time_t)seconds;
    end.tv_nsec += (long)((seconds - (time_t)seconds) * 1e9);
    
    // Normalize the nanoseconds field
    if (end.tv_nsec >= 1000000000L) {
        end.tv_sec += end.tv_nsec / 1000000000L;
        end.tv_nsec %= 1000000000L;
    }
    
    // First sleep for most of the time using nanosleep
    struct timespec sleep_time;
    sleep_time.tv_sec = (time_t)seconds;
    sleep_time.tv_nsec = (long)((seconds - (time_t)seconds) * 1e9);
    
    // Subtract a small buffer to ensure we don't oversleep
    const long buffer_ns = 1000000L; // 1 ms buffer
    if (sleep_time.tv_nsec > buffer_ns) {
        sleep_time.tv_nsec -= buffer_ns;
    } else if (sleep_time.tv_sec > 0) {
        sleep_time.tv_sec -= 1;
        sleep_time.tv_nsec = 1000000000L - (buffer_ns - sleep_time.tv_nsec);
    } else {
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 0;
    }
    
    // Perform the initial sleep
    if (sleep_time.tv_sec > 0 || sleep_time.tv_nsec > 0) {
        nanosleep(&sleep_time, NULL);
    }
    
    // Busy-wait for the remaining time
    do {
        clock_gettime(CLOCK_MONOTONIC, &current);
    } while (current.tv_sec < end.tv_sec || (current.tv_sec == end.tv_sec && current.tv_nsec < end.tv_nsec));
}


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



#if 1


static char executable_path[PATH_MAX] = {0};

const char* get_executable_path() {
    if (executable_path[0] != '\0') {
        return executable_path;
    }

    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        perror("readlink");
        return NULL;
    }
    path[len] = '\0';

    char path_copy[PATH_MAX];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char* dir = dirname(path_copy);
    if (dir != NULL) {
        strncpy(executable_path, dir, sizeof(executable_path) - 1);
        executable_path[sizeof(executable_path) - 1] = '\0';
    }

    return executable_path;
}


#else

const char* get_executable_path() {

    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) { 
        perror("readlink");
        return NULL;
    }
    path[len] = '\0'; // add null terminator 
    // dirname may modify the string, so we copy it first
    char dir[PATH_MAX];
    strncpy(dir, path, sizeof(dir));
    return dirname(dir);
}

#endif


int get_executable_path_buf(char *out, size_t outlen) {

    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1)
        return -1;

    path[len] = '\0';
    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    char *d = dirname(tmp);
    if (!d)
        return -1;
        
    strncpy(out, d, outlen);
    out[outlen-1] = '\0';
    return 0;
}
