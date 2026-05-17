#include "gl_internal.h"

#include <glad/gl.h>
#include <stdio.h>

static void _builtin_debug_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *user_param
) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    const char *src = "UNKNOWN";
    const char *typ = "UNKNOWN";
    const char *sev = "UNKNOWN";

    switch (source) {
        case GL_DEBUG_SOURCE_API:             src = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "WINDOW"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "SHADER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "3RD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     src = "APP"; break;
        case GL_DEBUG_SOURCE_OTHER:           src = "OTHER"; break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typ = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typ = "DEPRECATED"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typ = "UNDEFINED"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typ = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typ = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER:              typ = "MARKER"; break;
        case GL_DEBUG_TYPE_OTHER:               typ = "OTHER"; break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   sev = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: sev = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:    sev = "LOW"; break;
    }

    fprintf(stderr, "\n[GL][%s][%s][%s][%u]: %s", src, typ, sev, id, message);
}

// TODO: Expose API to allow users to register their
// custom debug callback function as glDebugMessageCallback
void enable_gl_debug_output() {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT)) return;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(_builtin_debug_callback, nullptr);
    glDebugMessageControl(
        GL_DONT_CARE,
        GL_DONT_CARE,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        0,
        nullptr,
        GL_FALSE
    );
}
