#include "server.hpp"
#include "_peeker.hpp"

#include <print>
#include <set>
#include <utility>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

namespace IPC {
// TODO(lbarreiro): Reimplement with non-blocking I/O
std::optional<Message> read_message(std::ifstream& pipe_stream)
{
    Message msg {};
    try {
        pipe_stream >> msg;
        if (pipe_stream.fail()) {
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        std::println(stderr, "Exception: {}", e.what());
        return std::nullopt;
    }
    // std::println(">> Message PID {}, kind {}", msg.pid, std::to_underlying(msg.kind));
    return msg;
}

PipeServer::PipeServer(std::string_view path)
    : m_pipe_name(path) { };

PipeServer::~PipeServer()
{
    m_pipe_stream.close();
    if (m_pipe_stream.fail()) {
        std::println(stderr, "Error while closing pipe. {}", strerror(errno));
    }
    if (unlink(m_pipe_name.c_str()) != 0) {
        std::println(stderr, "Error while unlinking pipe. {}", strerror(errno));
    }
}

bool PipeServer::init()
{
    // Create the named pipe (fifo) with permission
    const auto result = mkfifo(m_pipe_name.c_str(), 0666);
    if (result < 0 && errno != EEXIST) {
        std::println(stderr, "Error when creating FIFO. {}.", strerror(errno));
        return false;
    }

    m_pipe_stream = std::ifstream(m_pipe_name.c_str(), std::ios::binary | std::ios::in);
    if (!m_pipe_stream.is_open() || m_pipe_stream.fail()) {
        std::println(stderr, "Failed to open pipe for reading.");
        return false;
    }
    std::println("Server initialized on pipe: {}", m_pipe_name);
    return true;
}

void PipeServer::run(MessageHandler message_handler, StopHandler stop_handler)
{
    bool should_run { true };
    while (should_run) {
        const auto msg = read_message(m_pipe_stream);
        if (!msg.has_value()) {
            std::println(stderr, "Error reading message. Restarting pipe.");
            std::ignore = init();
            continue;
        }

        if (msg->kind == MessageKind::DATA) {
            // std::println(">> Message from PID [{}]", msg->pid);
            message_handler(msg.value());
        }

        {
            switch (client_handler(msg.value())) {
            case HandlerResult::CONTINUE:
                break;
            case HandlerResult::STOP:
                should_run = false;
                continue;
            }
        }
    }
    std::println(stderr, "No active clients remaining, stopping server");
    stop_handler();
}

HandlerResult PipeServer::client_handler(const Message& msg)
{
    const bool is_new_client = m_active_clients.emplace(msg.pid).second;
    if (is_new_client) {
        std::println(">> New client PID [{}]", msg.pid);
    }

    if (msg.kind == MessageKind::STOP) {
        m_active_clients.erase(msg.pid);
        std::println(">> Client [{}] disconnected. Clients {}", msg.pid, m_active_clients.size());
    }

    // Wait for new clients before shutting down
    if (m_active_clients.empty()) {
        return PipePeeker(m_pipe_name, m_timeout_ms).peek() == PollResult::NEW_DATA
            ? HandlerResult::CONTINUE
            : HandlerResult::STOP;
    }
    return HandlerResult::CONTINUE;
}
} // namespace IPC
