#include "server.hpp"

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

void handle_new_clients(const Message& msg, std::set<int>& active_clients)
{
    const bool is_new_client = active_clients.emplace(msg.pid).second;
    if (is_new_client) {
        std::println(">> New client PID [{}]", msg.pid);
    }
}

void handle_stopped_clients(const Message& msg, std::set<int>& active_clients)
{
    if (msg.kind == MessageKind::STOP) {
        active_clients.erase(msg.pid);
        std::println(">> Client [{}] disconnected. Clients {}", msg.pid, active_clients.size());
    }
}

PipeServer::PipeServer(std::string_view path)
    : m_pipe_name(path) { };

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
    std::set<int> active_clients {};

    while (true) {
        const auto msg = read_message(m_pipe_stream);
        if (!msg.has_value()) {
            std::println(stderr, "Error reading message. Restarting pipe.");
            std::ignore = init();
            continue;
        }

        handle_new_clients(msg.value(), active_clients);
        handle_stopped_clients(msg.value(), active_clients);

        if (active_clients.empty()) {
            break;
        }
        message_handler(msg.value());
    }
    std::println(stderr, "No active clients remaining, stopping server");
    stop_handler();
}

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

} // namespace IPC
