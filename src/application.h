#pragma once

#include "util/io/logger.h"
#include "util/system.h"
#include "platform/window.h"


typedef struct {
    window_info window;
    b8 is_running;
} application_state;

//
b8 init_application(int argc, char *argv[]);

//
void shutdown_application();

//
void run_application();

