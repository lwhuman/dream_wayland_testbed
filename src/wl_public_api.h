#ifndef WAYLAND_CLIENT_PUBLIC_API
#define WAYLAND_CLIENT_PUBLIC_API

#include "keycodes.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "wl_platform_state.h"
#include "wl_window.h"

// Platform lifetime management API:
int wayland_platform_initialize_s(WaylandPlatformState *wlPlatformState);
WaylandPlatformState *wayland_platform_initialize_h();
void wayland_platform_shutdown(WaylandPlatformState *wlPlatformState);

// Window creation-destruction API:
int wayland_create_window_s(WaylandWindow *window);
WaylandWindow *
wayland_create_window_h(uint32_t width, uint32_t height, const char *title);
void wayland_destroy_window(WaylandWindow *window);

// Window management API:
void wayland_show_window(WaylandWindow *window);
void wayland_set_window_title(WaylandWindow *window, const char *title);
void wayland_set_window_max_size(
    WaylandWindow *window, uint32_t width, uint32_t height
);
void wayland_set_window_min_size(
    WaylandWindow *window, uint32_t width, uint32_t height
);
void wayland_set_window_fullscreen(WaylandWindow *window, bool flag);
void wayland_set_window_borderless(WaylandWindow *window, bool flag);
void wayland_make_window_unresizable(WaylandWindow *window, bool flag);

// Event polling API:
void wayland_poll_event();
void wayland_wait_for_event();
void wayland_wait_for_event_till(double timeout);

// Input state query API:
KeyState get_key_state(WaylandWindow *window, KeyCode key);
KeyState get_mouse_button_state(WaylandWindow *window, MouseButtonCode btn);

bool is_key_pressed(WaylandWindow *window, KeyCode key);
bool is_key_released(WaylandWindow *window, KeyCode key);
bool is_mouse_button_pressed(WaylandWindow *window, MouseButtonCode btn);
bool is_mouse_button_released(WaylandWindow *window, MouseButtonCode btn);

void get_mouse_pos(WaylandWindow *window, float *x, float *y);
float get_mouse_x(WaylandWindow *window);
float get_mouse_y(WaylandWindow *window);

// Pointer management API:
void wayland_set_pointer_lock(WaylandWindow *window, bool lock);
void wayland_set_pointer_visibility(WaylandWindow *window, bool visibility);

// Toggle raw mouse motion feed API:
void wayland_set_raw_mouse_motion_enabled(bool enabled);

// Clipboard handling API:
void wayland_push_text_to_clipboard(const char *text);
const char *wayland_get_text_from_clipboard();

void wayland_trigger_frame(WaylandWindow *window);

#ifdef __cplusplus
}
#endif

#endif // !WAYLAND_CLIENT_PUBLIC_API
