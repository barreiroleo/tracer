#pragma once

#include "message.hpp"
#include <filesystem>
#include <fstream>
#include <optional>
#include <print>
#include <string>
#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

class PipeClient {
public:
    PipeClient(std::string_view path)
        : m_pid(getpid())
        , m_pipe_path(path)
    {
    }

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ofstream>> init()
    {
        namespace fs = std::filesystem;
        if (!fs::exists(m_pipe_path)) {
            std::println("Waiting for pipe to be created at {}...", m_pipe_path);
            while (!fs::exists(m_pipe_path)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        m_pipe_stream.open(m_pipe_path.data(), std::ios::binary | std::ios::out);
        if (!m_pipe_stream.is_open() || m_pipe_stream.fail()) {
            std::println(stderr, "Failed to open pipe.");
            return std::nullopt;
        }
        return m_pipe_stream;
    }

    [[nodiscard]] bool write_message(const Message& msg)
    {
        m_pipe_stream << msg;
        if (m_pipe_stream.fail()) {
            std::println(stderr, "PID {}: Error while writing message. {}", m_pid, strerror(errno));
            return false;
        }
        return true;
    }

    ~PipeClient()
    {
        m_pipe_stream.close();
        if (m_pipe_stream.fail()) {
            std::println(stderr, "PID {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipe_path;
    std::ofstream m_pipe_stream;
};

}
