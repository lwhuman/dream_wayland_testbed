#include "wl_public_api.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <xkbcommon/xkbcommon.h>

#include "keycodes.h"
#include "protocols/pointer-constraints-protocol.h"
#include "protocols/relative-pointer-protocol.h"
#include "protocols/xdg-decoration-protocol.h"
#include "protocols/xdg-shell-client-protocol.h"
#include "wl_internal_api.h"
#include "wl_platform_state.h"
#include "wl_window.h"

#include "dynarr.h"

#define VERBOSE
#include "utility.h"

//////// Internal APIs for input handling ////////
#include <string.h>
#include <xkbcommon/xkbcommon-keysyms.h>

// These codes are copied from
// linux/input-event-codes.h
// This header is not included
// directly because our custom
// key codes conflicts with codes
// defined in it.
#define BTN_LEFT   0x110
#define BTN_RIGHT  0x111
#define BTN_MIDDLE 0x112

KeyCode _getOurKeyCode_from_xkb_keysym(xkb_keysym_t sym) {
    switch (sym) {
        case XKB_KEY_a:
        case XKB_KEY_A: return KEY_A;
        case XKB_KEY_b:
        case XKB_KEY_B: return KEY_B;
        case XKB_KEY_c:
        case XKB_KEY_C: return KEY_C;
        case XKB_KEY_d:
        case XKB_KEY_D: return KEY_D;
        case XKB_KEY_e:
        case XKB_KEY_E: return KEY_E;
        case XKB_KEY_f:
        case XKB_KEY_F: return KEY_F;
        case XKB_KEY_g:
        case XKB_KEY_G: return KEY_G;
        case XKB_KEY_h:
        case XKB_KEY_H: return KEY_H;
        case XKB_KEY_i:
        case XKB_KEY_I: return KEY_I;
        case XKB_KEY_j:
        case XKB_KEY_J: return KEY_J;
        case XKB_KEY_k:
        case XKB_KEY_K: return KEY_K;
        case XKB_KEY_l:
        case XKB_KEY_L: return KEY_L;
        case XKB_KEY_m:
        case XKB_KEY_M: return KEY_M;
        case XKB_KEY_n:
        case XKB_KEY_N: return KEY_N;
        case XKB_KEY_o:
        case XKB_KEY_O: return KEY_O;
        case XKB_KEY_p:
        case XKB_KEY_P: return KEY_P;
        case XKB_KEY_q:
        case XKB_KEY_Q: return KEY_Q;
        case XKB_KEY_r:
        case XKB_KEY_R: return KEY_R;
        case XKB_KEY_s:
        case XKB_KEY_S: return KEY_S;
        case XKB_KEY_t:
        case XKB_KEY_T: return KEY_T;
        case XKB_KEY_u:
        case XKB_KEY_U: return KEY_U;
        case XKB_KEY_v:
        case XKB_KEY_V: return KEY_V;
        case XKB_KEY_w:
        case XKB_KEY_W: return KEY_W;
        case XKB_KEY_x:
        case XKB_KEY_X: return KEY_X;
        case XKB_KEY_y:
        case XKB_KEY_Y: return KEY_Y;
        case XKB_KEY_z:
        case XKB_KEY_Z: return KEY_Z;

        case XKB_KEY_0: return KEY_0;
        case XKB_KEY_1: return KEY_1;
        case XKB_KEY_2: return KEY_2;
        case XKB_KEY_3: return KEY_3;
        case XKB_KEY_4: return KEY_4;
        case XKB_KEY_5: return KEY_5;
        case XKB_KEY_6: return KEY_6;
        case XKB_KEY_7: return KEY_7;
        case XKB_KEY_8: return KEY_8;
        case XKB_KEY_9: return KEY_9;

        case XKB_KEY_F1:  return KEY_F1;
        case XKB_KEY_F2:  return KEY_F2;
        case XKB_KEY_F3:  return KEY_F3;
        case XKB_KEY_F4:  return KEY_F4;
        case XKB_KEY_F5:  return KEY_F5;
        case XKB_KEY_F6:  return KEY_F6;
        case XKB_KEY_F7:  return KEY_F7;
        case XKB_KEY_F8:  return KEY_F8;
        case XKB_KEY_F9:  return KEY_F9;
        case XKB_KEY_F10: return KEY_F10;
        case XKB_KEY_F11: return KEY_F11;
        case XKB_KEY_F12: return KEY_F12;

        case XKB_KEY_semicolon:    return KEY_SEMICOLON;
        case XKB_KEY_equal:        return KEY_EQUAL;
        case XKB_KEY_bracketleft:  return KEY_LEFT_BRACKET;
        case XKB_KEY_bracketright: return KEY_RIGHT_BRACKET;
        case XKB_KEY_slash:        return KEY_SLASH;
        case XKB_KEY_backslash:    return KEY_BACKSLASH;
        case XKB_KEY_grave:        return KEY_GRAVE_ACCENT;
        case XKB_KEY_apostrophe:   return KEY_APOSTROPHE;
        case XKB_KEY_comma:        return KEY_COMMA;
        case XKB_KEY_minus:        return KEY_MINUS;
        case XKB_KEY_period:       return KEY_PERIOD;

        case XKB_KEY_rightarrow: return KEY_RIGHT_ARROW;
        case XKB_KEY_leftarrow:  return KEY_LEFT_ARROW;
        case XKB_KEY_uparrow:    return KEY_UP_ARROW;
        case XKB_KEY_downarrow:  return KEY_DOWN_ARROW;

        case XKB_KEY_Insert:    return KEY_INSERT;
        case XKB_KEY_Delete:    return KEY_DELETE;
        case XKB_KEY_Home:      return KEY_HOME;
        case XKB_KEY_End:       return KEY_END;
        case XKB_KEY_Page_Up:   return KEY_PAGE_UP;
        case XKB_KEY_Page_Down: return KEY_PAGE_DOWN;

        case XKB_KEY_Escape:    return KEY_ESCAPE;
        case XKB_KEY_Return:    return KEY_ENTER;
        case XKB_KEY_Tab:       return KEY_TAB;
        case XKB_KEY_BackSpace: return KEY_BACKSPACE;
        case XKB_KEY_space:     return KEY_SPACE;
        case XKB_KEY_Shift_L:   return KEY_LEFT_SHIFT;
        case XKB_KEY_Shift_R:   return KEY_RIGHT_SHIFT;
        case XKB_KEY_Control_L: return KEY_LEFT_CONTROL;
        case XKB_KEY_Control_R: return KEY_RIGHT_CONTROL;
        case XKB_KEY_Alt_L:     return KEY_LEFT_ALT;
        case XKB_KEY_Alt_R:     return KEY_RIGHT_ALT;
        case XKB_KEY_Super_L:   return KEY_LEFT_SUPER;
        case XKB_KEY_Super_R:   return KEY_RIGHT_SUPER;

        case XKB_KEY_Caps_Lock:   return KEY_CAPS_LOCK;
        case XKB_KEY_Scroll_Lock: return KEY_SCROLL_LOCK;
        case XKB_KEY_Num_Lock:    return KEY_NUM_LOCK;
        case XKB_KEY_Print:       return KEY_PRINT_SCREEN;
        case XKB_KEY_Pause:       return KEY_PAUSE;

        case XKB_KEY_KP_0:        return KEY_NP_0;
        case XKB_KEY_KP_1:        return KEY_NP_1;
        case XKB_KEY_KP_2:        return KEY_NP_2;
        case XKB_KEY_KP_3:        return KEY_NP_3;
        case XKB_KEY_KP_4:        return KEY_NP_4;
        case XKB_KEY_KP_5:        return KEY_NP_5;
        case XKB_KEY_KP_6:        return KEY_NP_6;
        case XKB_KEY_KP_7:        return KEY_NP_7;
        case XKB_KEY_KP_8:        return KEY_NP_8;
        case XKB_KEY_KP_9:        return KEY_NP_9;
        case XKB_KEY_KP_Decimal:  return KEY_NP_DECIMAL;
        case XKB_KEY_KP_Divide:   return KEY_NP_DIVIDE;
        case XKB_KEY_KP_Add:      return KEY_NP_ADD;
        case XKB_KEY_KP_Multiply: return KEY_NP_MULTIPLY;
        case XKB_KEY_KP_Subtract: return KEY_NP_SUBTRACT;
        case XKB_KEY_KP_Enter:    return KEY_NP_ENTER;
        case XKB_KEY_KP_Equal:    return KEY_NP_EQUAL;
    }

    return KEY_UNKNOWN;
}

MouseButtonCode _getOurMouseBtnCode_from_linux_event_code(uint32_t btn) {
    switch (btn) {
        case BTN_LEFT:   return MOUSE_BUTTON_LEFT;
        case BTN_RIGHT:  return MOUSE_BUTTON_RIGHT;
        case BTN_MIDDLE: return MOUSE_BUTTON_MIDDLE;
    }

    return MOUSE_BUTTON_UNKNOWN;
}

void _registerKeyState(
    WaylandPlatformState *platformState, KeyCode key, KeyState state
) {
    platformState->inputState.ks.keyState[key] = state;
}

void _resetKeyState(WaylandPlatformState *platformState) {
    memset(
        platformState->inputState.ks.keyState,
        0,
        sizeof(platformState->inputState.ks.keyState)
    );
}

void _registerMouseBtnState(
    WaylandPlatformState *platformState, MouseButtonCode btn, KeyState state
) {
    platformState->inputState.ps.mouseButtonState[btn] = state;
}

void _resetMouseBtnState(WaylandPlatformState *platformState) {
    memset(
        platformState->inputState.ps.mouseButtonState,
        0,
        sizeof(platformState->inputState.ps.mouseButtonState)
    );
}

void _updateMousePos(WaylandPlatformState *platformState, float x, float y) {
    platformState->inputState.ps.position.x = x;
    platformState->inputState.ps.position.y = y;
}

void _updateMouseDelta(
    WaylandPlatformState *platformState, float dx, float dy
) {
    platformState->inputState.ps.position_delta.x = dx;
    platformState->inputState.ps.position_delta.y = dy;
}
//////// END OF Internal APIs for input handling ////////

//////// wl_keyboard EVENT LISTENER CALLBACKS ////////
static void wl_keyboard_keymap(
    void *data,
    struct wl_keyboard *wl_keyboard,
    uint32_t format,
    int32_t fd,
    uint32_t size
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

    char *map_shm = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    assert(map_shm != MAP_FAILED);

    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
        platform_state->xkb_context,
        map_shm,
        XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS
    );
    munmap(map_shm, size);
    close(fd);

    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(platform_state->xkb_keymap);
    xkb_state_unref(platform_state->xkb_state);
    platform_state->xkb_keymap = xkb_keymap;
    platform_state->xkb_state  = xkb_state;
}

static void wl_keyboard_enter(
    void *data,
    struct wl_keyboard *wl_keyboard,
    uint32_t serial,
    struct wl_surface *surface,
    struct wl_array *keys
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    verbose("keyboard enter\n");
    size_t arr_len = dyn_arr_len(platform_state->window_array);
    for (size_t i = 0; i < arr_len; ++i) {
        WaylandWindow *w = platform_state->window_array[i];
        if (w->wl_surface == surface) {
            platform_state->focused_window = w;
            break;
        }
    }
    // uint32_t *key;
    // for (key = (uint32_t *)keys->data;
    //      (const char *)key < ((const char *)keys->data + keys->size);
    //      key++) {
    //     char buf[128];
    //     xkb_keysym_t sym =
    //         xkb_state_key_get_one_sym(platform_state->xkb_state, *key + 8);
    //     xkb_keysym_get_name(sym, buf, sizeof(buf));
    //     verbose("sym: %-12s (%d), ", buf, sym);
    //     xkb_state_key_get_utf8(
    //         platform_state->xkb_state, *key + 8, buf, sizeof(buf)
    //     );
    //     verbose("utf8: '%s'\n", buf);
    // }
}

static void wl_keyboard_key(
    void *data,
    struct wl_keyboard *wl_keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;

    const KeyCode key_code = _getOurKeyCode_from_xkb_keysym(
        xkb_state_key_get_one_sym(platform_state->xkb_state, key + 8)
    );
    const KeyState key_state =
        (state == WL_KEYBOARD_KEY_STATE_PRESSED ? PRESSED : UNPRESSED);
    const KeyAction action =
        (state == WL_KEYBOARD_KEY_STATE_RELEASED ? KEY_RELEASED : KEY_PRESSED);
    _registerKeyState(platform_state, key_code, key_state);

    if (focused_window->callbacks.key_event) {
        focused_window->callbacks.key_event(key_code, action);
    }

    char buf[2];
    if (xkb_state_key_get_utf8(
            platform_state->xkb_state, key + 8, buf, sizeof(buf)
        ) != 0) {
        // TODO: call get_character callback from here
        verbose("utf8: '%s'\n", buf);
    }
}

static void wl_keyboard_leave(
    void *data,
    struct wl_keyboard *wl_keyboard,
    uint32_t serial,
    struct wl_surface *surface
) {
    // verbose("keyboard leave\n");
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    platform_state->focused_window       = nullptr;
}

static void wl_keyboard_modifiers(
    void *data,
    struct wl_keyboard *wl_keyboard,
    uint32_t serial,
    uint32_t mods_depressed,
    uint32_t mods_latched,
    uint32_t mods_locked,
    uint32_t group
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    xkb_state_update_mask(
        platform_state->xkb_state,
        mods_depressed,
        mods_latched,
        mods_locked,
        0,
        0,
        group
    );
}

static void wl_keyboard_repeat_info(
    void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;

    platform_state->keyRepeat_rate  = rate;
    platform_state->keyRepeat_delay = delay;
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap      = wl_keyboard_keymap,
    .enter       = wl_keyboard_enter,
    .leave       = wl_keyboard_leave,
    .key         = wl_keyboard_key,
    .modifiers   = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};
//////// END OF wl_keyboard EVENT LISTENER CALLBACKS ////////

//////// wl_pointer EVENT LISTENER CALLBACKS ////////
enum pointer_event_mask {
    POINTER_EVENT_ENTER         = 1 << 0,
    POINTER_EVENT_LEAVE         = 1 << 1,
    POINTER_EVENT_MOTION        = 1 << 2,
    POINTER_EVENT_BUTTON        = 1 << 3,
    POINTER_EVENT_AXIS          = 1 << 4,
    POINTER_EVENT_AXIS_SOURCE   = 1 << 5,
    POINTER_EVENT_AXIS_STOP     = 1 << 6,
    POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

static void wl_pointer_enter(
    void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    struct wl_surface *surface,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_ENTER;
    focused_window->pointer_events.serial    = serial;
    focused_window->pointer_events.surface_x = surface_x,
    focused_window->pointer_events.surface_y = surface_y;

    // TODO: Should we register window as focused on pointer gain ?
    // size_t arr_len = dyn_arr_len(platform_state->window_array);
    // for (size_t i = 0; i < arr_len; ++i) {
    //     WaylandWindow *w = platform_state->window_array[i];
    //     if (w->wl_surface == surface) {
    //         platform_state->focused_window = w;
    //         break;
    //     }
    // }
}

static void wl_pointer_leave(
    void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    struct wl_surface *surface
) {
    WaylandPlatformState *platform_state  = (WaylandPlatformState *)data;
    WaylandWindow *focused_window         = platform_state->focused_window;
    focused_window->pointer_events.serial = serial;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_LEAVE;
}

static void wl_pointer_motion(
    void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_MOTION;
    focused_window->pointer_events.time      = time;
    focused_window->pointer_events.surface_x = surface_x,
    focused_window->pointer_events.surface_y = surface_y;
}

static void wl_pointer_button(
    void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_BUTTON;
    focused_window->pointer_events.time   = time;
    focused_window->pointer_events.serial = serial;
    focused_window->pointer_events.button = button,
    focused_window->pointer_events.state  = state;
}

static void wl_pointer_axis(
    void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_AXIS;
    focused_window->pointer_events.time             = time;
    focused_window->pointer_events.axes[axis].valid = true;
    focused_window->pointer_events.axes[axis].value = value;
}

static void wl_pointer_axis_source(
    void *data, struct wl_pointer *wl_pointer, uint32_t axis_source
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_AXIS_SOURCE;
    focused_window->pointer_events.axis_source = axis_source;
}

static void wl_pointer_axis_stop(
    void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.time  = time;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_AXIS_STOP;
    focused_window->pointer_events.axes[axis].valid = true;
}

static void wl_pointer_axis_discrete(
    void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->pointer_events.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
    focused_window->pointer_events.axes[axis].valid    = true;
    focused_window->pointer_events.axes[axis].discrete = discrete;
}

static void wl_pointer_high_resolution_axis_event(
    void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120
) {

    // Print axis event information
    verbose("Axis value 120 event:\n");
    verbose("  Axis: %d\n", axis);
    verbose("  Value: %d\n", value120);

    // Handle horizontal scroll
    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        // Calculate scroll amount
        int scroll_amount = value120 / 120;

        // Scroll horizontally
        verbose("Scrolling horizontally by %d units\n", scroll_amount);
        // Add code to handle horizontal scrolling
    }

    // Handle vertical scroll
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        // Calculate scroll amount
        int scroll_amount = value120 / 120;

        // Scroll vertically
        verbose("Scrolling vertically by %d units\n", scroll_amount);
        // Add code to handle vertical scrolling
    }

    // Handle other axes (e.g., WL_POINTER_AXIS_LEFT_BTN,
    // WL_POINTER_AXIS_RIGHT_BTN)
    else {
        verbose("Unhandled axis event\n");
    }
}

static void wl_pointer_relative_direction_event(
    void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction
) {
    // Print axis relative event information
    verbose("Axis relative event:\n");
    verbose("  Axis: %d\n", axis);
    verbose("  Direction: %d\n", direction);

    // Handle horizontal axis
    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        if (direction > 0) {
            verbose("Moving right\n");
            // Add code to handle right movement
        } else if (direction < 0) {
            verbose("Moving left\n");
            // Add code to handle left movement
        }
    }

    // Handle vertical axis
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        if (direction > 0) {
            verbose("Moving down\n");
            // Add code to handle down movement
        } else if (direction < 0) {
            verbose("Moving up\n");
            // Add code to handle up movement
        }
    }
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
    WaylandPlatformState *platform_state      = (WaylandPlatformState *)data;
    WaylandWindow *focused_window             = platform_state->focused_window;
    struct pointer_event_accumulator_t *event = &focused_window->pointer_events;
    /*verbose("pointer frame @ %d: ", event->time);*/

    if (event->event_mask & POINTER_EVENT_ENTER) {
        // verbose("entered %f, %f ", wl_fixed_to_double(event->surface_x),
        // wl_fixed_to_double(event->surface_y));
    }

    if (event->event_mask & POINTER_EVENT_LEAVE) {
        // verbose("leave");
    }

    if (event->event_mask & POINTER_EVENT_MOTION) {
        float x = wl_fixed_to_double(event->surface_x);
        float y = wl_fixed_to_double(event->surface_y);
        _updateMousePos(platform_state, x, y);
        if (focused_window->callbacks.mouse_motion) {
            focused_window->callbacks.mouse_motion(x, y);
        }
    }

    if (event->event_mask & POINTER_EVENT_BUTTON) {

        const MouseButtonCode btn =
            _getOurMouseBtnCode_from_linux_event_code(event->button);
        const KeyState state =
            (event->state == WL_POINTER_BUTTON_STATE_RELEASED ? UNPRESSED
                                                              : PRESSED);
        const KeyAction action =
            (event->state == WL_POINTER_BUTTON_STATE_RELEASED ? KEY_RELEASED
                                                              : KEY_PRESSED);
        _registerMouseBtnState(platform_state, btn, state);
        if (focused_window->callbacks.mouse_btn_event) {
            focused_window->callbacks.mouse_btn_event(btn, action);
        }
    }

    uint32_t axis_events     = POINTER_EVENT_AXIS | POINTER_EVENT_AXIS_SOURCE |
                               POINTER_EVENT_AXIS_STOP |
                               POINTER_EVENT_AXIS_DISCRETE;
    const char *axis_name[2] = {
        //[WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
        //[WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal",
    };
    const char *axis_source[4] = {
        // [WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
        // [WL_POINTER_AXIS_SOURCE_FINGER] = "finger",
        // [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
        // [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt",
    };
    if (event->event_mask & axis_events) {
        for (size_t i = 0; i < 2; ++i) {
            if (!event->axes[i].valid) {
                continue;
            }
            verbose("%s axis ", axis_name[i]);
            if (event->event_mask & POINTER_EVENT_AXIS) {
                verbose("value %f ", wl_fixed_to_double(event->axes[i].value));
            }
            if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE) {
                verbose("discrete %d ", event->axes[i].discrete);
            }
            if (event->event_mask & POINTER_EVENT_AXIS_SOURCE) {
                verbose("via %s ", axis_source[event->axis_source]);
            }
            if (event->event_mask & POINTER_EVENT_AXIS_STOP) {
                verbose("(stopped) ");
            }
        }
    }

    memset(event, 0, sizeof(*event));
}

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter                   = wl_pointer_enter,
    .leave                   = wl_pointer_leave,
    .motion                  = wl_pointer_motion,
    .button                  = wl_pointer_button,
    .axis                    = wl_pointer_axis,
    .frame                   = wl_pointer_frame,
    .axis_source             = wl_pointer_axis_source,
    .axis_stop               = wl_pointer_axis_stop,
    .axis_discrete           = wl_pointer_axis_discrete,
    .axis_value120           = wl_pointer_high_resolution_axis_event,
    .axis_relative_direction = wl_pointer_relative_direction_event,
};
//////// END OF wl_pointer EVENT LISTENER CALLBACKS ////////

//////// relative_pointer EVENT LISTENER CALLBACKS ////////
static void on_relative_motion(
    void *data,
    struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
    uint32_t utime_hi,
    uint32_t utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
) {

    double _dx = wl_fixed_to_double(dx);
    double _dy = wl_fixed_to_double(dy);

    double na_dx = wl_fixed_to_double(dx_unaccel);
    double na_dy = wl_fixed_to_double(dy_unaccel);

    // verbose("\nrelative pointer event: (%f %f) (%f %f)", _dx, _dy, na_dx,
    //         na_dy);
    // TODO: Handle relative_pointer event data
}
static const struct zwp_relative_pointer_v1_listener relative_pointer_listener =
    {
        .relative_motion = on_relative_motion,
};
//////// END OF relative_pointer EVENT LISTENER CALLBACKS ////////

//////// wl_touch EVENT LISTENER CALLBACKS ////////
enum touch_event_mask {
    TOUCH_EVENT_DOWN        = 1 << 0,
    TOUCH_EVENT_UP          = 1 << 1,
    TOUCH_EVENT_MOTION      = 1 << 2,
    TOUCH_EVENT_CANCEL      = 1 << 3,
    TOUCH_EVENT_SHAPE       = 1 << 4,
    TOUCH_EVENT_ORIENTATION = 1 << 5,
};

static struct touch_point *
get_touch_point(WaylandPlatformState *platform_state, int32_t id) {
    WaylandWindow *focused_window           = platform_state->focused_window;
    struct touch_event_accumulator_t *touch = &focused_window->touch_events;
    const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
    int invalid        = -1;
    for (size_t i = 0; i < nmemb; ++i) {
        if (touch->points[i].id == id) {
            return &touch->points[i];
        }
        if (invalid == -1 && !touch->points[i].valid) {
            invalid = i;
        }
    }
    if (invalid == -1) {
        return NULL;
    }
    touch->points[invalid].valid = true;
    touch->points[invalid].id    = id;
    return &touch->points[invalid];
}

static void wl_touch_down(
    void *data,
    struct wl_touch *wl_touch,
    uint32_t serial,
    uint32_t time,
    struct wl_surface *surface,
    int32_t id,
    wl_fixed_t x,
    wl_fixed_t y
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    struct touch_point *point            = get_touch_point(platform_state, id);
    if (point == NULL) {
        return;
    }
    point->event_mask |= TOUCH_EVENT_UP;
    point->surface_x                    = wl_fixed_to_double(x),
    point->surface_y                    = wl_fixed_to_double(y);
    focused_window->touch_events.time   = time;
    focused_window->touch_events.serial = serial;
}

static void wl_touch_up(
    void *data,
    struct wl_touch *wl_touch,
    uint32_t serial,
    uint32_t time,
    int32_t id
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    struct touch_point *point            = get_touch_point(platform_state, id);
    if (point == NULL) {
        return;
    }
    point->event_mask |= TOUCH_EVENT_UP;
}

static void wl_touch_motion(
    void *data,
    struct wl_touch *wl_touch,
    uint32_t time,
    int32_t id,
    wl_fixed_t x,
    wl_fixed_t y
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    struct touch_point *point            = get_touch_point(platform_state, id);
    if (point == NULL) {
        return;
    }
    point->event_mask |= TOUCH_EVENT_MOTION;
    point->surface_x = x, point->surface_y = y;
    focused_window->touch_events.time = time;
}

static void wl_touch_cancel(void *data, struct wl_touch *wl_touch) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->touch_events.event_mask |= TOUCH_EVENT_CANCEL;
}

static void wl_touch_shape(
    void *data,
    struct wl_touch *wl_touch,
    int32_t id,
    wl_fixed_t major,
    wl_fixed_t minor
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    struct touch_point *point            = get_touch_point(platform_state, id);
    if (point == NULL) {
        return;
    }
    point->event_mask |= TOUCH_EVENT_SHAPE;
    point->major = major, point->minor = minor;
}

static void wl_touch_orientation(
    void *data, struct wl_touch *wl_touch, int32_t id, wl_fixed_t orientation
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    struct touch_point *point            = get_touch_point(platform_state, id);
    if (point == NULL) {
        return;
    }
    point->event_mask |= TOUCH_EVENT_ORIENTATION;
    point->orientation = orientation;
}

static void wl_touch_frame(void *data, struct wl_touch *wl_touch) {
    WaylandPlatformState *platform_state    = (WaylandPlatformState *)data;
    WaylandWindow *focused_window           = platform_state->focused_window;
    struct touch_event_accumulator_t *touch = &focused_window->touch_events;
    const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
    verbose("touch event @ %d:\n", touch->time);

    for (size_t i = 0; i < nmemb; ++i) {
        struct touch_point *point = &touch->points[i];
        if (!point->valid) {
            continue;
        }
        verbose("point %d: ", touch->points[i].id);

        if (point->event_mask & TOUCH_EVENT_DOWN) {
            verbose(
                "down %f,%f ",
                wl_fixed_to_double(point->surface_x),
                wl_fixed_to_double(point->surface_y)
            );
        }

        if (point->event_mask & TOUCH_EVENT_UP) {
            verbose("up ");
        }

        if (point->event_mask & TOUCH_EVENT_MOTION) {
            verbose(
                "motion %f,%f ",
                wl_fixed_to_double(point->surface_x),
                wl_fixed_to_double(point->surface_y)
            );
        }

        if (point->event_mask & TOUCH_EVENT_SHAPE) {
            verbose(
                "shape %fx%f ",
                wl_fixed_to_double(point->major),
                wl_fixed_to_double(point->minor)
            );
        }

        if (point->event_mask & TOUCH_EVENT_ORIENTATION) {
            verbose("orientation %f ", wl_fixed_to_double(point->orientation));
        }

        point->valid = false;
        verbose("\n");
    }
}

static const struct wl_touch_listener wl_touch_listener = {
    .down        = wl_touch_down,
    .up          = wl_touch_up,
    .motion      = wl_touch_motion,
    .frame       = wl_touch_frame,
    .cancel      = wl_touch_cancel,
    .shape       = wl_touch_shape,
    .orientation = wl_touch_orientation,
};
//////// END OF wl_touch EVENT LISTENER CALLBACKS ////////

//////// wl_seat EVENT LISTENER CALLBACKS ////////
static void wl_seat_capabilities(
    void *data, struct wl_seat *wl_seat, uint32_t capabilities
) {
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && platform_state->wl_pointer == NULL) {
        platform_state->wl_pointer =
            wl_seat_get_pointer(platform_state->wl_seat);
        wl_pointer_add_listener(
            platform_state->wl_pointer, &wl_pointer_listener, platform_state
        );
    } else if (!have_pointer && platform_state->wl_pointer != NULL) {
        wl_pointer_release(platform_state->wl_pointer);
        platform_state->wl_pointer = NULL;
    }

    bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (have_keyboard && platform_state->wl_keyboard == NULL) {
        platform_state->wl_keyboard =
            wl_seat_get_keyboard(platform_state->wl_seat);
        wl_keyboard_add_listener(
            platform_state->wl_keyboard, &wl_keyboard_listener, platform_state
        );
    } else if (!have_keyboard && platform_state->wl_keyboard != NULL) {
        wl_keyboard_release(platform_state->wl_keyboard);
        platform_state->wl_keyboard = NULL;
    }

    bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

    if (have_touch && platform_state->wl_touch == NULL) {
        platform_state->wl_touch = wl_seat_get_touch(platform_state->wl_seat);
        wl_touch_add_listener(
            platform_state->wl_touch, &wl_touch_listener, platform_state
        );
    } else if (!have_touch && platform_state->wl_touch != NULL) {
        wl_touch_release(platform_state->wl_touch);
        platform_state->wl_touch = NULL;
    }
}

static void
wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
    // do nothing !
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name         = wl_seat_name,
};
//////// END OF wl_seat EVENT LISTENER CALLBACKS ////////

//////// xdg_wm_base EVENT LISTENER CALLBACKS ////////
static void xdg_wm_base_ping_handler(
    void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial
) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping_handler,
};
//////// END OF xdg_wm_base EVENT LISTENER CALLBACKS ////////

//////// WAYLAND REGISTRY LISTENER CALLBACKS ////////
static void wl_registry_global_object_announcement_handler(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface_string,
    uint32_t version
) {
    WaylandPlatformState *state = (WaylandPlatformState *)data;
    if (strcmp(interface_string, wl_compositor_interface.name) == 0) {
        state->compositor = (struct wl_compositor *)wl_registry_bind(
            registry, name, &wl_compositor_interface, version
        );
    } /*else if (strcmp(interface_string, wl_shm_interface.name) == 0) {
        state->shm = (struct wl_shm *)wl_registry_bind(
            registry, name, &wl_shm_interface, version);
    }*/
    else if (strcmp(interface_string, xdg_wm_base_interface.name) == 0) {
        state->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
            registry, name, &xdg_wm_base_interface, version
        );
        xdg_wm_base_add_listener(
            state->xdg_wm_base, &xdg_wm_base_listener, state
        );
    } else if (strcmp(interface_string, wl_seat_interface.name) == 0) {
        state->wl_seat = (struct wl_seat *)wl_registry_bind(
            registry, name, &wl_seat_interface, version
        );
        wl_seat_add_listener(state->wl_seat, &wl_seat_listener, state);
    } else if (
        strcmp(interface_string, zxdg_decoration_manager_v1_interface.name) == 0
    ) {
        state->xdg_decoration_manager =
            (struct zxdg_decoration_manager_v1 *)wl_registry_bind(
                registry, name, &zxdg_decoration_manager_v1_interface, version
            );
    } else if (
        strcmp(
            interface_string, zwp_relative_pointer_manager_v1_interface.name
        ) == 0
    ) {
        state->relative_pointer_manager =
            (struct zwp_relative_pointer_manager_v1 *)wl_registry_bind(
                registry,
                name,
                &zwp_relative_pointer_manager_v1_interface,
                version
            );
    } else if (
        strcmp(interface_string, zwp_pointer_constraints_v1_interface.name) == 0
    ) {
        state->pointer_constraint_manager =
            (struct zwp_pointer_constraints_v1 *)wl_registry_bind(
                registry, name, &zwp_pointer_constraints_v1_interface, version
            );
    }
}

static void wl_registry_global_object_remove_handler(
    void *data, struct wl_registry *registry, uint32_t name
) {
    // TODO: handle this later
    verbose("\nregistry global object remove event recieved!");
}

static const struct wl_registry_listener registry_listener = {
    .global        = wl_registry_global_object_announcement_handler,
    .global_remove = wl_registry_global_object_remove_handler,
};
//////// END OF WAYLAND REGISTRY LISTENER CALLBACKS ////////

//////// xdg_surface EVENT LISTENER CALLBACKS ////////
static void xdg_surface_configure_event_handler(
    void *data, struct xdg_surface *xdg_surface, uint32_t serial
) {
    verbose("xdg surface configure called\n");
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}
static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_event_handler,
};
//////// END OF xdg_surface EVENT LISTENER CALLBACKS ////////

//////// xdg_toplevel EVENT LISTENER CALLBACKS ////////
static void xdg_toplevel_configure_event_handler(
    void *data,
    struct xdg_toplevel *xdg_toplevel,
    int32_t width,
    int32_t height,
    struct wl_array *states
) {

    verbose("xdg toplevel configure called\n");

    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;

    /*state->pending.activated = false;*/
    /*state->pending.maximized = false;*/
    /**/
    /*enum xdg_toplevel_state *st;*/
    /*for (st = (enum xdg_toplevel_state *)(states)->data;*/
    /*     (states)->size != 0 &&*/
    /*     (const char *)st < ((const char *)(states)->data + (states)->size);*/
    /*     (st)++) {*/
    /*    if (*st == XDG_TOPLEVEL_STATE_MAXIMIZED) {*/
    /*        verbose("xdg toplevel MAXIMIZED recieved !!\n");*/
    /*        state->pending.maximized = true;*/
    /*    }*/
    /*    if (*st == XDG_TOPLEVEL_STATE_FULLSCREEN)*/
    /*        state->fullscreen = true;*/
    /*    if (*st == XDG_TOPLEVEL_STATE_ACTIVATED) {*/
    /*        verbose("xdg toplevel ACTIVATED recieved !!\n");*/
    /*        state->pending.activated = true;*/
    /*    }*/
    /*    if (*st == XDG_TOPLEVEL_STATE_SUSPENDED) {*/
    /*        verbose("xdg toplevel SUSPENDED recieved !!\n");*/
    /*    }*/
    /*    if (*st == XDG_TOPLEVEL_STATE_RESIZING) {*/
    /*        verbose("xdg toplevel RESIZING recieved !!\n");*/
    /*    }*/
    /*}*/

    if (width && height) {
        focused_window->width  = width;
        focused_window->height = height;

        if (focused_window->callbacks.resize)
            focused_window->callbacks.resize(focused_window);
    }
}

static void xdg_toplevel_close_event_handler(
    void *data, struct xdg_toplevel *xdg_toplevel
) {
    verbose("\nclose button clicked!");
    WaylandPlatformState *platform_state = (WaylandPlatformState *)data;
    WaylandWindow *focused_window        = platform_state->focused_window;
    focused_window->shouldClose          = true;
    if (focused_window->callbacks.window_close) {
        focused_window->callbacks.window_close();
    }
}

static void wm_capabilities_broadcast_event_handler(
    void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities
) {
    // Process the list of cabapilities supported by the compositor
    // and enable or disable the equivalent UI features.
    verbose("xdg_toplevel::wm_capabilities_broadcast event fired!\n");
}

static void xdg_toplevel_configure_bounds_event_handler(
    void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height
) {
    // TODO: handle later !
}

static const struct xdg_toplevel_listener xdg_toplevel_events_listener = {
    .configure        = xdg_toplevel_configure_event_handler,
    .close            = xdg_toplevel_close_event_handler,
    .configure_bounds = xdg_toplevel_configure_bounds_event_handler,
    .wm_capabilities  = wm_capabilities_broadcast_event_handler,
};
//////// END OF xdg_toplevel EVENT LISTENER CALLBACKS ////////

///////////////////////////////////////////////////////
//////// BEGINING OF PUBLIC API IMPLEMENTATION ////////
///////////////////////////////////////////////////////

// Platform lifetime management API implementation:
int wayland_platform_initialize_s(WaylandPlatformState *wlPlatformState) {
    *wlPlatformState            = (WaylandPlatformState){0};
    wlPlatformState->connection = wl_display_connect(nullptr);
    if (!wlPlatformState->connection) {
        verbose("Failed to connect to the wayland compositor!\n");
        return 0;
    }
    wlPlatformState->registry =
        wl_display_get_registry(wlPlatformState->connection);
    wlPlatformState->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    wl_registry_add_listener(
        wlPlatformState->registry, &registry_listener, wlPlatformState
    );
    wl_display_roundtrip(wlPlatformState->connection);

    wlPlatformState->initialized    = true;
    wlPlatformState->heap_allocated = false;
    wlPlatformState->focused_window = nullptr;
    wlPlatformState->window_array   = nullptr;

    wlPlatformState->locked_pointer = nullptr;

    _wayland_set_platform_global_state(wlPlatformState);
    return 0;
}

WaylandPlatformState *wayland_platform_initialize_h() {
    WaylandPlatformState *wlPlatformState = calloc(1, sizeof(*wlPlatformState));
    _wayland_set_platform_global_state(wlPlatformState);

    wlPlatformState->connection = wl_display_connect(nullptr);
    if (!wlPlatformState->connection) {
        verbose("Failed to connect to the wayland compositor!\n");
        return nullptr;
    }
    wlPlatformState->registry =
        wl_display_get_registry(wlPlatformState->connection);
    wlPlatformState->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    wl_registry_add_listener(
        wlPlatformState->registry, &registry_listener, wlPlatformState
    );
    wl_display_roundtrip(wlPlatformState->connection);

    wlPlatformState->initialized    = true;
    wlPlatformState->heap_allocated = true;
    return wlPlatformState;
}

void wayland_platform_shutdown(WaylandPlatformState *wlPlatformState) {
    zxdg_decoration_manager_v1_destroy(wlPlatformState->xdg_decoration_manager);

    xdg_wm_base_destroy(wlPlatformState->xdg_wm_base);

    if (wlPlatformState->relative_pointer)
        zwp_relative_pointer_v1_destroy(wlPlatformState->relative_pointer);
    if (wlPlatformState->relative_pointer_manager)
        zwp_relative_pointer_manager_v1_destroy(
            wlPlatformState->relative_pointer_manager
        );

    zwp_pointer_constraints_v1_destroy(
        wlPlatformState->pointer_constraint_manager
    );

    if (wlPlatformState->wl_keyboard)
        wl_keyboard_release(wlPlatformState->wl_keyboard);
    if (wlPlatformState->wl_pointer)
        wl_pointer_release(wlPlatformState->wl_pointer);
    if (wlPlatformState->wl_touch) wl_touch_release(wlPlatformState->wl_touch);
    wl_seat_release(wlPlatformState->wl_seat);

    wl_compositor_destroy(wlPlatformState->compositor);
    wl_registry_destroy(wlPlatformState->registry);
    wl_display_disconnect(wlPlatformState->connection);
    wlPlatformState->initialized = false;
}

// Window creation-destruction API implementation:
int wayland_create_window_s(WaylandWindow *window) {
    WaylandPlatformState *wlPlatformState =
        _wayland_get_platform_global_state();
    dyn_arr_push(wlPlatformState->window_array, window);

    // TODO: Should we immediately register a newly created window
    // as focused ?
    wlPlatformState->focused_window = window;

    window->wl_surface =
        wl_compositor_create_surface(wlPlatformState->compositor);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(
        wlPlatformState->xdg_wm_base, window->wl_surface
    );
    xdg_surface_add_listener(
        window->xdg_surface, &xdg_surface_listener, wlPlatformState
    );
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(
        window->xdg_toplevel, &xdg_toplevel_events_listener, wlPlatformState
    );
    xdg_toplevel_set_title(window->xdg_toplevel, window->title);
    xdg_toplevel_set_app_id(window->xdg_toplevel, "wl_test_client");
    window->xdg_toplevel_decoration =
        zxdg_decoration_manager_v1_get_toplevel_decoration(
            wlPlatformState->xdg_decoration_manager, window->xdg_toplevel
        );

    wl_surface_commit(window->wl_surface);
    wl_display_roundtrip(wlPlatformState->connection);

    window->initialized    = true;
    window->heap_allocated = false;
    return 0;
}

WaylandWindow *
wayland_create_window_h(uint32_t width, uint32_t height, const char *title) {
    WaylandPlatformState *wlPlatformState =
        _wayland_get_platform_global_state();
    WaylandWindow *window = calloc(1, sizeof(*window));

    dyn_arr_push(wlPlatformState->window_array, window);
    // TODO: Should we immediately register a newly created window
    // as focused ?
    wlPlatformState->focused_window = window;

    window->width       = width;
    window->height      = height;
    window->title       = title;
    window->isResizable = true;
    window->shouldClose = false;

    window->wl_surface =
        wl_compositor_create_surface(wlPlatformState->compositor);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(
        wlPlatformState->xdg_wm_base, window->wl_surface
    );
    xdg_surface_add_listener(
        window->xdg_surface, &xdg_surface_listener, wlPlatformState
    );
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(
        window->xdg_toplevel, &xdg_toplevel_events_listener, wlPlatformState
    );
    xdg_toplevel_set_title(window->xdg_toplevel, title);
    xdg_toplevel_set_app_id(window->xdg_toplevel, "wl_test_client");
    window->xdg_toplevel_decoration =
        zxdg_decoration_manager_v1_get_toplevel_decoration(
            wlPlatformState->xdg_decoration_manager, window->xdg_toplevel
        );
    wl_surface_commit(window->wl_surface);
    wl_display_roundtrip(wlPlatformState->connection);

    window->initialized    = true;
    window->heap_allocated = true;
    return window;
}

void wayland_destroy_window(WaylandWindow *window) {
    WaylandPlatformState *wlPlatformState =
        _wayland_get_platform_global_state();

    zxdg_toplevel_decoration_v1_destroy(window->xdg_toplevel_decoration);
    xdg_toplevel_destroy(window->xdg_toplevel);
    xdg_surface_destroy(window->xdg_surface);
    wl_surface_destroy(window->wl_surface);
    window->initialized = false;

    // swap remove but while searching item backwards:
    size_t last_index = dyn_arr_last_index(wlPlatformState->window_array);
    for (signed i = last_index; i >= 0; i--) {
        if (wlPlatformState->window_array[i] == window) {
            if (i == last_index) {
                dyn_arr_pop(wlPlatformState->window_array);
            } else {
                wlPlatformState->window_array[i] =
                    wlPlatformState->window_array[last_index];
                dyn_arr_pop(wlPlatformState->window_array);
            }
        }
    }
}

// Window management API implementation:
void wayland_show_window(WaylandWindow *window) { return; }
void wayland_set_window_title(WaylandWindow *window, const char *title) {
    xdg_toplevel_set_title(window->xdg_toplevel, title);
}
void wayland_set_window_max_size(
    WaylandWindow *window, uint32_t width, uint32_t height
) {
    xdg_toplevel_set_max_size(window->xdg_toplevel, width, height);
}
void wayland_set_window_min_size(
    WaylandWindow *window, uint32_t width, uint32_t height
) {
    xdg_toplevel_set_min_size(window->xdg_toplevel, width, height);
}
void wayland_set_window_fullscreen(WaylandWindow *window, bool flag) {
    if (window->isFullscreen && flag) return;
    if (!window->isFullscreen && !flag) return;
    if (flag) {
        xdg_toplevel_set_fullscreen(window->xdg_toplevel, nullptr);
    } else {
        xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
    }
}
void wayland_set_window_borderless(WaylandWindow *window, bool flag) {}
void wayland_make_window_unresizable(WaylandWindow *window, bool flag) {}

// Event polling API implementation:
void wayland_poll_event() {
    double timeout = 0.0;
    _wl_poll_events(&timeout);
}

void wayland_wait_for_event() { _wl_poll_events(nullptr); }

void wayland_wait_for_event_till(double timeout) { _wl_poll_events(&timeout); }

// Input state query API implementation:
KeyState get_key_state(WaylandWindow *window, KeyCode key) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return ps->inputState.ks.keyState[key];
}
KeyState get_mouse_button_state(WaylandWindow *window, MouseButtonCode btn) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return ps->inputState.ps.mouseButtonState[btn];
}

bool is_key_pressed(WaylandWindow *window, KeyCode key) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return (ps->inputState.ks.keyState[key] == PRESSED);
}
bool is_key_released(WaylandWindow *window, KeyCode key) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    // TODO: Implement later.
    return false;
}

bool is_mouse_button_pressed(WaylandWindow *window, MouseButtonCode btn) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return (ps->inputState.ps.mouseButtonState[btn] == PRESSED);
}
bool is_mouse_button_released(WaylandWindow *window, MouseButtonCode btn) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    // TODO: Implement later.
    return false;
}

void get_mouse_pos(WaylandWindow *window, float *x, float *y) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    *x                       = ps->inputState.ps.position.x;
    *y                       = ps->inputState.ps.position.y;
}
float get_mouse_x(WaylandWindow *window) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return ps->inputState.ps.position.x;
}
float get_mouse_y(WaylandWindow *window) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    return ps->inputState.ps.position.y;
}

// Pointer management API implementation:
void wayland_set_pointer_lock(WaylandWindow *window, bool lock) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    if (lock && !ps->locked_pointer) {
        ps->locked_pointer = zwp_pointer_constraints_v1_lock_pointer(
            ps->pointer_constraint_manager,
            window->wl_surface,
            ps->wl_pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
        );
        zwp_locked_pointer_v1_set_cursor_position_hint(
            ps->locked_pointer,
            wl_fixed_from_int(window->width / 2),
            wl_fixed_from_int(window->height / 2)
        );
    } else if (!lock && ps->locked_pointer) {
        zwp_locked_pointer_v1_destroy(ps->locked_pointer);
        ps->locked_pointer = nullptr;
    }
}
void wayland_set_pointer_visibility(WaylandWindow *window, bool visibility) {}

// Toggle raw mouse motion feed API implementation:
void wayland_set_raw_mouse_motion_enabled(bool enabled) {
    WaylandPlatformState *ps = _wayland_get_platform_global_state();
    if (ps->relative_pointer && enabled) return;
    if (!ps->relative_pointer && !enabled) return;
    if (enabled) {
        ps->relative_pointer =
            zwp_relative_pointer_manager_v1_get_relative_pointer(
                ps->relative_pointer_manager, ps->wl_pointer
            );
        zwp_relative_pointer_v1_add_listener(
            ps->relative_pointer, &relative_pointer_listener, ps
        );
    } else {
        zwp_relative_pointer_v1_destroy(ps->relative_pointer);
        zwp_relative_pointer_manager_v1_destroy(ps->relative_pointer_manager);
    }
}

// Clipboard handling API implementation:
void wayland_push_text_to_clipboard(const char *text) {}
const char *wayland_get_text_from_clipboard() { return nullptr; }

//////// wayland platform specific public API ////////

//////// wl_callback EVENT LISTENER CALLBACKS ////////
void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time);
struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done,
};
void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time) {
    if (cb) wl_callback_destroy(cb);

    struct WaylandWindow *window = (struct WaylandWindow *)data;
    window->wl_surface_frame_done_callback =
        wl_surface_frame(window->wl_surface);
    wl_callback_add_listener(
        window->wl_surface_frame_done_callback,
        &wl_surface_frame_listener,
        window
    );

    double time_secs = time * 1e-3f;
    window->callbacks.render(window, time_secs);
    /*wl_surface_commit(window->wl_surface);*/
    window->last_frame_time = time_secs;
}
//////// END OF wl_callback EVENT LISTENER CALLBACKS ////////

void wayland_trigger_frame(WaylandWindow *window) {
    wl_surface_frame_done(window, nullptr, 0);
}
//////// end of wayland platform specific public API ////////

///////////////////////////////////////////////////////
///////    END OF PUBLIC API IMPLEMENTATION    ////////
///////////////////////////////////////////////////////
