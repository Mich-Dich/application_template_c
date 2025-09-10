
#include <errno.h>
#include <string.h>
#include <regex.h>
#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "util/io/logger.h"
#include "util/util.h"
#include "util/system.h"

#include "util/io/serializer_yaml.h"



// ============================================================================================================================================
// helper functions
// ============================================================================================================================================

#define STR_LINE_LEN    32000               // !!! longest possible length for a line in YAML file !!!

// write indentation
static void write_indentation(FILE* fp, u32 indentation) {
    for (u32 x = 0; x < indentation; x++)
        fputs("  ", fp);
}


// will get a char array like his: [char line[STR_LINE_LEN]]
u32 get_indentation(const char *line) {

    if (!line) return 0;
    u32 count = 0;
    u32 pointer = 0;
    
    while ((line[pointer] == ' ' && line[pointer +1] == ' ') || line[count] == '\t') {

        pointer += (line[count] == '\t') ? 1 : 2 ;          // shift pointer by 2 if spaces
        count++;
    }

    return count;
}


//
const char* skip_indentation(const char* line) {
    
    if (!line) return NULL;

    size_t i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    
    return line + i;  // Return pointer past indentation
}


// ============================================================================================================================================
// section handling
// ============================================================================================================================================

// get all lines that match the section and indentation and save them in [serializer->section_content]
// lines inside [serializer->section_content] are be "\n" terminated
b8 get_content_of_section(serializer_yaml* serializer) {

    // reset string
    ds_free(&serializer->section_content);
    ds_init(&serializer->section_content);

    char line[STR_LINE_LEN] = {0};
    char section_name[STR_LINE_LEN] = {0};
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
    static const char *pattern = "^[ \t]*[A-Za-z0-9_-]+:[ \t]*[^ \t\n]+.*$";
    regex_t regex;
    VALIDATE(!regcomp(&regex, pattern, REG_EXTENDED), return false, "", "Regex compilation failed")
    
    // pars all lines that come after
    while (fgets(line, sizeof(line), serializer->fp)) {

        const u32 indent = get_indentation(line);
        if (indent < serializer->current_indentation) break;         // stop when section ends
        if (indent > serializer->current_indentation) continue;      // skip any potential subsection

        // check if line looks like this using regex:      <leading indentation><string>: <string>
        if (regexec(&regex, line, 0, NULL, 0) == 0)
            ds_append_str(&serializer->section_content, skip_indentation(line));
    }
    regfree(&regex); // Don't forget to free the regex

    LOG(Info, "serializer->section_content: \n%s", serializer->section_content.data)

    return true;
}


typedef struct {
    serializer_yaml*    serializer;
    const char*         start;
    const char*         end;
    dyn_str*            file_content;
} serializer_section_data;

b8 find_section_start_and_end_callback(const char* line, size_t len, void* user_data) {

    serializer_section_data* sec_data = (serializer_section_data*)user_data;
    LOG(Trace, "line: %.*s\n", (int)len, line);

    if (!sec_data->start) {         // Find start first: currect section (name and indentation)

        const char*     needle = sec_data->serializer->current_section_name;
        size_t          needle_len = strlen(needle);
        b8              found = false;
        if (needle_len <= len) {
            for (size_t x = 0; x < len - needle_len; x++) {
                if (memcmp(line + x, needle, needle_len) == 0) {

                    found = true;
                    break;
                }
            }
        }

        //  check indentation (remove 1 for header)                                    correct title
        if (get_indentation(line) == (sec_data->serializer->current_indentation -1) && found) {

            sec_data->start = line + len;        // start pointer at next line (ignore header)
            LOG(Debug, "found start")
        }
        
        return true;       // always return true because ds_iterate_lines still need to continue until end if found
    } 

    // Find end of section
    if (get_indentation(line) < sec_data->serializer->current_indentation || line[len -1] == ":") {
        sec_data->end = line -1;
        LOG(Debug, "found end")
        return false;
    }

    return true;
}

#if 0

b8 add_or_update_entry(const char* line, size_t len, void* user_data) {

    serializer_section_data* sec_data = (serializer_section_data*)user_data;

    const char* unindented_line = skip_indentation(line);
    LOG(Debug, "line: [%*s]", len, line)
    
    // get key
    char key_str[PATH_MAX] = {0};
    for (size_t x = 0; x < len; x++) {
        if (line[x] == ':')
            break;

        key_str[x] = line[x];
    }
    LOG(Trace, "key_str: %s", key_str)

    // search for key in sec_data->file_content  between:  sec_data.start and sec_data.end
    const size_t end_position = (sec_data->end - sec_data->file_content->data);
    const ssize_t key_location_in_file = ds_find_str(&sec_data->file_content, &key_str, line);
    if (key_location_in_file == -1 || key_location_in_file >= end_position) {          // key not in current section, append to end
        
        memset(key_str, 0, sizeof(key_str));        // can reuse key_str as it's not needed anymore
        memcpy(&key_str, line, len);
        ds_insert_str(sec_data->file_content, end_position, key_str);                       // copy <line> to sec_data->file_content after sec_data.end
        sec_data->end += len;                                                           // move sec_data.end to new end
        return true;
    }

    // key_location_in_file is found and needs to be updated
    const size_t key_len = strlen(key_str);
    char value_str[PATH_MAX] = {0};
    for (size_t x = key_len +2; x < len; x++)          // start after key_str (skip ":" as well)
        value_str[x - key_len -2] = line[x];
    LOG(Trace, "value_str: %s", value_str)
      
    const size_t value_start_pos = key_location_in_file + key_len +2;
    const result = ds_replace_range(sec_data->file_content, value_start_pos, len - key_len -2, value_str);
    VALIDATE(!result, , "", "ds_replace_range result: %s", strerrno(result))

    LOG(Debug, "line: [%s]", sec_data->file_content->data)
    return true;
}

#else

b8 add_or_update_entry(const char* line, size_t len, void* user_data) {
    serializer_section_data* sec_data = (serializer_section_data*)user_data;

    const char* unindented_line = skip_indentation(line);
    // LOG(Debug, "line: [%.*s]", (int)len, line)
    
    // Get key
    char key_str[PATH_MAX] = {0};
    size_t key_len = 0;
    for (size_t x = 0; x < len; x++) {
        if (line[x] == ':')
            break;
        key_str[x] = line[x];
        key_len++;
    }
    // LOG(Trace, "key_str: %s", key_str)

    // Search for key in the section range
    const size_t end_position = (sec_data->end - sec_data->file_content->data);
    ssize_t key_location_in_file = -1;
    
    // Search only within the section bounds
    const char* section_start = sec_data->file_content->data + (sec_data->start - sec_data->file_content->data);
    const char* section_end = sec_data->end;
    
    char* current_pos = (char*)section_start;
    while (current_pos < section_end) {
        char* found = strstr(current_pos, key_str);
        if (!found || found >= section_end) break;
        
        // Check if this is a complete key (followed by colon)
        if (found[key_len] == ':') {
            key_location_in_file = found - sec_data->file_content->data;
            break;
        }
        current_pos = found + key_len;
    }

    if (key_location_in_file == -1) {
        // Key not found, append to end of section
        char new_line[PATH_MAX];
        snprintf(new_line, sizeof(new_line), "%.*s\n", (int)len, line);
        ds_insert_str(sec_data->file_content, end_position, new_line);
        sec_data->end += strlen(new_line);
        return true;
    }

    // Key found, update the value
    const char* value_start = strchr(line, ':');
    if (!value_start) return true;
    value_start++; // Skip colon
    
    // Skip whitespace after colon
    while (*value_start == ' ' || *value_start == '\t') {
        value_start++;
        len--;
    }
    
    size_t value_len = len - (value_start - line) +1;
    char value_str[PATH_MAX] = {0};
    strncpy(value_str, value_start, value_len);
    
    // Find the value position in the file
    char* file_value_start = strchr(sec_data->file_content->data + key_location_in_file, ':');
    if (!file_value_start) return true;
    file_value_start++; // Skip colon
    
    // Skip whitespace
    while (*file_value_start == ' ' || *file_value_start == '\t')
        file_value_start++;
    
    // Find end of current value (newline or end of string)
    char* file_value_end = strchr(file_value_start, '\n');
    if (!file_value_end) file_value_end = sec_data->file_content->data + sec_data->file_content->len;
    
    size_t file_value_pos = file_value_start - sec_data->file_content->data;
    size_t file_value_len = file_value_end - file_value_start;
    
    i32 result = ds_replace_range(sec_data->file_content, file_value_pos, file_value_len, value_str);
    if (result != AT_SUCCESS)
        LOG(Error, "ds_replace_range failed: %d", result)

    return true;
}

#endif


void save_section(serializer_yaml* serializer) {

    // load entire file content into dyn_str
    dyn_str file_content = {0};
    const i32 result = ds_from_file(&file_content, serializer->fp);
    VALIDATE(!result, return, "", "Error reading file: %d", result)

    LOG(Info, "file_content: \n%s\n", file_content.data)
    LOG(Info, "section_content: \n%s\n", serializer->section_content.data)

    serializer_section_data sec_data = {0};
    sec_data.serializer = serializer;
    ds_iterate_lines(&file_content, find_section_start_and_end_callback, (void*)&sec_data);        // find section start & end in file content
    if (sec_data.start && !sec_data.end) {              // start found but not end -> assuming section is at end file
        LOG(Trace, "start found, but NOT end")
        sec_data.end = file_content.data[file_content.len -1];

    } else if (!sec_data.start && !sec_data.end) {      // both not found -> section not in file yet
        LOG(Trace, "both start & end NOT found")
        sec_data.start = file_content.data[file_content.len -1];
        sec_data.end = file_content.data[file_content.len -1];
    } else if (sec_data.start && sec_data.end)
        LOG(Trace, "both start & end FOUND")

    // LOG(Info, "start:   %*s", strlen(sec_data.start), sec_data.start)
    // LOG(Info, "end:     %*s", strlen(sec_data.end), sec_data.end)   

    LOG(Warn, "before: %s", file_content)
    sec_data.file_content = &file_content;
    ds_iterate_lines(&serializer->section_content, add_or_update_entry, (void*)&sec_data);
    LOG(Warn, "after: %s", file_content)


    // TODO: save data to file


    ds_free(&file_content);
    LOG(Debug, "Saved section")
}

// ============================================================================================================================================
// value parsing
// ============================================================================================================================================

typedef struct {
    const char*     key;
    const char*     format;
    void*           value;
    b8              found;
    dyn_str*        section_content;
} ds_iterator_data;


// ================================= set value =================================

// 
b8 set_value_callback(const char* line, size_t len, void *user_data) {

    ds_iterator_data* loc_data = (ds_iterator_data*)user_data;

    const size_t key_len = strlen(loc_data->key);
    if (len <= key_len || memcmp(line, loc_data->key, key_len) != 0 || line[key_len] != ':')        // Check if this line starts with target key followed by a colon
        return true;

    // LOG(Trace, "found [%s] in [%.*s]", loc_data->key, len, line)

    size_t value_len = len - key_len;                                            // Calculate the full length of the line (including potential newline)
    size_t offset = (line - loc_data->section_content->data) + key_len;         // Calculate the offset of this line in the dynamic string
    while (offset < loc_data->section_content->len && (loc_data->section_content->data[offset] == ':' || loc_data->section_content->data[offset] == ' ')) {

        offset++;       // Include trailing ":" and space
        value_len--;
    }
    
    // Handle different types appropriately
    char value_str[STR_LINE_LEN] = {0};
    const char* format = loc_data->format;

    // Handle different format types
    if (strcmp(format, "%d") == 0)          snprintf(value_str, sizeof(value_str), format, *(int*)loc_data->value);
    else if (strcmp(format, "%u") == 0)     snprintf(value_str, sizeof(value_str), format, *(unsigned int*)loc_data->value);
    else if (strcmp(format, "%ld") == 0)    snprintf(value_str, sizeof(value_str), format, *(long*)loc_data->value);
    else if (strcmp(format, "%lu") == 0)    snprintf(value_str, sizeof(value_str), format, *(unsigned long*)loc_data->value);
    else if (strcmp(format, "%lld") == 0)   snprintf(value_str, sizeof(value_str), format, *(long long*)loc_data->value);
    else if (strcmp(format, "%llu") == 0)   snprintf(value_str, sizeof(value_str), format, *(unsigned long long*)loc_data->value);
    else if (strcmp(format, "%f") == 0)     snprintf(value_str, sizeof(value_str), format, *(float*)loc_data->value);
    else if (strcmp(format, "%lf") == 0)    snprintf(value_str, sizeof(value_str), format, *(double*)loc_data->value);
    else if (strcmp(format, "%Lf") == 0)    snprintf(value_str, sizeof(value_str), format, *(long double*)loc_data->value);
    else if (strcmp(format, "%s") == 0)     snprintf(value_str, sizeof(value_str), format, (char*)loc_data->value);
    else                                    snprintf(value_str, sizeof(value_str), format, *(handle*)loc_data->value);

    // Replace the old value strinf inside the line with the new one
    ds_remove_range(loc_data->section_content, offset, value_len);
    ds_insert_str(loc_data->section_content, offset, value_str);
    
    // LOG(Trace, "section_content [\n%s]", loc_data->section_content->data)
    loc_data->found = true;
    return false;
}

// tries to find a line containing the key, if found it will update the value, if not it will append a new line at the end
b8 set_value(serializer_yaml* serializer, const char* key, const char* format, void* value) {
    
    if (!serializer || !key || !format || !value) return false;

    ds_iterator_data loc_data;
    loc_data.key = key;
    loc_data.format = format;
    loc_data.value = value;
    loc_data.found = false;
    loc_data.section_content = &serializer->section_content;

    // LOG(Trace, "serializer->section_content [%s]", serializer->section_content.data)
    ds_iterate_lines(&serializer->section_content, set_value_callback, (void*)&loc_data);
    
    if (!loc_data.found) {                      // append new line at end if not found
        
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
    return loc_data.found;
}

// ================================= get value =================================

b8 get_value_callback(const char* line, size_t len, void *user_data) {

    ds_iterator_data* loc_data = (ds_iterator_data*)user_data;

    // Check if this line starts with target key followed by a colon
    if (len <= strlen(loc_data->key) || memcmp(line, loc_data->key, strlen(loc_data->key)) != 0 || line[strlen(loc_data->key)] != ':')
        return true;
        
    // LOG(Trace, " pointer [%p] searching for [%s] in [%.*s]", line, loc_data->key, (int)(strchr(line, '\n') - line), line)
        
    const char* value_start = line + strlen(loc_data->key) + 1;                     // Find the position after the colon
    while (value_start < line || (*value_start == ' ' || *value_start == '\t'))     // Skip any whitespace after the colon
        value_start++;
    
    // Extract the value
    char value_str[STR_LINE_LEN] = {0};
    size_t value_len = len - (value_start - line);
    if (value_len >= sizeof(value_str))                 // force length to be shorter than buffer
        value_len = sizeof(value_str) - 1;

    memcpy(value_str, value_start, value_len);
    value_str[value_len] = '\0';
    
    loc_data->found = sscanf(value_str, loc_data->format, loc_data->value) == 1;          // Parse the value
    return !loc_data->found;
}

// tries to find a line containing the key, if found it will return true and set [char* line] to the line containing the key
b8 get_value(serializer_yaml* serializer, const char* key, const char* format, handle* value) {

    if (!serializer || !key || !format || !value) return false;

    ds_iterator_data loc_data;
    loc_data.key = key;
    loc_data.format = format;
    loc_data.value = value;
    loc_data.found = false;
    loc_data.section_content = NULL;

    ds_iterate_lines(&serializer->section_content, get_value_callback, (void*)&loc_data);
    return loc_data.found;
}


// ============================================================================================================================================
// serializer
// ============================================================================================================================================


// Core functions
b8 yaml_serializer_init(serializer_yaml* serializer, const char* dir_path, const char* file_name, const char* section_name, const serializer_option option) {
    
    ASSERT(dir_path != NULL, "", "failed to provide a directory path");
    ASSERT(file_name != NULL, "", "failed to provide a file name");
    ASSERT(section_name != NULL, "", "failed to provide a section name");

    if (!system_ensure_directory_exists(dir_path)) {
        LOG(Error, "failed to create directory: %s\n", dir_path);
        return false;
    }

    char loc_file_path[PATH_MAX];
    memset(loc_file_path, 0, sizeof(loc_file_path));


    // Build full path safely: dir_path + "/" + file_name
    size_t dir_len = strlen(dir_path);
    size_t file_len = strlen(file_name);

    if (dir_len + 1 + file_len + 1 > sizeof(loc_file_path)) {
        LOG(Error, "Path too long: %s/%s\n", dir_path, file_name);
        return false;
    }

    const int written = snprintf(loc_file_path, sizeof(loc_file_path), "%s/%s", dir_path, file_name);           // Use snprintf safely

    if (written < 0 || (size_t)written >= sizeof(loc_file_path)) {
        LOG(Error, "Path too long or snprintf failed: %s/%s\n", dir_path, file_name);
        return false;
    }

    system_ensure_file_exists(loc_file_path);

    serializer->fp = fopen(loc_file_path, "a+");                                                                 // Open file for reading (saving will happen later in shutdown)
    VALIDATE(serializer->fp, return false, "opened file [%s]", "Failed to open file [%s]", loc_file_path);

    serializer->option = option;                                                                                // Store serializer settings

    strncpy(serializer->current_section_name, section_name, sizeof(serializer->current_section_name) - 1);      // Copy section name safely
    serializer->current_section_name[sizeof(serializer->current_section_name) - 1] = '\0';

    ds_init(&serializer->section_content);                                                                      // Initialize dynamic string buffer and parse initial section
    get_content_of_section(serializer);

    return true;
}


void yaml_serializer_shutdown(serializer_yaml* serializer) {

    // LOG(Info, "serializer->section_content: \n%s", serializer->section_content.data)
    
    // TODO: need to save this at the right location in the file and remember to save the section_header as well
    if (serializer->option == SERIALIZER_OPTION_SAVE)           // dump content to file
        save_section(serializer);

    // close file
    if (serializer->fp) {
        fclose(serializer->fp);
        serializer->fp = NULL;
    }
    ds_free(&serializer->section_content);
}


// ============================================================================================================================================
// Subsection function
// ============================================================================================================================================

void yaml_serializer_subsection_begin(serializer_yaml* serializer, const char* name) {

    ASSERT(strlen(name) < STR_SEC_LEN, "", "Provided section name is to long [%s] may size [%u]", name, STR_SEC_LEN)
    memcpy(serializer->previous_section_name, serializer->current_section_name, sizeof(serializer->previous_section_name));
    memset(serializer->current_section_name, '\0', sizeof(serializer->current_section_name));
    strncpy(serializer->current_section_name, name, strlen(name));

    if (serializer->option == SERIALIZER_OPTION_SAVE) {
        // TODO: need to save content in [serializer->section_content] to the correct location
        // !!! can be in the middle of the file !!!
    }

    get_content_of_section(serializer);                         // get content of new section
}


void yaml_serializer_subsection_end(serializer_yaml* serializer) {

    // switch name back to previous
    char buffer[STR_SEC_LEN];
    memcpy(buffer, serializer->current_section_name, sizeof(buffer));
    memcpy(serializer->current_section_name, serializer->previous_section_name, sizeof(buffer));
    memcpy(serializer->previous_section_name, buffer, sizeof(buffer));
    serializer->current_indentation--;

    if (serializer->option == SERIALIZER_OPTION_SAVE) {
        // TODO: need to save content in [serializer->section_content] to the correct location
        // !!! can be in the middle of the file !!!
    }

    get_content_of_section(serializer);                         // get content of new section
}


// ============================================================================================================================================
// serializer entry functions
// ============================================================================================================================================

#define PARSE_VALUE(format)                                                                                     \
    if (serializer->option == SERIALIZER_OPTION_SAVE)   set_value(serializer, key, format, (void*)value);       \
    else                                                get_value(serializer, key, format, (void*)value);

void yaml_serializer_entry(serializer_yaml* serializer, const char* key, void* value, const char* format)     { PARSE_VALUE(format) }

void yaml_serializer_entry_int(serializer_yaml* serializer, const char* key, int* value)                        { PARSE_VALUE("%d") }

void yaml_serializer_entry_f32(serializer_yaml* serializer, const char* key, f32* value)                        { PARSE_VALUE("%f") }

void yaml_serializer_entry_b32(serializer_yaml* serializer, const char* key, b32* value)                        { PARSE_VALUE("%u") }

void yaml_serializer_entry_str(serializer_yaml* serializer, const char* key, char* value, size_t buffer_size)   {

    if (serializer->option == SERIALIZER_OPTION_SAVE) {
        set_value(serializer, key, "%s", (void*)value);
    } else {
        char temp[STR_LINE_LEN] = {0};
        if (get_value(serializer, key, "%s", (handle*)&temp)) {
            strncpy(value, temp, buffer_size);
            value[buffer_size - 1] = '\0'; // Ensure null termination
        }
    }
}

#undef PARSE_VALUE

