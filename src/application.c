
#include "application.h"


b8 init_application(int argc, char *argv[]) {

    LOG_INIT

    // TODO: create window
    // TODO: init OpenGL renderer
    
    return true;
}

void shutdown_application() {

    LOG_SHUTDOWN

    // TODO :cleanup
}


void run_application() {

    // TODO: init usercode from [init_dashboard()]
    
    // TODO: infinite loop
    //      TODO: check for close request (glfw)
    //      TODO: call updates
    //      TODO: call render
    //      TODO: wait to limit FPS

    // TODO: shutdown usercode from [shutdown_dashboard()]
}
