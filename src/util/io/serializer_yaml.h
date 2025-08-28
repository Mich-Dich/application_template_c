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


// --------------------- DISABLED FOR NOW ---------------------
// void yaml_serializer_entry_float(serializer_yaml* serializer, const char* key, float* value);
// void yaml_serializer_entry_bool(serializer_yaml* serializer, const char* key, bool* value);
// void yaml_serializer_entry_string(serializer_yaml* serializer, const char* key, const char* value);

// // Subsection function
// void yaml_serializer_subsection_begin(serializer_yaml* serializer, const char* name);
// void yaml_serializer_subsection_end(serializer_yaml* serializer);
// ------------------------------------------------------------




// // usage example:
// {

//     #include "util/data_structure/data_types.h"
//     #include "util/io/logger.h"
//     #include "util/io/serializer_yaml.h"
    
//     f32 test_float = 3.14;
//     i32 test_i32 = 42;
//     const char* test_str = "test_str";
//     const char* test_str_sub = "test_str_sub";
    
//     b8 serialize(const serializer_option option) {

//         serializer_yaml sy;
//         VALIDATE(yaml_serializer_init(&sy, "./config/test.yml", "main_section", SERIALIZER_OPTION_SAVE), return false, "", "");
//         yaml_serializer_entry_int(&sy, KEA_VALUE(test_i32));
//         yaml_serializer_entry_float(&sy, KEA_VALUE(test_float));
//         yaml_serializer_entry_string(&sy, KEA_VALUE(test_str));
//         yaml_serializer_subsection_begin(&sy, "test_subsection");
//         yaml_serializer_entry_string(&sy, "test_str_sub", &test_str_sub);
//         yaml_serializer_subsection_end(&sy);
//         yaml_serializer_shutdown(&sy);
//     }
    
// }
