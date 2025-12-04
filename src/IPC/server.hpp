#pragma once

#include "message.hpp"
#include <fstream>
#include <functional>
#include <print>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;
using MessageHandler = std::function<void(IPC::Message)>;
using StopHandler = std::function<void()>;

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

    void run(MessageHandler message_handler, StopHandler stop_handler)
    {
        std::optional<IPC::Message> msg;
        while (true) {
            try {
                msg = read_message();
            } catch (const std::exception& e) {
                std::println(stderr, "PID {}; Exception while reading message: {}", getpid(), e.what());
            }

            if (!msg.has_value()) {
                std::println(stderr, "PID {}; Message failed, restarting pipe.", getpid());
                std::ignore = init();
                continue;
            }
            if (msg->kind == IPC::MessageKind::STOP) {
                std::println("PID {}; Trace collector stopping as per STOP message", getpid());
                break;
            }
            message_handler(msg.value());
        }
        std::println("PID {}; Pipe server exiting", getpid());
        stop_handler();
    }

    [[nodiscard]] std::optional<Message> read_message()
    {
        Message msg {};
        m_pipe_stream >> msg;
        if (m_pipe_stream.fail()) {
            std::println(stderr, "PID {}: Error while reading message. {}", m_pid, strerror(errno));
            return std::nullopt;
        }
        return msg;
    }

    ~PipeServer()
    {
        m_pipe_stream.close();
        if (m_pipe_stream.fail()) {
            std::println(stderr, "PID {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
        if (unlink(m_pipe_name.c_str()) != 0) {
            std::println(stderr, "PID {}: Error while unlinking pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipe_name;
    std::ifstream m_pipe_stream;
};

}
