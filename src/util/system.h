#pragma once

#include "util/data_structure/data_types.h"

typedef struct {
    i16 year, month, day;
    i16 hour, minute, second;
    i16 millisec;
} system_time;


//
system_time get_system_time();

//
const char* get_executable_path();

