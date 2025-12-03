#pragma once

#include "message.hpp"
#include <fstream>
#include <print>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

class PipeServer {
public:
    PipeServer(std::string_view path)
        : m_pid(getpid())
        , m_pipe_name(path)
    {
    }

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ifstream>> init()
    {
        // Create the named pipe (fifo) with permission
        const auto result = mkfifo(m_pipe_name.c_str(), 0666);
        if (result < 0 && errno != EEXIST) {
            std::println(stderr, "Error when creating FIFO. {}.", strerror(errno));
            return std::nullopt;
        }

        m_pipe_stream = std::ifstream(m_pipe_name.c_str(), std::ios::binary | std::ios::in);
        if (!m_pipe_stream.is_open() || m_pipe_stream.fail()) {
            std::println(stderr, "Failed to open pipe for reading.");
            return std::nullopt;
        }
        std::println("Server initialized on pipe: {}", m_pipe_name);
        return m_pipe_stream;
    }

    [[nodiscard]] std::optional<Message> read_message()
    {
        Message msg {};
        m_pipe_stream >> msg;
        if (m_pipe_stream.fail()) {
            std::println(stderr, "Process {}: Error while reading message. {}", m_pid, strerror(errno));
            return std::nullopt;
        }
        return msg;
    }

    ~PipeServer()
    {
        m_pipe_stream.close();
        if (m_pipe_stream.fail()) {
            std::println(stderr, "Process {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
        if (unlink(m_pipe_name.c_str()) != 0) {
            std::println(stderr, "Process {}: Error while unlinking pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipe_name;
    std::ifstream m_pipe_stream;
};

}
