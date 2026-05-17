#include "gl_public_api.h"
#include "wl_public_api.h"

#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl-core.h>

#include <glad/gl.h>

// #define DEBUG 1
#define VERBOSE 1
#include "utility.h"

static void on_resize(struct WaylandWindow *window) {
    /*xdg_surface_set_window_geometry(state->xdg_surface, 0, 0, state->width,
     * state->height);*/
    wl_egl_window_resize(
        window->egl_window, window->width, window->height, 0, 0
    );
    glViewport(0, 0, window->width, window->height);
    glScissor(0, 0, window->width, window->height);
}

// static void draw_frame_gpu(struct WaylandWindow *window, double now) {
//     float r = 0.5f + 0.5f * sin(now * 0.5f);
//     float g = 0.5f + 0.5f * sin(now * 0.7f);
//     float b = 0.5f + 0.5f * sin(now * 0.9f);
//     /*glClearColor(1.0f, 0.0f, 0.0f, 1.0f);*/
//     glClearColor(r, g, b, 1);
//     glClear(GL_COLOR_BUFFER_BIT);
//
//     eglSwapBuffers(window->egl_display, window->egl_surface);
// }

int main(int argc, char **argv) {
    WaylandPlatformState *wlPlaformState = wayland_platform_initialize_h();

    WaylandWindow *window =
        wayland_create_window_h(800, 600, "Wayland testbed");
    wayland_show_window(window);

    window->callbacks.resize = on_resize;
    // window->callbacks.render = draw_frame_gpu;

    GLDeviceDesc desc = {
        .opengl_major_version = 4,
        .opengl_minor_version = 6,
        .opengl_core_profile  = true,
        .debug                = true,
        .forward_compatible   = true,
    };

    GLDevice *gldev = gl_device_create(&desc);
    if (!gldev) {
        ERROR("Failed to create OpenGL context!");
        ERROR("Closing program ...");
        return 1;
    }
    GLPresentable *target = gl_presentable_create(gldev, window);
    if (!target) {
        ERROR("Failed to create presentable!");
        ERROR("Closing program ...");
        return 1;
    }

    gl_make_current(gldev, target);

    gl_presentable_set_swap_interval(target, GL_SWAP_VSYNC);

    uint32_t fc = 0;
    while (!window->shouldClose) {
        verbose("fc=%d", fc++);
        wayland_poll_event();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

        gl_present(target);
    }

    gl_device_destroy(gldev);
    gl_presentable_destroy(target);

    wayland_destroy_window(window);
    wayland_platform_shutdown(wlPlaformState);

    return 0;
}
