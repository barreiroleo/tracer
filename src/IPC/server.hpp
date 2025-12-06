#pragma once

#include "message.hpp"

#include <fstream>
#include <functional>
#include <set>

namespace IPC {
using MessageHandler = std::function<void(IPC::Message)>;
using StopHandler = std::function<void()>;

enum class HandlerResult {
    CONTINUE,
    STOP,
};

class PipeServer {
public:
    PipeServer(std::string_view path);
    ~PipeServer();

    [[nodiscard]] bool init();

    /// @brief Run the server loop.
    ///
    /// MessageKind::DATA messages are passed to the provided message_handler.
    /// MessageKind::STOP is handled internally to manage active clients.
    ///
    /// @param[in] message_handler Function to handle incoming messages.
    /// @param[in] stop_handler Function to handle server stop event.
    void run(MessageHandler message_handler, StopHandler stop_handler);

private:
    HandlerResult client_handler(const Message& msg);

    int m_timeout_ms = 1000;
    std::set<int> m_active_clients {};
    std::string m_pipe_name;
    std::ifstream m_pipe_stream;
};

} // namespace IPC
