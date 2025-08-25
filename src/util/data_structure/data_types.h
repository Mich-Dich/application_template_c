#pragma once

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

typedef int b32;
typedef char b8;

typedef u64 handle;  				                // Generic handle type for OS resources


#if defined(__clang__) || defined(__gcc__)
    #define static_assert       _Static_assert
#else
    #define static_assert       static_assert
#endif

static_assert(sizeof(u8) == 1,  "Expected [u8] to be 1 byte");
static_assert(sizeof(u16) == 2, "Expected [u16] to be 2 byte");
static_assert(sizeof(u32) == 4, "Expected [u32] to be 4 byte");
static_assert(sizeof(u64) == 8, "Expected [u64] to be 8 byte");

static_assert(sizeof(i8) == 1,  "Expected [i8] to be 1 byte");
static_assert(sizeof(i16) == 2, "Expected [i16] to be 2 byte");
static_assert(sizeof(i32) == 4, "Expected [i32] to be 4 byte");
static_assert(sizeof(i64) == 8, "Expected [i64] to be 8 byte");

static_assert(sizeof(f32) == 4, "Expected [f32] to be 4 byte");
static_assert(sizeof(f64) == 8, "Expected [f64] to be 8 byte");

static_assert(sizeof(b8) == 1,  "Expected [b8] to be 1 byte");
static_assert(sizeof(b32) == 4, "Expected [b32] to be 4 byte");


// Extension for asset files
#define AT_ASSET_EXTENTION			".gltasset"

// Extension for project files
#define PROJECT_EXTENTION    		".gltproj"

// Configuration file extensions
#define FILE_EXTENSION_CONFIG   	".yml"        	// Extension for YAML config files
#define FILE_EXTENSION_INI      	".ini"          // Extension for INI config files

// Temporary directory for DLL builds
#define PROJECT_TEMP_DLL_PATH 		"build_DLL"

// Directory structure macros
#define METADATA_DIR            	"metadata"      // Directory for metadata files
#define CONFIG_DIR              	"config"        // Directory for configuration files
#define CONTENT_DIR             	"content"       // Directory for content files
#define SOURCE_DIR              	"src"           // Directory for source code

#define PROJECT_PATH				application::get().get_project_path()
#define PROJECT_NAME				application::get().get_project_data().name

#define ASSET_PATH					util::get_executable_path() / "assets"

