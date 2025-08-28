#pragma once

#include <string.h>
#include <regex.h>

#include "util/io/logger.h"

#include "util/io/serializer_yaml.h"



// ============================================================================================================================================
// helper functions
// ============================================================================================================================================


// write indentation
static void write_indentation(FILE* fp, u32 indentation) {
    for (u32 x = 0; x < indentation; x++)
        fputs("  ", fp);
}

// will get a char array like his: [char line[256]]
u32 get_indentation(const char *line) {

    if (!line) return 0;
    u32 count = 0;
    while (line[count] == ' ' || line[count] == '\t')       // Count spaces and tabs at the beginning
        count++;

    return count;
}


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

    char line[256];
    char section_name[64];
    snprintf(section_name, sizeof(section_name), "%s:", serializer->current_section_name);
    b8 found_section = false;
    rewind(serializer->fp);

    // Find currect section (name and indentation)
    while (fgets(line, sizeof(line), serializer->fp)) {
        if (strstr(line, section_name) && get_indentation(line) == serializer->current_indentation) {
            found_section = true;
            break;                   // exit search loop -> found header
        }
    }
    VALIDATE(found_section, return false, "", "could not find section ")

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

    return true;
}


// tries to find a line containing the key, if found it will return true and set [char* line] to the line containing the key
b8 find_key(serializer_yaml* serializer, const char* key, char* line) {

    /*
    
    while () {      // get line from serializer->section_content

        if ()
    
    }

    */
}


// ============================================================================================================================================
// serializer
// ============================================================================================================================================

// Core functions
b8 yaml_serializer_init(serializer_yaml* serializer, const char* file_path, const char* section_name, const serializer_option option) {
    
    // if LOAD make sure file exists
    if (option == SERIALIZER_OPTION_LOAD) {

        FILE* fp = fopen(file_path, "r");
        VALIDATE(fp, return false, "", "Failed to open file [%s]", file_path)
        fclose(fp);
    }

    const char* mode = (option == SERIALIZER_OPTION_LOAD) ? "r" : "w";
    serializer->fp = fopen(file_path, mode);
    VALIDATE(serializer->fp, return false, "", "Failed to open file [%s]", file_path)

    serializer->option = option;
    strncpy(serializer->current_section_name, section_name, strlen(section_name));
    ds_init(&serializer->section_content);

    if (option == SERIALIZER_OPTION_SAVE)
        ds_append_fmt(&serializer->section_content, "%s:\n", section_name);         // buffer section name 

    if (option == SERIALIZER_OPTION_LOAD)
        get_content_of_section(serializer);

    return true;
}

void yaml_serializer_shutdown(serializer_yaml* serializer) {

    if (serializer->option == SERIALIZER_OPTION_SAVE)           // dump content to file
        fputs(&serializer->section_content, serializer->fp);
        

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



void yaml_serializer_entry_int(serializer_yaml* serializer, const char* key, int* value) {

    if (serializer->option == SERIALIZER_OPTION_SAVE) {
        // write_indentation(serializer, 1);
        // fprintf(serializer->fp, "%s: %d\n", key, *value);
    } else {

        get_entry(serializer, key, "%d", value);
    }
}




// // --------------------- DISABLED FOR NOW ---------------------
// void yaml_serializer_entry_float(serializer_yaml* serializer, const char* key, float* value);
// void yaml_serializer_entry_bool(serializer_yaml* serializer, const char* key, bool* value);
// void yaml_serializer_entry_string(serializer_yaml* serializer, const char* key, const char* value);
//
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
