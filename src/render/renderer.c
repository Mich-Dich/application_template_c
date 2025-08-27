
#include "util/core_config.h"
#include "platform/window.h"
#include "imgui_config/imgui_config.h"

#include "renderer.h"


#if defined(RENDER_API_VULKAN)
    // Not to be implemented for a long while

#elif defined(RENDER_API_OPENGL)

    #include <GL/gl.h>
    #include <GL/glx.h>
    #include <GLFW/glfw3.h>

    // load function pointers manually, because glad made some trouble
    PFNGLCREATESHADERPROC        glCreateShader        = NULL;
    PFNGLSHADERSOURCEPROC        glShaderSource        = NULL;
    PFNGLCOMPILESHADERPROC       glCompileShader       = NULL;
    PFNGLGETSHADERIVPROC         glGetShaderiv         = NULL;
    PFNGLGETSHADERINFOLOGPROC    glGetShaderInfoLog    = NULL;
    PFNGLCREATEPROGRAMPROC       glCreateProgram       = NULL;
    PFNGLATTACHSHADERPROC        glAttachShader        = NULL;
    PFNGLLINKPROGRAMPROC         glLinkProgram         = NULL;
    PFNGLGETPROGRAMIVPROC        glGetProgramiv        = NULL;
    PFNGLGETPROGRAMINFOLOGPROC   glGetProgramInfoLog   = NULL;
    PFNGLDELETESHADERPROC        glDeleteShader        = NULL;
    PFNGLUSEPROGRAMPROC          glUseProgram          = NULL;


    // Simple shader sources
    static const char* vertex_shader_source = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";

    static const char* fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

    // Function to compile shaders
    static u32 compile_shader(u32 type, const char* source) {

        u32 shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        
        // Check compilation errors
        int success;
        char info_log[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, info_log);
            LOG(Error, "Shader compilation failed: %s", info_log);
        }
        return shader;
    }


b8 renderer_init(renderer_state* renderer) {
    // Initialize ImGui
    if (!imgui_init(application_get_window())) {
        LOG(Error, "Failed to initialize ImGui");
        return false;
    }

    // Get OpenGL version info
    LOG(Trace, "OpenGL version: %s", glGetString(GL_VERSION));
    LOG(Trace, "OpenGL vendor: %s", glGetString(GL_VENDOR));
    LOG(Trace, "OpenGL renderer: %s", glGetString(GL_RENDERER));

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    renderer->initialized = true;
    LOG_INIT
    return true;
}

void renderer_shutdown(renderer_state* renderer) {
    
    // imgui_shutdown();
    renderer->initialized = false;
    LOG_SHUTDOWN
}

// ============================================================================================================================================
// draw
// ============================================================================================================================================

void renderer_begin_frame(renderer_state* renderer) {
    
    if (!renderer->initialized) return;
    imgui_begin_frame();

    // ImGui demo code
    if (igBegin("My Window", NULL, 0)) {
        igText("Hello, world!");
        igEnd();
    }
}

void renderer_end_frame(window_info* window) {
    
    imgui_end_frame(window);
    window_swap_buffers(window);
}

void renderer_on_resize(renderer_state* renderer, u16 width, u16 height) {
    
    if (!renderer->initialized) return;
    glViewport(0, 0, width, height);
}

#endif
