
#include "util/io/logger.h"
#include "util/io/serializer_yaml.h"
#include "util/crash_handler.h"
#include "util/system.h"
#include "imgui_config/imgui_config.h"
#include "dashboard/dashboard.h"

#include "application.h"


// DEV-ONLY
#include <string.h>
// DEV-ONLY


static application_state app_state;


// ============================================================================================================================================
// FPS control
// ============================================================================================================================================

static u32 dashboard_crash_callback;
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



#define USE_SUB_SECTION 0

    i32 test_i32 = 200;
    b32 test_bool = true;
    f128 test_long_long = 5555;
    f32 test_f32 = 404.5050;
    f32 test_f32_s = 666.5050;
    
    serializer_yaml sy;

    const char* exec_path = get_executable_path();
    // char exec_path[4096] = {0};
    // get_executable_path_buf(exec_path, sizeof(exec_path));
    // ASSERT(exec_path != NULL, "", "FAILED")

    char loc_file_path[4096];
    memset(loc_file_path, '\0', sizeof(loc_file_path));
    const int written = snprintf(loc_file_path, sizeof(loc_file_path), "%s/%s", exec_path, "config");
    if (written < 0 || (size_t)written >= sizeof(loc_file_path)) {
        fprintf(stderr, "Path too long: %s/%s\n", exec_path, "config");
        return false; // or handle error properly
    }



    ASSERT(yaml_serializer_init(&sy, loc_file_path, "test.yml", "main_section", SERIALIZER_OPTION_SAVE), "", "");

    LOG(Trace, "ptr: [%p]", &test_i32)
    LOG(Trace, "value: [%u]", test_i32)
    LOG(Trace, "value: [%u]", &test_i32)

    yaml_serializer_entry(&sy, "test_i32", (void*)&test_i32, "%u");
    yaml_serializer_entry(&sy, "test_f32", (void*)&test_f32, "%f");
    yaml_serializer_entry(&sy, "test_bool", (void*)&test_bool, "%u");
    yaml_serializer_entry(&sy, "test_long_long", (void*)&test_long_long, "%Lf");
    
    char test_str[32000] = {0};
    strcpy(test_str, "Since C doesn't support switching on strings directly, we need to use a different approach.");
    yaml_serializer_entry_str(&sy, "test_str", (void*)&test_str, sizeof(test_str));

#if USE_SUB_SECTION
    yaml_serializer_subsection_begin(&sy, "sub_section");
    yaml_serializer_entry(&sy, KEY_VALUE(test_f32_s), "%f");
    yaml_serializer_subsection_end(&sy);
#endif

    yaml_serializer_shutdown(&sy);

    LOG(Trace, "test_i32:       [%u]", test_i32)
    LOG(Trace, "test_f32:       [%f]", test_f32)
    LOG(Trace, "test_bool       [%d]", test_bool)
    LOG(Trace, "test_long_long  [%Lf]", test_long_long)
    LOG(Trace, "test_str        [%s]", test_str)

#if USE_SUB_SECTION
    LOG(Trace, "SubSection: sub_section")
    LOG(Trace, "test_f32_s:         [%f]", test_f32_s)
#endif


    dashboard_crash_callback = crash_handler_subscribe_callback(dashboard_on_crash);

    app_state.is_running = true;
    LOG_INIT
    return true;
}


void application_shutdown() {

    crash_handler_unsubscribe_callback(dashboard_crash_callback);

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


/*

*/