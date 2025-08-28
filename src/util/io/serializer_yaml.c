#pragma once

#include <string.h>
#include <regex.h>
#include <limits.h>

#include "util/io/logger.h"

#include "util/io/serializer_yaml.h"



// ============================================================================================================================================
// helper functions
// ============================================================================================================================================

#define STR_LINE_LEN    256

// write indentation
static void write_indentation(FILE* fp, u32 indentation) {
    for (u32 x = 0; x < indentation; x++)
        fputs("  ", fp);
}


// will get a char array like his: [char line[STR_LINE_LEN]]
u32 get_indentation(const char *line) {

    if (!line) return 0;
    u32 count = 0;
    
    // Count spaces and tabs at the beginning
    while (line[count] == ' ' && line[count +1] == ' ' || line[count] == '\t')
        count++;

    return count;
}


//
const char* remove_indentation(const char* line) {
    
    if (!line) return NULL;

    size_t i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    
    return line + i;  // Return pointer past indentation
}


// get all lines that match the section and indentation and save them in [serializer->section_content]
// lines inside [serializer->section_content] are be "\n" terminated
b8 get_content_of_section(serializer_yaml* serializer) {

    // reset string
    ds_free(&serializer->section_content);
    ds_init(&serializer->section_content);

    char line[STR_LINE_LEN] = {0};
    char section_name[64];
    snprintf(section_name, sizeof(section_name), "%s:", serializer->current_section_name);
    b8 found_section = false;
    rewind(serializer->fp);

    // Find currect section (name and indentation)
    while (fgets(line, sizeof(line), serializer->fp)) {
        if (!strstr(line, section_name) || get_indentation(line) != serializer->current_indentation)
            continue;

        found_section = true;
        break;                   // exit search loop -> found header
    }
    VALIDATE(found_section, return false, "", "could not find section ")

    serializer->current_indentation++;      // found section header -> entrys are indented 1 more

    // Prepare regex to match key-value lines
    static const char *pattern = "^[ \t]*[A-Za-z0-9_-]+:[ \t]*.+$";
    regex_t regex;
    VALIDATE(!regcomp(&regex, pattern, REG_EXTENDED), return false, "", "Regex compilation failed")
    
    // pars all lines that come after
    while (fgets(line, sizeof(line), serializer->fp)) {

        const u32 indent = get_indentation(line);
        if (indent < serializer->current_indentation) break;         // stop when section ends
        if (indent > serializer->current_indentation) continue;      // skip any potential subsection

        // check if line looks like this using regex:      <leading indentation><string>: <string>
        if (regexec(&regex, line, 0, NULL, 0) == 0)
            ds_append_str(&serializer->section_content, remove_indentation(line));
    }

    LOG(Info, "serializer->section_content: \n%s", serializer->section_content.data)

    return true;
}


// tries to find a line containing the key, if found it will return true and set [char* line] to the line containing the key
b8 get_value(serializer_yaml* serializer, const char* key, const char* format, void* value) {

    if (!serializer || !key || !format || !value) return false;

    // Get pointers to the start and end of the string data
    char* start = serializer->section_content.data;
    const char* end = start + serializer->section_content.len;
    char* current = start;

    b8 found = false;
    while (current < end) {                                             // Iterate through each line

        const char* newline = memchr(current, '\n', end - current);     // Find the next newline character
        if (newline == NULL)
            newline = end;                                              // Last line might not have a newline
        
        const size_t line_length = newline - current;                   // Calculate line length
        
        // Check if this line starts with our key followed by a colon
        if (line_length > strlen(key) && memcmp(current, key, strlen(key)) == 0 && current[strlen(key)] == ':') {
            
            const char* value_start = current + strlen(key) + 1;                            // Find the position after the colon
            while (value_start < newline && (*value_start == ' ' || *value_start == '\t'))  // Skip any whitespace after the colon
                value_start++;
            
            // Extract the value
            char value_str[256];
            size_t value_len = newline - value_start;
            if (value_len >= sizeof(value_str))
                value_len = sizeof(value_str) - 1;
            
            memcpy(value_str, value_start, value_len);
            value_str[value_len] = '\0';
            
            if (sscanf(value_str, format, value) == 1)          // Parse the value
                return true;
            else
                return false;                                   // Failed to parse value
        }
        
        current = newline + 1;          // Move to next line
    }
    
    return found;
}


// tries to find a line containing the key, if found it will update the value, if not it will append a new line at the end
b8 set_value(serializer_yaml* serializer, const char* key, const char* format, void* value) {
    
    if (!serializer || !key || !format || !value) return false;

    // Get pointers to the start and end of the string data
    char* start = serializer->section_content.data;
    const char* end = start + serializer->section_content.len;
    char* current = start;

    b8 found = false;
    while (current < end) {                                             // Iterate through each line

        const char* newline = memchr(current, '\n', end - current);     // Find the next newline character
        if (newline == NULL) {
            newline = end;                                              // Last line might not have a newline
        }
        
        const size_t line_length = newline - current;                   // Calculate line length
        
        // Check if this line starts with our key followed by a colon
        if (line_length > strlen(key) && memcmp(current, key, strlen(key)) == 0 && current[strlen(key)] == ':') {
            
            char new_line[STR_LINE_LEN];
            snprintf(new_line, sizeof(new_line), "%s: ", key);
            
            char value_str[STR_LINE_LEN];
            snprintf(value_str, sizeof(value_str), format, value);
            strcat(new_line, value_str);                                // Append the formatted value
            
            size_t line_start = current - start;                        // Calculate positions for replacement
            size_t line_end = newline - start;
            
            // Replace the line in the dynamic string
            ds_remove_range(&serializer->section_content, line_start, line_end - line_start);
            ds_insert_str(&serializer->section_content, line_start, new_line);
            
            found = true;
            break;
        }
        
        current = newline + 1;          // Move to next line
    }
    
    if (!found) {
        
        // Key not found - append a new line
        char new_line[STR_LINE_LEN];
        snprintf(new_line, sizeof(new_line), "%s: ", key);
        
        // Append the formatted value
        char value_str[STR_LINE_LEN];
        snprintf(value_str, sizeof(value_str), format, value);

        strcat(new_line, value_str);
        strcat(new_line, "\n");
        ds_append_str(&serializer->section_content, new_line);
    }
    
    return found;
}


// ============================================================================================================================================
// serializer
// ============================================================================================================================================

// Core functions
b8 yaml_serializer_init(serializer_yaml* serializer, const char* file_path, const char* section_name, const serializer_option option) {
    
    ASSERT(file_path != NULL, "", "failed to provide a file path")

    char exec_path[PATH_MAX] = {0};
    get_executable_path_buf(exec_path, sizeof(exec_path));
    ASSERT(exec_path != NULL, "", "FAILED")

    char loc_file_path[PATH_MAX];
    memset(loc_file_path, '\0', sizeof(loc_file_path));
    snprintf(loc_file_path, sizeof(loc_file_path), "%s/%s", exec_path, file_path);

    // if LOAD make sure file exists
    if (option == SERIALIZER_OPTION_LOAD) {

        LOG(Trace, "opening file [%s]", loc_file_path)
        FILE* fp = fopen(loc_file_path, "r");
        VALIDATE(fp, return false, "", "Failed to open file [%s]", loc_file_path)
        fclose(fp);
    }

    const char* mode = (option == SERIALIZER_OPTION_LOAD) ? "r" : "w";
    serializer->fp = fopen(loc_file_path, mode);
    VALIDATE(serializer->fp, return false, "", "Failed to open file [%s]", loc_file_path)

    serializer->option = option;
    strncpy(serializer->current_section_name, section_name, strlen(section_name));
    ds_init(&serializer->section_content);
    get_content_of_section(serializer);

    return true;
}


void yaml_serializer_shutdown(serializer_yaml* serializer) {

    // TODO: need to save this at the right location in the file and remember to save the section_header as well
    // if (serializer->option == SERIALIZER_OPTION_SAVE)           // dump content to file
    //     fputs(&serializer->section_content, serializer->fp);

    // close file
    if (serializer->fp) {
        fclose(serializer->fp);
        serializer->fp = NULL;
    }
    ds_free(&serializer->section_content);

}


// // Entry functions for different types
// // Need value as pointer because value will be overwritten when option = LOAD



void get_entry(serializer_yaml* serializer, const char* key, const char* format, int* value) {


}


#define PARSE_VALUE(format)                                                                                     \
    if (serializer->option == SERIALIZER_OPTION_SAVE)   set_value(serializer, key, format, (void*)value);       \
    else                                                get_value(serializer, key, format, (void*)value);


void yaml_serializer_entry(serializer_yaml* serializer, const char* key, int* value, const char* format)    { PARSE_VALUE(format) }


void yaml_serializer_entry_int(serializer_yaml* serializer, const char* key, int* value)                    { PARSE_VALUE("%d") }


void yaml_serializer_entry_float(serializer_yaml* serializer, const char* key, float* value)                { PARSE_VALUE("%f") }


void yaml_serializer_entry_bool(serializer_yaml* serializer, const char* key, bool* value)                  { PARSE_VALUE("%d") }


void yaml_serializer_entry_string(serializer_yaml* serializer, const char* key, char* value, size_t buffer_size) {

    if (serializer->option == SERIALIZER_OPTION_SAVE) {
        set_value(serializer, key, "%s", (void*)value);
    } else {
        char temp[STR_LINE_LEN] = {0};
        if (get_value(serializer, key, "%s", temp)) {
            strncpy(value, temp, buffer_size);
            value[buffer_size - 1] = '\0'; // Ensure null termination
        }
    }
}


#undef PARSE_VALUE




// // --------------------- DISABLED FOR NOW ---------------------
// // ============================================================================================================================================
// // Subsection function
// // ============================================================================================================================================
//
// void yaml_serializer_subsection_begin(serializer_yaml* serializer, const char* name) {
//
//     ASSERT(strlen(name) < STR_SEC_LEN, "", "Provided section name is to long [%s] may size [%u]", name, STR_SEC_LEN)
//     memcpy(serializer->previous_section_name, serializer->current_section_name, sizeof(serializer->previous_section_name));
//     strncpy(serializer->current_section_name, name, strlen(name));
//     serializer->current_indentation++;
//
//     if (serializer->option == SERIALIZER_OPTION_LOAD)
//         get_content_of_section(serializer);                         // get content of new section
//
// }
//
//
// void yaml_serializer_subsection_end(serializer_yaml* serializer) {
//
//     if (serializer->option == SERIALIZER_OPTION_SAVE)           // dump content to file
//         fputs(&serializer->section_content, serializer->fp);
//      
//     // switch name back to previous
//     char buffer[STR_SEC_LEN];
//     memcpy(buffer, serializer->current_section_name, sizeof(buffer));
//     memcpy(serializer->current_section_name, serializer->previous_section_name, sizeof(buffer));
//     memcpy(serializer->previous_section_name, buffer, sizeof(buffer));
//     serializer->current_indentation--;
//
//     if (serializer->option == SERIALIZER_OPTION_LOAD)
//         get_content_of_section(serializer);                         // get content of new section
//
// }
// // ------------------------------------------------------------
