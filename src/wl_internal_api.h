#ifndef WAYLAND_INTERNAL_API_H
#define WAYLAND_INTERNAL_API_H

#include "wl_platform_state.h"

WaylandPlatformState *_wayland_get_platform_global_state();
void _wayland_set_platform_global_state(WaylandPlatformState *state);

void _wl_poll_events(double *timeout);

#endif // !WAYLAND_INTERNAL_API_H
