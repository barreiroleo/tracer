#pragma once

#include "message.hpp"
#include <filesystem>
#include <fstream>
#include <mutex>
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
    PipeClient(std::string path)
        : m_pid(getpid())
        , m_pipe_path(std::move(path))
    {
    }

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ofstream>> init()
    {
        std::once_flag retried;
        while (!std::filesystem::exists(m_pipe_path)) {
            std::call_once(retried, [&] {
                std::println(stderr, "Waiting for pipe to be created at {}...", m_pipe_path);
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
            std::println(stderr, "Process {}: Error while writing message. {}", m_pid, strerror(errno));
            return false;
        }
        return true;
    }

    ~PipeClient()
    {
        m_pipe_stream.close();
        if (m_pipe_stream.fail()) {
            std::println(stderr, "Process {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipe_path;
    std::ofstream m_pipe_stream;
};

}
