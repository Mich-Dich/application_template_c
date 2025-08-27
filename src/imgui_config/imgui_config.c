
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "cimgui.h"
#include "cimgui_impl.h"

#include "util/core_config.h"
#include "util/data_structure/data_types.h"
#include "platform/window.h"

#include "imgui_config.h"



static ImVec4 s_clear_color;




b8 imgui_init(window_info* window_data) {

    // setup imgui
    igCreateContext(NULL);
    
    // set docking
    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    //ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
#ifdef IMGUI_HAS_DOCK
    ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
    ioptr->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows
#endif


    // Setup scaling
    ImGuiStyle* style = igGetStyle();
    const float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    ImGuiStyle_ScaleAllSizes(style, main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style->FontScaleDpi = main_scale;                   // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
    ioptr->ConfigDpiScaleFonts = true;                  // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    ioptr->ConfigDpiScaleViewports = true;              // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
#endif
    ImGui_ImplGlfw_InitForOpenGL(window_data->window_ptr, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
    // ImFontAtlas_AddFontDefault(io.Fonts, NULL);

    s_clear_color.x = 0.45f;
    s_clear_color.y = 0.55f;
    s_clear_color.z = 0.60f;
    s_clear_color.w = 1.00f;
}


void imgui_shutdown() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
}


void imgui_begin_frame() {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}


void imgui_end_frame(window_info* window_data) {

    // render
    igRender();
    ImGuiIO *ioptr = igGetIO();
    glfwMakeContextCurrent(window_data->window_ptr);
    glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
    glClearColor(s_clear_color.x, s_clear_color.y, s_clear_color.z, s_clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
#ifdef IMGUI_HAS_DOCK
    if (ioptr->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
    {
    GLFWwindow *backup_current_window = glfwGetCurrentContext();
    igUpdatePlatformWindows();
    igRenderPlatformWindowsDefault(NULL, NULL);
    glfwMakeContextCurrent(backup_current_window);
    }
#endif

    glfwSwapBuffers(window_data->window_ptr);
}


ImVec4* imgui_config_get_clear_color_ptr() { return &s_clear_color; }


