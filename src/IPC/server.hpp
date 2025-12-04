#pragma once

#include "message.hpp"

#include <fstream>
#include <functional>

namespace IPC {
using FileDescriptor = int;
using MessageHandler = std::function<void(IPC::Message)>;
using StopHandler = std::function<void()>;

class PipeServer {
public:
    PipeServer(std::string_view path);
    ~PipeServer();

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ifstream>> init();

    void run(MessageHandler message_handler, StopHandler stop_handler);

    [[nodiscard]]
    std::optional<Message> read_message();

private:
    pid_t m_pid {};
    std::string m_pipe_name;
    std::ifstream m_pipe_stream;
};

}
