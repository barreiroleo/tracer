#pragma once

#include <optional>
#include <print>
#include <string>
#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

/// @brief Concept for types that can be written as IPC messages
/// @tparam T The type to check
template <typename T>
concept IPCMessage = requires(T msg) {
    { msg.length } -> std::convertible_to<size_t>;
    { msg.body } -> std::convertible_to<const char*>;
    requires std::is_array_v<decltype(T::body)>;
    { msg.size() } -> std::convertible_to<size_t>;
};

class PipeClient {
public:
    PipeClient(std::string path)
        : m_pid(getpid())
        , m_pipename(std::move(path))
    {
    }

    [[nodiscard]]
    std::optional<FileDescriptor> init()
    {
        // Try to open pipe for writing only
        m_file_descriptor = open(m_pipename.data(), O_WRONLY);
        while (m_file_descriptor < 0 && errno == ENOENT) {
            std::println("Process {}: Pipe not found. Retrying...", m_pid);
            m_file_descriptor = open(m_pipename.data(), O_WRONLY);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return m_file_descriptor;
    }

    /// @brief Write a message to the pipe
    /// @tparam MessageType The message type (must satisfy IPCMessage concept)
    /// @param msg Reference to message to send
    /// @return true if message was written successfully, false otherwise
    template <IPCMessage MessageType>
    [[nodiscard]] bool write_message(const MessageType& msg)
    {
        const auto bytes_written = write(m_file_descriptor, &msg, msg.size());
        if (bytes_written < 0) {
            std::println(stderr, "Process {}: Error while writing message. {}", m_pid, strerror(errno));
            return false;
        }
        return true;
    }

    ~PipeClient()
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
