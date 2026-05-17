#ifndef WAYLAND_PLATFORM_STATE
#define WAYLAND_PLATFORM_STATE

#include "keycodes.h"
#include "wl_window.h"
#include <EGL/egl.h>
#include <stdint.h>

typedef struct KeyboardState {
    KeyState keyState[KEY_COUNT];
    uint32_t last_keyPress_time;
} KeyboardState;

typedef struct PointerState {
    struct Vector2i {
        int x, y;
    } position, position_delta, last_btn_press_position;
    uint32_t motion_timestamp;
    uint32_t relative_motion_timestamp;
    uint32_t last_button_press_timestamp;
    KeyState mouseButtonState[MOUSE_BUTTON_COUNT];
    MouseButtonCode last_pressed_button;
} PointerState;

typedef struct InputState {
    KeyboardState ks;
    PointerState ps;
} InputState;

typedef struct WaylandPlatformState {
    bool initialized;
    bool heap_allocated;

    struct wl_display *connection;
    struct wl_registry *registry;

    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;

    struct zxdg_decoration_manager_v1 *xdg_decoration_manager;

    struct wl_seat *wl_seat;
    struct wl_keyboard *wl_keyboard;
    struct wl_pointer *wl_pointer;
    struct wl_touch *wl_touch;

    struct zwp_relative_pointer_manager_v1 *relative_pointer_manager;
    struct zwp_relative_pointer_v1 *relative_pointer;

    struct zwp_pointer_constraints_v1 *pointer_constraint_manager;
    struct zwp_locked_pointer_v1 *locked_pointer;
    struct zwp_confined_pointer_v1 *confined_pointer;

    EGLDisplay egl_display;
    bool egl_initialized;

    InputState inputState;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;

    bool isKeyRepeat_on;
    int32_t keyRepeat_rate;
    int32_t keyRepeat_delay;

    WaylandWindow *focused_window;
    WaylandWindow **window_array;
} WaylandPlatformState;

#endif // !WAYLAND_PLATFORM_STATE
