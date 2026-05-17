#ifndef ABSTRACT_OPENGL_CONTEXT_H
#define ABSTRACT_OPENGL_CONTEXT_H

#include <EGL/egl.h>
#include <stdint.h>
#include "wl_window.h"

typedef struct GLDevice GLDevice;
typedef struct GLPresentable GLPresentable;

typedef enum GLSwapInterval {
    GL_SWAP_IMMEDIATE = 0,
    GL_SWAP_VSYNC = 1,
} GLSwapInterval;

typedef struct GLDeviceDesc {
    uint8_t opengl_major_version;
    uint8_t opengl_minor_version;
    bool opengl_core_profile;
    bool debug;
    bool forward_compatible;
} GLDeviceDesc;

#ifdef  __cplusplus
extern "C" {
#endif

GLDevice *gl_device_create(const GLDeviceDesc *desc);
void gl_device_destroy(GLDevice *device);

GLPresentable *gl_presentable_create(GLDevice *device, WaylandWindow *window);
void gl_presentable_destroy(GLPresentable *presentable);
void gl_presentable_recreate(GLPresentable *presentable);
void gl_presentable_set_swap_interval(
    GLPresentable *presentable, GLSwapInterval interval
);

void gl_make_current(GLDevice *device, GLPresentable *target);
void gl_present(GLPresentable *target);

#ifdef  __cplusplus
}
#endif

#endif // !ABSTRACT_OPENGL_CONTEXT_H
