#pragma once

// -------------------------------------------------------------------------------------------------------------------------------------------
// Defines that have influence on system behavior
// -------------------------------------------------------------------------------------------------------------------------------------------

#define RENDER_API_OPENGL
// #define RENDER_API_VULKAN



#if defined(RENDER_API_OPENGL)
    #if __APPLE__                                   // GL 3.2 Core + GLSL 150
        static const char *glsl_version = "#version 150";
    #else                                           // GL 3.2 + GLSL 130
        static const char *glsl_version = "#version 130";
    #endif
#endif



// collect timing-data from every major function?
#define PROFILE_GENREAL								    0	// general level overview
#define PROFILE_RENDERER								0	// general level overview


// log assert and validation behaviour?
// NOTE - expr in assert/validation will still be executed
#define ENABLE_LOGGING_FOR_ASSERTS                      1

// toggle logging depending on build config
#if defined(DEBUG)
    #define ENABLE_LOGGING_FOR_VALIDATION               1
#else
    #define ENABLE_LOGGING_FOR_VALIDATION               0
#endif
