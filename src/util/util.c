
#include <string.h>
#include <stddef.h>

#include "util/util.h"



// Example usage:
// util_extract_variable_name("option->m_font_size") returns "m_font_size"
// util_extract_variable_name("window.window_width") returns "window_width"
// util_extract_variable_name("parameter_x") returns "parameter_x"
const char* util_extract_variable_name(const char* name) {
    
    if (name == NULL)
        return NULL;
    
    // Find the last occurrence of "->"
    const char* arrow_pos = strstr(name, "->");
    const char* dot_pos = strrchr(name, '.');
    const char* last_delimiter = NULL;                                      // Determine which delimiter comes last in the string
    
    if (arrow_pos && dot_pos)                                               // Both found, use the one that appears later in the string
        last_delimiter = (arrow_pos > dot_pos) ? arrow_pos : dot_pos;
    else if (arrow_pos)                                                     // Only "->" found
        last_delimiter = arrow_pos;
    else if (dot_pos)                                                       // Only "." found
        last_delimiter = dot_pos;
    else                                                                    // No delimiter found, return the whole string
        return name;
    
    // Return the part after the delimiter
    // For "->", we need to skip both characters
    if (last_delimiter == arrow_pos)
        return last_delimiter + 2; // Skip "->"
    else
        return last_delimiter + 1; // Skip "."
}

