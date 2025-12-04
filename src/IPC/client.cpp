#include "client.hpp"

#include <filesystem>
#include <print>
#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

namespace IPC {

PipeClient::PipeClient(std::string_view path)
    : m_pid(getpid())
    , m_pipe_path(path)
{
}

PipeClient::~PipeClient()
{
    m_pipe_stream.close();
    if (m_pipe_stream.fail()) {
        std::println(stderr, "PID {}: Error while closing pipe. {}", m_pid, strerror(errno));
    }
}

std::optional<std::reference_wrapper<std::ofstream>> PipeClient::init()
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

bool PipeClient::write_message(const Message& msg)
{
    m_pipe_stream << msg;
    if (m_pipe_stream.fail()) {
        std::println(stderr, "PID {}: Error while writing message. {}", m_pid, strerror(errno));
        return false;
    }
    return true;
}

}
