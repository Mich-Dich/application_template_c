#pragma once

#include "util/data_structure/data_types.h"


//
b8 dashboard_init();

//
void dashboard_shutdown();

//
void dashboard_update(const f32 delta_time);

//
void dashboard_draw(const f32 delta_time);
