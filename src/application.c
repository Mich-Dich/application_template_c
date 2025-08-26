
#include "util/system.h"
#include "platform/window.h"

#include "application.h"


static application_state app_state;


// ============================================================================================================================================
// util
// ============================================================================================================================================

static f64 s_desired_loop_duration_s = 10.f;       // in seconds
static f64 s_loop_start_time = 0.f;

//
void set_fps_values(const u16 desired_framerate) {

    s_desired_loop_duration_s = 1/(f32)desired_framerate;
    s_loop_start_time = get_precise_time();
}

//
void limit_fps() {

    const f64 current = get_precise_time();                     // I guess time in milliseconds
    const f64 difference = current - s_loop_start_time;
    if (difference < s_desired_loop_duration_s)
        precise_sleep(s_desired_loop_duration_s - difference);
    
    s_loop_start_time = get_precise_time();                     // as this is the last function call before next loop iteration
}

// ============================================================================================================================================
// util
// ============================================================================================================================================


b8 init_application(int argc, char *argv[]) {

    LOG_INIT

    ASSERT(create_window(&app_state.window, 800, 600, "Test Application"), "", "Failed to initialize window")

    // TODO: Initialize renderer

    app_state.is_running = true;
    return true;
}


void shutdown_application() {

    destroy_window(&app_state.window);
    
    LOG_SHUTDOWN
}


void run_application() {

    // TODO: Call init_dashboard()
    
    set_fps_values(30);         // set target FPS to 30
    while (!window_should_close(&app_state.window) && app_state.is_running) {
        window_poll_events();
        
        // TODO: Update all states
        // TODO: Render frame
        window_swap_buffers(&app_state.window);     // TODO: move to renderer
        
        limit_fps();
    }
    
    // TODO: Call shutdown_dashboard()
}
