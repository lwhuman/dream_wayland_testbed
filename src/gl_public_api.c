#include "gl_public_api.h"
#include "gl_internal.h"
#include "wl_internal_api.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <stdlib.h>
#include <wayland-egl-core.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define VERBOSE
#include "utility.h"

static inline bool _load_gl_func() {
    if (!gladLoadGL((GLADloadfunc)eglGetProcAddress)) {
        ERROR("\n[GLAD]: Failed to load opengl functions!");
        return false;
    }
    return true;
}

static inline bool egl_initialize() {
    WaylandPlatformState *p = _wayland_get_platform_global_state();

    // Initialize EGL
    p->egl_display = eglGetDisplay((EGLNativeDisplayType)p->connection);
    if (p->egl_display == EGL_NO_DISPLAY) {
        debug(
            "\nFailed to get direct "
            "EGL Display interface "
            "to our Wayland connection !"
        );
        // Fallback
        p->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (p->egl_display == EGL_NO_DISPLAY) {
            debug(
                "\nFailed to get EGL Display "
                "interface to default Wayland connection!"
            );
            ERROR("\nEGL not supported by the current drivers!");
            return false;
        }
    }

    EGLint egl_major, egl_minor;
    if (!eglInitialize(p->egl_display, &egl_major, &egl_minor)) {
        EGLint err = eglGetError();
        ERROR("eglInitialize failed: 0x%x", err);
        ERROR("Failed to initialize EGL!");
        return false;
    }
    // verbose("EGL Version: %d.%d", egl_major, egl_minor);

    p->egl_initialized = true;

    return true;
}

static inline EGLConfig egl_choose_config(EGLDisplay display) {
    static const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_BIT,
        EGL_CONFIG_CAVEAT,
        EGL_NONE,
        EGL_RED_SIZE,
        8,
        EGL_GREEN_SIZE,
        8,
        EGL_BLUE_SIZE,
        8,
        EGL_ALPHA_SIZE,
        8,
        EGL_DEPTH_SIZE,
        24,
        EGL_STENCIL_SIZE,
        8,
        EGL_SAMPLES,
        0,
        EGL_NONE
    };

    EGLConfig config;

    EGLint no_of_matching_configs;
    if (!eglChooseConfig(
            display, config_attribs, &config, 1, &no_of_matching_configs
        )) {
        ERROR("Failed to get valid EGL config!\n");
    }

    if (no_of_matching_configs == 0) {
        ERROR("No matching EGL config found!");
        return nullptr;
    }

    return config;
}

GLDevice *gl_device_create(const GLDeviceDesc *desc) {
    WaylandPlatformState *p = _wayland_get_platform_global_state();

    GLDevice *d = calloc(1, sizeof(*d));

    if (!p->egl_initialized) {
        bool egl_init_status = egl_initialize();
        if (!egl_init_status) {
            goto fail;
        }
    }

    EGLDisplay display = p->egl_display;

    d->config = egl_choose_config(display);
    if (!d->config) goto fail;

    eglBindAPI(EGL_OPENGL_API);

    EGLint ctx_attribs[16];
    int i = 0;

    ctx_attribs[i++] = EGL_CONTEXT_MAJOR_VERSION;
    ctx_attribs[i++] = desc->opengl_major_version;
    ctx_attribs[i++] = EGL_CONTEXT_MINOR_VERSION;
    ctx_attribs[i++] = desc->opengl_minor_version;

    ctx_attribs[i++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
    ctx_attribs[i++] = (desc->opengl_core_profile)
                           ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
                           : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;

    ctx_attribs[i++] = EGL_CONTEXT_OPENGL_DEBUG;
    ctx_attribs[i++] = (desc->debug) ? EGL_TRUE : EGL_FALSE;

    ctx_attribs[i++] = EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE;
    ctx_attribs[i++] = (desc->forward_compatible) ? EGL_TRUE : EGL_FALSE;

    ctx_attribs[i++] = EGL_NONE;

    d->context =
        eglCreateContext(display, d->config, EGL_NO_CONTEXT, ctx_attribs);
    if (d->context == EGL_NO_CONTEXT) {
        ERROR("\nFailed to create EGL OpenGL context!");
        goto fail;
    }

    d->debug = desc->debug;

    // Load opengl functions
    EGLSurface dummySurface = eglCreatePbufferSurface(
        display, d->config, (EGLint[]){EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE}
    );
    eglMakeCurrent(display, dummySurface, dummySurface, d->context);

    bool load_success = _load_gl_func();

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, dummySurface);

    if (!load_success) goto fail;

    return d;

fail:
    free(d);
    return nullptr;
}

void gl_device_destroy(GLDevice *dev) {
    if (!dev) return;

    WaylandPlatformState *p = _wayland_get_platform_global_state();

    eglDestroyContext(p->egl_display, dev->context);
    free(dev);
}

GLPresentable *gl_presentable_create(GLDevice *device, WaylandWindow *window) {
    GLPresentable *presentable = calloc(1, sizeof(*presentable));

    presentable->device = device;
    presentable->window = window;

    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    window->egl_window =
        wl_egl_window_create(window->wl_surface, window->width, window->height);

    const EGLint egl_surface_attribs[] = {
        EGL_GL_COLORSPACE,
        EGL_GL_COLORSPACE_LINEAR,
        EGL_RENDER_BUFFER,
        EGL_BACK_BUFFER,
        EGL_NONE,
    };

    presentable->surface = eglCreateWindowSurface(
        display,
        device->config,
        (EGLNativeWindowType)window->egl_window,
        egl_surface_attribs
    );

    if (presentable->surface == EGL_NO_SURFACE) {
        EGLint err = eglGetError();
        ERROR("eglCreateWindowSurface failed: 0x%x", err);
        ERROR("Failed to create EGL window surface\n");
        free(p);
        return nullptr;
    }

    return presentable;
}

void gl_presentable_destroy(GLPresentable *presentable) {
    if (!presentable) return;

    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    eglDestroySurface(display, presentable->surface);

    free(presentable);
}

void gl_presentable_recreate(GLPresentable *presentable) {
    if (!presentable) return;

    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    // Unbind surface if currently bound
    EGLContext current = eglGetCurrentContext();
    if (current == presentable->device->context) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    // Destroy old surface
    if (presentable->surface != EGL_NO_SURFACE) {
        eglDestroySurface(display, presentable->surface);
        presentable->surface = EGL_NO_SURFACE;
    }

    // Recreate the surface
    const EGLint egl_surface_attribs[] = {
        EGL_GL_COLORSPACE,
        EGL_GL_COLORSPACE_LINEAR,
        EGL_RENDER_BUFFER,
        EGL_BACK_BUFFER,
        EGL_NONE,
    };
    presentable->surface = eglCreateWindowSurface(
        display,
        presentable->device->config,
        (EGLNativeWindowType)presentable->window->egl_window,
        egl_surface_attribs
    );

    if (presentable->surface == EGL_NO_SURFACE) {
        ERROR("\nFailed to recreate EGL surface after resize!");
        return;
    }

    // Rebind context
    eglMakeCurrent(
        display,
        presentable->surface,
        presentable->surface,
        presentable->device->context
    );

    // TODO: see below:
    // Should we update GL viewport from here ?
    // glViewport(
    //     0,
    //     0,
    //     (GLsizei)p->window->width,
    //     (GLsizei)p->window->height
    // );
}

void gl_presentable_set_swap_interval(
    GLPresentable *presentable, GLSwapInterval interval
) {
    if (!presentable) return;

    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    eglSwapInterval(display, (EGLint)interval);
}

void gl_make_current(GLDevice *device, GLPresentable *target) {
    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    eglMakeCurrent(display, target->surface, target->surface, device->context);

    static bool debug_initialized = false;

    if (!debug_initialized && device->debug) {
        enable_gl_debug_output();
        debug_initialized = true;
    }
}

void gl_present(GLPresentable *target) {
    WaylandPlatformState *p = _wayland_get_platform_global_state();

    EGLDisplay display = p->egl_display;

    eglSwapBuffers(display, target->surface);
}
