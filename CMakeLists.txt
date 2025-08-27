cmake_minimum_required(VERSION 3.15)

# Project name and language - enable both C and CXX
project(application_template_c LANGUAGES C CXX)

# Use C11 by default
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Use C++11 for ImGui
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set shared library prefix for MinGW on Windows
if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

# Set installation prefix to build directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

# Set IMGUI source files
set(IMGUI_SOURCES 
    vendor/cimgui/cimgui.cpp 
    vendor/cimgui/imgui/imgui.cpp 
    vendor/cimgui/imgui/imgui_draw.cpp 
    vendor/cimgui/imgui/imgui_demo.cpp 
    vendor/cimgui/imgui/imgui_widgets.cpp
)

# Check if newer ImGui version with tables exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cimgui/imgui/imgui_tables.cpp)
    list(APPEND IMGUI_SOURCES vendor/cimgui/imgui/imgui_tables.cpp)
endif()

# Set IMGUI include directories
include_directories(
    vendor/cimgui
    vendor/cimgui/imgui
)

# Set IMGUI definitions
add_definitions(
    -DIMGUI_USER_CONFIG=\"${CMAKE_CURRENT_SOURCE_DIR}/vendor/cimgui/cimconfig.h\"
    -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
)

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add GLFW using FetchContent
set(GLFW_VERSION 3.4)
include(FetchContent)
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/${GLFW_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE  # Fix the timestamp warning
)

FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Add backend implementations
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cimgui/imgui/backends)
    set(BACKENDS_DIR "vendor/cimgui/imgui/backends")
else()
    set(BACKENDS_DIR "vendor/cimgui/imgui/examples")
endif()

list(APPEND IMGUI_SOURCES 
    ${BACKENDS_DIR}/imgui_impl_opengl3.cpp
    ${BACKENDS_DIR}/imgui_impl_glfw.cpp
)

# Create cimgui library
add_library(cimgui STATIC ${IMGUI_SOURCES})

# For Windows: Remove the outer quotes and ensure proper escaping
if(WIN32)
    target_compile_definitions(cimgui PUBLIC -DIMGUI_IMPL_API=extern\ \"C\"\ __declspec(dllexport))
else()
    target_compile_definitions(cimgui PUBLIC -DIMGUI_IMPL_API=extern\ \"C\")
endif()

# Link dependencies
target_link_libraries(cimgui 
    OpenGL::GL
    glfw
)

file(GLOB_RECURSE SOURCES "src/**.c")

# Add main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set executable-specific definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS
    -DCIMGUI_USE_OPENGL3
    -DCIMGUI_USE_GLFW
)

# Link executable with cimgui and dependencies
target_link_libraries(${PROJECT_NAME}
    cimgui
    OpenGL::GL
    glfw
)

# Set include directories for executable
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cimgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cimgui/imgui
    ${BACKENDS_DIR}
)