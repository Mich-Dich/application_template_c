#pragma once

#include <stdbool.h>

// =============================================
// @brief Primitive type definitions for consistent sizing across platforms
// =============================================

typedef unsigned char  u8;    						// 8-bit unsigned integer
typedef unsigned short u16;   						// 16-bit unsigned integer
typedef unsigned int u32;   						// 32-bit unsigned integer
typedef unsigned long long u64;                     // 64-bit unsigned integer

typedef signed char  i8;   							// 8-bit signed integer
typedef signed short i16;  							// 16-bit signed integer
typedef signed int i32;  							// 32-bit signed integer
typedef signed long long i64;  						// 64-bit signed integer

typedef float f32;          						// 32-bit floating point
typedef double f64;         						// 64-bit floating point
typedef long double f128;   						// 128-bit floating point (platform dependent)

typedef bool     b8;
// typedef char b8;                                    // 8-bit bool
typedef int b32;                                    // 32-bit bool (often used as flag)

typedef u64 handle;  				                // Generic handle type for OS resources


// portable static assert for C11 and fallback 
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  // C11
  #define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
  // fallback: negative array size causes compile-time error for false condition
  #define STATIC_ASSERT_GLUE(a, b) a##b
  #define STATIC_ASSERT_LINE(name, line) STATIC_ASSERT_GLUE(name, line)
  #define STATIC_ASSERT(cond, msg) typedef char STATIC_ASSERT_LINE(static_assertion_, __LINE__)[(cond) ? 1 : -1]
#endif

STATIC_ASSERT(sizeof(u8) == 1,  "Expected [u8] to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected [u16] to be 2 byte");
STATIC_ASSERT(sizeof(u32) == 4, "Expected [u32] to be 4 byte");
STATIC_ASSERT(sizeof(u64) == 8, "Expected [u64] to be 8 byte");

STATIC_ASSERT(sizeof(i8) == 1,  "Expected [i8] to be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "Expected [i16] to be 2 byte");
STATIC_ASSERT(sizeof(i32) == 4, "Expected [i32] to be 4 byte");
STATIC_ASSERT(sizeof(i64) == 8, "Expected [i64] to be 8 byte");

STATIC_ASSERT(sizeof(f32) == 4, "Expected [f32] to be 4 byte");
STATIC_ASSERT(sizeof(f64) == 8, "Expected [f64] to be 8 byte");

STATIC_ASSERT(sizeof(b8) == 1,  "Expected [b8] to be 1 byte");
STATIC_ASSERT(sizeof(b32) == 4, "Expected [b32] to be 4 byte");


// Extension for asset files
#define AT_ASSET_EXTENTION			".atasset"

// Extension for project files
#define PROJECT_EXTENTION    		".atproj"

// Configuration file extensions
#define FILE_EXTENSION_CONFIG   	".yml"        	// Extension for YAML config files
#define FILE_EXTENSION_INI      	".ini"          // Extension for INI config files

// Directory structure macros
#define METADATA_DIR            	"metadata"      // Directory for metadata files
#define CONFIG_DIR              	"config"        // Directory for configuration files
#define CONTENT_DIR             	"content"       // Directory for content files
#define SOURCE_DIR              	"src"           // Directory for source code
// #define ASSET_DIR                   util_get_executable_path() / "assets"
