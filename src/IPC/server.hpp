#pragma once

#include <print>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

/// @brief Concept for types that can be read as IPC messages
/// @tparam T The type to check
template <typename T>
concept IPCMessage = requires(T msg) {
    { msg.length } -> std::convertible_to<size_t>;
    { msg.body } -> std::convertible_to<const char*>;
    requires std::is_array_v<decltype(T::body)>;
};

class PipeServer {
public:
    PipeServer(std::string path)
        : m_pid(getpid())
        , m_pipename(std::move(path))
    {
    }

    [[nodiscard]]
    std::optional<FileDescriptor> init()
    {
        // Create the named pipe (fifo) with permission
        const auto result = mkfifo(m_pipename.data(), 0666);
        if (result < 0 && errno != EEXIST) {
            std::println("Error when creating FIFO. {}.", strerror(errno));
            return std::nullopt;
        }

        // Try to open pipe for read only
        m_file_descriptor = open(m_pipename.data(), O_RDONLY);
        if (m_file_descriptor < 0 && errno == ENOENT) {
            std::println("PID {}: Something went wrong creating the pipe", m_pid);
            return std::nullopt;
        }
        return m_file_descriptor;
    }

    /// @brief Read a message from the pipe with proper handling of variable-length messages
    /// @tparam MessageType The message type (must satisfy IPCMessage concept)
    /// @param msg Reference to message structure to fill
    /// @return true if message was read successfully, false otherwise
    template <IPCMessage MessageType>
    [[nodiscard]] std::optional<MessageType> read_message()
    {
        MessageType msg {};

        // First, read the header to determine the message length
        constexpr size_t header_size = offsetof(MessageType, body);
        if (read(m_file_descriptor, &msg, header_size) < static_cast<ssize_t>(header_size)) {
            std::println(stderr, "PID {}; Error reading msg header. {}.", m_pid, strerror(errno));
            return std::nullopt;
        }

        // Validate and handle oversized messages
        if (msg.length > sizeof(msg.body)) {
            std::println(stderr, "PID {}; Invalid msg length: {} (max: {}), skipping {} bytes",
                m_pid, msg.length, sizeof(msg.body), msg.length - sizeof(msg.body));

            // Discard excess bytes in chunks
            for (size_t remaining = msg.length - sizeof(msg.body); remaining > 0;) {
                const size_t chunk = std::min(remaining, sizeof(msg.body));
                read(m_file_descriptor, msg.body, chunk);
                remaining -= chunk;
            }
            msg.length = sizeof(msg.body);
        }

        // Read the body content
        if (msg.length > 0 && read(m_file_descriptor, msg.body, msg.length) < static_cast<ssize_t>(msg.length)) {
            std::println(stderr, "PID {}; Error reading msg body. {}.", m_pid, strerror(errno));
            return std::nullopt;
        }

        return msg;
    }

    ~PipeServer()
    {
        if (close(m_file_descriptor) != 0) {
            std::println("Process {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
        if (unlink(m_pipename.data()) != 0) {
            std::println("Process {}: Error while unlinking pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipename;
    FileDescriptor m_file_descriptor { -1 };
};

}
