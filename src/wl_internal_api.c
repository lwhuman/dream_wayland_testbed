#include "wl_internal_api.h"
#include <errno.h>
#define __USE_GNU
#include "timer/timer_api.h"
#include <assert.h>
#include <errno.h>
#include <poll.h>
// #include <signal.h>
#include <stdint.h>
#include <sys/poll.h>
#include <time.h>

#include <wayland-client-core.h>

static WaylandPlatformState *g_waylandPlatformState;

WaylandPlatformState *_wayland_get_platform_global_state() {
    assert(g_waylandPlatformState && "Wayland platform not initialized!");
    return g_waylandPlatformState;
}

void _wayland_set_platform_global_state(WaylandPlatformState *state) {
    g_waylandPlatformState = state;
}

// for event polling
static int _posixPoll(struct pollfd *fds, nfds_t count, double *timeout) {
    while (1) {
        if (timeout) {
            const uint64_t start = posixGetTime_ns();

            const uint64_t seconds   = (time_t)*timeout;
            const long nanoseconds   = (long)((*timeout - seconds) * 1e9);
            const struct timespec ts = {seconds, nanoseconds};

            const int result = ppoll(fds, count, &ts, nullptr);

            const int poll_err = errno;

            *timeout = *timeout - (posixGetTime_ns() - start) / 1000000000.0;

            if (result > 0)
                return 1;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return 0; // for errno = EINTR and EAGAIN we retry
            else if (*timeout <= 0.0)
                return 0;
        } else { // if timeout is = NULL
            const int result = poll(fds, count, -1);
            if (result > 0)
                return 1;
            else if (result == -1 && errno != EINTR && errno != EAGAIN)
                return 0; // for errno = EINTR and EAGAIN we retry
        }
    }
}

static int _client_display_flush() {
    WaylandPlatformState *wlPlatformState =
        _wayland_get_platform_global_state();
    // we call wl_display_flush till all the buffered data in client side
    // is send to the compositor.
    while (wl_display_flush(wlPlatformState->connection) == -1) {
        // if errno = EAGAIN, we poll the display fd to wait for it to become
        // writable again.
        if (errno != EAGAIN) return 0;

        struct pollfd fd = {
            wl_display_get_fd(wlPlatformState->connection), POLLOUT
        };

        while (poll(&fd, 1, -1) == -1) {
            if (errno != EINTR && errno != EAGAIN) return 0;
        }
    }

    return 1;
}

void _wl_poll_events(double *timeout) {
    WaylandPlatformState *wlPlatformState =
        _wayland_get_platform_global_state();
    int done = 0;

    struct pollfd fd = {wl_display_get_fd(wlPlatformState->connection), POLLIN};

    while (!done) {

        while (wl_display_prepare_read(wlPlatformState->connection) != 0) {
            if (wl_display_dispatch_pending(wlPlatformState->connection) > 0)
                return;
        }

        int flash_status = _client_display_flush();
        if (flash_status == 0) {
            // Broke frozen state
            wl_display_cancel_read(wlPlatformState->connection);

            // TODO: Close the application somehow
            // Maybe: wlClientState->shouldClose = true;
            return;
        }

        int poll_status = _posixPoll(&fd, 1, timeout);
        if (poll_status == 0) {
            wl_display_cancel_read(wlPlatformState->connection);
            return;
        }

        if (fd.revents & POLLIN) {
            wl_display_read_events(wlPlatformState->connection);
            if (wl_display_dispatch_pending(wlPlatformState->connection) > 0)
                done = 1;
        }
    }
}
