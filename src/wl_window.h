#ifndef WAYLAND_WINDOW_H
#define WAYLAND_WINDOW_H

#include "internals/types/events.h"
#include "keycodes.h"
#include <EGL/egl.h>

struct WaylandWindow;

typedef void (*fp_resize)(struct WaylandWindow *window);
typedef void (*fp_render)(struct WaylandWindow *window, double time);
typedef void (*fp_keyEvent)(KeyCode key, KeyAction action);
typedef void (*fp_mouseBtnEvent)(MouseButtonCode btn, KeyAction action);
typedef void (*fp_mouseMoveEvent)(float xpos, float ypos);
typedef void (*fp_mouseScrollEvent)(float amt, ScrollDir dir);
typedef void (*fp_windowFocusLoss)(struct WaylandWindow *window);
typedef void (*fp_windowFocusGain)(struct WaylandWindow *window);
typedef void (*fp_mouseEnter)(struct WaylandWindow *window);
typedef void (*fp_mouseLeave)(struct WaylandWindow *window);
typedef void (*fp_windowCloseBtnPress)();
typedef void (*fp_windowMaximize)(struct WaylandWindow *window);
typedef void (*fp_windowMinimize)(struct WaylandWindow *window);

typedef enum WindowState {
    WINDOWED   = 0,
    MAXIMIZED  = 1,
    MINIMIZED  = 2,
    FULLSCREEN = 3,
    ACTIVATED  = 4,
    SUSPENDED  = 5,
    RESIZING   = 6
} WindowState;

typedef struct WaylandWindow {
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    struct zxdg_toplevel_decoration_v1 *xdg_toplevel_decoration;

    struct wl_callback *wl_surface_frame_done_callback;

    struct pointer_event_accumulator_t pointer_events;
    struct touch_event_accumulator_t touch_events;

    EGLSurface egl_surface;
    struct wl_egl_window *egl_window;

    bool initialized;
    bool heap_allocated;
    const char *title;
    uint32_t width;
    uint32_t height;
    bool shouldClose;
    bool isFullscreen;
    bool isBorderless;
    bool isResizable;
    double last_frame_time;

    // TODO: pack capabilities in a single uint8_t
    bool canMaximize;
    bool canMinimize;
    bool canFullscreen;

    // Do we need them ?
    bool maximized;
    bool minimized;
    bool activated;

    // Callbacks:
    struct {
        fp_resize resize;
        fp_render render;
        fp_keyEvent key_event;
        fp_mouseBtnEvent mouse_btn_event;
        fp_mouseMoveEvent mouse_motion;
        fp_windowCloseBtnPress window_close;
    } callbacks;

    struct WaylandWindow *next;
} WaylandWindow;

#endif // !WAYLAND_WINDOW_H
