#pragma once

#include <stdio.h>

#include "util/data_structure/data_types.h"
#include "util/data_structure/dynamic_string.h"
#include "util/util.h"


typedef enum {
    SERIALIZER_OPTION_SAVE = 0,
    SERIALIZER_OPTION_LOAD,
} serializer_option;


#define STR_SEC_LEN     128

typedef struct {

    FILE*               fp;
    serializer_option   option;
    u32                 current_indentation;
    char                current_section_name[STR_SEC_LEN];
    char                previous_section_name[STR_SEC_LEN];
    dyn_str             section_content;

} serializer_yaml;


#define KEA_VALUE(variable)         util_extract_variable_name(#variable), &variable


// Core functions
b8 yaml_serializer_init(serializer_yaml* sy, const char* file_path, const char* section_name, const serializer_option option);
void yaml_serializer_shutdown(serializer_yaml* sy);

// Entry functions for different types
// Need value as pointer because value will be overwritten when option = LOAD

void yaml_serializer_entry_int(serializer_yaml* serializer, const char* key, int* value);
void yaml_serializer_entry_float(serializer_yaml* serializer, const char* key, float* value);
void yaml_serializer_entry_bool(serializer_yaml* serializer, const char* key, bool* value);
void yaml_serializer_entry_string(serializer_yaml* serializer, const char* key, char* value, size_t buffer_size);

// Generic version, user needs to define how he want to save the values
void yaml_serializer_entry(serializer_yaml* serializer, const char* key, int* value, const char* format);

// --------------------- DISABLED FOR NOW ---------------------
// // Subsection function
// void yaml_serializer_subsection_begin(serializer_yaml* serializer, const char* name);
// void yaml_serializer_subsection_end(serializer_yaml* serializer);
// ------------------------------------------------------------
