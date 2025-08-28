#pragma once



// @brief Extracts the variable name from a compound expression.
//        Useful for logging and debugging to extract meaningful names
//        from complex expressions containing pointers and structures.
// @param name The compound expression (e.g., "option->m_font_size")
// @return The extracted variable name (e.g., "m_font_size") or NULL if invalid
const char* util_extract_variable_name(const char* name);
