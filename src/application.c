
#include "platform/window.h"

#include "application.h"


static application_state app_state;



b8 init_application(int argc, char *argv[]) {

    LOG_INIT

    ASSERT(create_window(&app_state.window, 800, 600, "Test Application"), "", "Failed to initialize window")

    // TODO: Initialize OpenGL renderer

    app_state.is_running = true;
    return true;
}


void shutdown_application() {

    destroy_window(&app_state.window);
    
    LOG_SHUTDOWN
}


void run_application() {

    // TODO: Call init_dashboard()
    
    while (!window_should_close(&app_state.window) && app_state.is_running) {
        window_poll_events();
        
        // TODO: Update all states
        // TODO: Render frame
        window_swap_buffers(&app_state.window);     // TODO: move to renderer
        
        // TODO: sleep to limit FPS
    }
    
    // TODO: Call shutdown_dashboard()
}
