#pragma once

#include <print>

#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace {
enum class PollResult {
    ERROR,
    TIMEOUT,
    NEW_DATA,
    COUNT,
};

PollResult handle_poll_result(int poll_result, short revents)
{
    switch (poll_result) {
    case -1:
        std::println(stderr, "Poll error {}: {}", errno, strerror(errno));
        return PollResult::ERROR;
    case 0:
        std::println(stderr, "No new clients, stopping server loop");
        return PollResult::TIMEOUT;
    default:
        if (revents & (POLLHUP | POLLERR)) {
            std::println(stderr, "Pipe closed or error detected during grace period");
            return PollResult::ERROR;
        }
        if (revents & POLLIN) {
            std::println(stderr, "New data detected, continuing server loop");
        }
        return PollResult::NEW_DATA;
    }
}
}

namespace IPC {

/// @brief Monitors a named pipe for incoming data without consuming it.
///
/// Opens a separate read-only file descriptor to poll for data availability.
/// This allows checking if new clients are connecting during a grace period
/// without interfering with the main reader's buffered stream operations.
/// The poll() call only detects data presence - actual reading happens elsewhere.
class PipePeeker {
public:
    PipePeeker(const std::string& pipe_name, std::uint16_t timeout_ms)
        : m_pipe_fd(open(pipe_name.c_str(), O_RDONLY | O_NONBLOCK))
        , m_timeout_ms(timeout_ms)
    {
    }

    ~PipePeeker()
    {
        if (m_pipe_fd >= 0)
            close(m_pipe_fd);
    }

    PollResult peek()
    {
        std::println(stderr, "Waiting {}ms for new data...", m_timeout_ms);

        if (m_pipe_fd < 0) {
            std::println(stderr, "Failed to open pipe file descriptor: {}", strerror(errno));
            return PollResult::ERROR;
        }

        struct pollfd poll_request {
            .fd = m_pipe_fd, // File descriptor to poll.
            .events = POLLIN, // Types of events poller cares about.
            .revents = 0, // Types of events that actually occurred.
        };
        return handle_poll_result(poll(&poll_request, 1, m_timeout_ms), poll_request.revents);
    }

private:
    int m_pipe_fd;
    uint16_t m_timeout_ms;
};
} // namespace IPC
