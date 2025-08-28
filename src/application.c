
#include "util/io/logger.h"
#include "util/io/serializer_yaml.h"
#include "util/system.h"
#include "imgui_config/imgui_config.h"
#include "dashboard/dashboard.h"

#include "application.h"


static application_state app_state;


// ============================================================================================================================================
// FPS control
// ============================================================================================================================================

static f64 s_desired_loop_duration_s = 10.f;       // in seconds
static f64 s_loop_start_time = 0.f;
static f64 s_delta_time = 0.f;

//
void application_set_fps_values(const u16 desired_framerate) {

    s_desired_loop_duration_s = 1/(f32)desired_framerate;
    s_loop_start_time = get_precise_time();
}

//
void limit_fps() {

    const f64 current = get_precise_time();                     // I guess time in milliseconds
    const f64 difference = current - s_loop_start_time;
    s_delta_time = s_desired_loop_duration_s - difference;
    if (s_delta_time > 0.f)
        precise_sleep(s_delta_time);
    
    s_loop_start_time = get_precise_time();                     // as this is the last function call before next loop iteration
}

// ============================================================================================================================================
// application
// ============================================================================================================================================


b8 application_init(__attribute_maybe_unused__ int argc, __attribute_maybe_unused__ char *argv[]) {

    ASSERT(create_window(&app_state.window, 800, 600, "application_template_c"), "", "Failed to create window")
    // ASSERT(renderer_init(&app_state.renderer), "", "Failed to initialize renderer")
    imgui_init(&app_state.window);


    i32 test_i32 = 0;
    f32 test_float = 0;
    char test_str[256] = {0};
    bool test_bool = true;
    f128 test_long_long = 0;
    
    serializer_yaml sy;
    ASSERT(yaml_serializer_init(&sy, "config/test.yml", "main_section", SERIALIZER_OPTION_LOAD), "", "");
    yaml_serializer_entry_int(&sy, KEA_VALUE(test_i32));
    yaml_serializer_entry_float(&sy, KEA_VALUE(test_float));
    yaml_serializer_entry_bool(&sy, KEA_VALUE(test_bool));
    yaml_serializer_entry_string(&sy, KEA_VALUE(test_str), sizeof(test_str));
    yaml_serializer_entry(&sy, KEA_VALUE(test_long_long), "%llu");
    yaml_serializer_shutdown(&sy);

    LOG(Debug, "test_i32:       [%u]", test_i32)
    LOG(Debug, "test_float      [%f]", test_float)
    LOG(Debug, "test_str        [%s]", test_str)
    LOG(Debug, "test_bool       [%d]", test_bool)
    LOG(Debug, "test_long_long  [%d]", test_long_long)


    app_state.is_running = true;
    LOG_INIT
    return true;
}


void application_shutdown() {

    imgui_shutdown();
    // renderer_shutdown(&app_state.renderer);
    destroy_window(&app_state.window);
    
    LOG_SHUTDOWN
}


void application_run() {

    dashboard_init();
    
    application_set_fps_values(30);         // set target FPS to 30
    while (!window_should_close(&app_state.window) && app_state.is_running) {
        window_poll_events();
        
        dashboard_update(s_delta_time);
        
        // renderer_begin_frame(&app_state.renderer);
        imgui_begin_frame();
        dashboard_draw(s_delta_time);        
        imgui_end_frame(&app_state.window);
        // renderer_end_frame(&app_state.window);
        
        limit_fps();                        // sets [s_delta_time]
    }
    
    dashboard_shutdown();
}


// ============================================================================================================================================
// util
// ============================================================================================================================================

renderer_state* application_get_renderer()          { return &app_state.renderer; }

window_info* application_get_window()               { return &app_state.window; }

