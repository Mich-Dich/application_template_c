
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "util/io/logger.h"
#include "imgui_config/imgui_config.h"
#include "render/image.h"

#include "dashboard.h"


static bool showDemoWindow = true;
static bool showAnotherWindow = false;
image_t test_image = {0};


b8 read_stable_diffusion_parameters(const char* filename, char** parameters) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return false;

    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    if (!png_ptr || !info_ptr) {
        if (png_ptr) png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_textp text_ptr = NULL;
    int num_text = 0;
    png_get_text(png_ptr, info_ptr, &text_ptr, &num_text);

    for (int i = 0; i < num_text; i++) {
        if (strcmp(text_ptr[i].key, "parameters") == 0) {
            *parameters = strdup(text_ptr[i].text);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return true;
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return false;
}


//
b8 dashboard_init() {

    char exe_path[1024] = {0};
    get_executable_path(exe_path, sizeof(exe_path));
    char image_path[2048] = {0};
    snprintf(image_path, sizeof(image_path), "%s/assets/images/test_image.png", exe_path);
    LOG(Trace, "Image at [%s]", image_path)
    VALIDATE(image_create_from_file(&test_image, image_path, IF_RGBA, false), , "", "Failed to create image");

    // Read Stable Diffusion parameters
    char* parameters = NULL;
    if (read_stable_diffusion_parameters(image_path, &parameters)) {
        LOG(Trace, "Stable Diffusion Parameters:");
        LOG(Trace, "%s", parameters);
        
        // Store parameters for later use in ImGui
        // You'll want to store this in a global variable or struct
        // global_image_parameters = parameters; // Assuming you have this variable
    } else {
        LOG(Warn, "No Stable Diffusion parameters found in image");
    }

    return true;
}

//
void dashboard_shutdown() {

    LOG_SHUTDOWN
}

//
void dashboard_on_crash() { LOG(Debug, "User crash_callback")}


//
void dashboard_update(__attribute_maybe_unused__ const f32 delta_time) { }

//
void dashboard_draw(__attribute_maybe_unused__ const f32 delta_time) {

    if (showDemoWindow)
        igShowDemoWindow(&showDemoWindow);

    // show a simple window that we created ourselves.
    {
        static float f = 0.0f;
        static int counter = 0;

        igBegin("Hello, world!", NULL, 0);
        igText("This is some useful text");
        igCheckbox("Demo window", &showDemoWindow);
        igCheckbox("Another window", &showAnotherWindow);

        igSliderFloat("Float", &f, 0.0f, 1.0f, "%.3f", 0);
        igColorEdit3("clear color", (float *)imgui_config_get_clear_color_ptr(), 0);

        ImVec2 buttonSize;
        buttonSize.x = 0;
        buttonSize.y = 0;
        if (igButton("Button", buttonSize))
            counter++;
        igSameLine(0.0f, -1.0f);
        igText("counter = %d", counter);

        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        
        ImVec2 image_size = {120, 80};
        igImage(image_get_texture_id(&test_image), image_size, (ImVec2){0,0}, (ImVec2){1,1});


        igEnd();
    }

    if (showAnotherWindow) {
        igBegin("imgui Another Window", &showAnotherWindow, 0);
        igText("Hello from imgui");
        ImVec2 buttonSize;
        buttonSize.x = 0;
        buttonSize.y = 0;
        if (igButton("Close me", buttonSize)) {
            showAnotherWindow = false;
        }
        igEnd();
    }
}


void dashboard_draw_init_UI(const f32 delta_time) {

    static f32 total_time = 0.f;
    total_time += delta_time;
    if (total_time > 1.f)
        total_time = 0.f;
    
    ImGuiViewport* viewport = igGetMainViewport();
    
    // Set window to cover the entire viewport
    const ImVec2 pos_vec = {0};
    igSetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, pos_vec);
    igSetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, pos_vec);
    igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    igBegin("Initialization", NULL, window_flags);
    {
        // Center content in the window
        ImVec2 center_pos = {0};
        center_pos.x = (viewport->WorkSize.x - 200) * 0.5f;
        center_pos.y = (viewport->WorkSize.y - 100) * 0.5f;
        igSetCursorPos(center_pos);
        
        igPushFont(imgui_config_get_font(FT_GIANT), g_font_size_giant);
        if (total_time < (1.f/4.f))
            igText("Initializing");
        else if (total_time < (2.f/4.f))
            igText("Initializing.");
        else if (total_time < (3.f/4.f))
            igText("Initializing..");
        else
            igText("Initializing...");
        igPopFont();

    #if 0   // currently not working, dont know why

        ImVec4 main_color = {1.f, 1.f, 1.f, 1.0f};
        ImVec4 backdrop_color = {0.2f, 0.2f, 0.2f, 1.0f};
        UI_loading_indicator_circle("##loading_indicator", 30, 13, 5, &main_color, &backdrop_color);
    #endif

    }
    igEnd();
    
    igPopStyleVar(2);
}
