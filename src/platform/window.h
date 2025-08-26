#pragma once

#include <GLFW/glfw3.h>

#include "util/data_structure/data_types.h"



typedef struct {
    GLFWwindow*     window_ptr;
    u16             width;
    u16             height;
    const char*     title;
    b8              should_close;
} window_info;


//
b8 create_window(window_info* window_data, const u16 width, const u16 height, const char* title);

//
void destroy_window(window_info* window_data);

//
b8 window_should_close(window_info* window_data);

//
void window_poll_events();

//
void window_swap_buffers(window_info* window_data);

//
void window_set_should_close(window_info* window_data, b8 value);