

#include "window.h"

#include "util/io/logger.h"


static void glfw_error_callback(int error, const char* description) {
    LOG(Error, "GLFW Error (%d): %s", error, description);
}


b8 create_window(window_info* window_data, const u16 width, const u16 height, const char* title) {

    glfwSetErrorCallback(glfw_error_callback);

    VALIDATE(glfwInit(), return false, "", "Failed to initialize GLFW")

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window_data->window_ptr = glfwCreateWindow(width, height, title, NULL, NULL);
    VALIDATE(window_data->window_ptr, glfwTerminate(); return false, "", "Failed to create GLFW window")

    glfwMakeContextCurrent(window_data->window_ptr);
    glfwSwapInterval(1); // Enable vsync

    window_data->width = width;
    window_data->height = height;
    window_data->title = title;
    window_data->should_close = false;

    LOG(Trace, "Window [%s] created successfully (%dx%d)", title, width, height);
    return true;
}


void destroy_window(window_info* window_data) {

    if (window_data->window_ptr) {
        glfwDestroyWindow(window_data->window_ptr);
        window_data->window_ptr = NULL;
    }
    glfwTerminate();
    LOG(Trace, "shutdown window [%s]", window_data->title)
}


b8 window_should_close(window_info* window_data)                    { return window_data->should_close || glfwWindowShouldClose(window_data->window_ptr); }


void window_poll_events()                                           { glfwPollEvents(); }


void window_swap_buffers(window_info* window_data)                  { glfwSwapBuffers(window_data->window_ptr); }


void window_set_should_close(window_info* window_data, b8 value)    { window_data->should_close = value; }

