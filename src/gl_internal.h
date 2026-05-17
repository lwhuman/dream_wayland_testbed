#ifndef ABSTRACT_GL_INTERNAL_H
#define ABSTRACT_GL_INTERNAL_H

#include "wl_window.h"
#include <EGL/egl.h>

typedef struct GLDevice {
    EGLContext context;
    EGLConfig config;
    bool debug;
} GLDevice;

typedef struct GLPresentable {
    GLDevice *device;
    WaylandWindow *window;
    EGLSurface surface;
} GLPresentable;

void enable_gl_debug_output();

#endif // !ABSTRACT_GL_INTERNAL_H
