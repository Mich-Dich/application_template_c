#pragma once

#include <cimgui.h>
#include "cimgui_impl.h"
#include "platform/window.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#ifdef IMGUI_HAS_IMSTR
    #define igBegin igBegin_Str
    #define igSliderFloat igSliderFloat_Str
    #define igCheckbox igCheckbox_Str
    #define igColorEdit3 igColorEdit3_Str
    #define igButton igButton_Str
#endif

#define igGetIO igGetIO_Nil


//
b8 imgui_init(window_info* window);

//
void imgui_shutdown();

//
void imgui_begin_frame();

//
void imgui_end_frame(window_info* window_date);

//
ImVec4* imgui_config_get_clear_color_ptr();
