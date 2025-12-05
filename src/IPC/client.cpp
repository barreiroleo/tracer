#include "client.hpp"

#include <filesystem>
#include <iostream>
#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

namespace IPC {

PipeClient::PipeClient(const char* path)
    : m_pid(getpid())
    , m_pipe_path(path)
{
}

PipeClient::~PipeClient()
{
    m_pipe_stream.close();
    if (m_pipe_stream.fail()) {
        std::cerr << "PID " << m_pid << ": Error while closing pipe. " << strerror(errno) << '\n';
    }
}

bool PipeClient::init()
{
    namespace fs = std::filesystem;
    if (!fs::exists(m_pipe_path)) {
        std::cerr << "Waiting for pipe to be created at " << m_pipe_path << "...\n";
        while (!fs::exists(m_pipe_path)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    m_pipe_stream.open(m_pipe_path.data(), std::ios::binary | std::ios::out);
    if (!m_pipe_stream.is_open() || m_pipe_stream.fail()) {
        std::cerr << "Failed to open pipe.\n";
        return false;
    }
    return true;
}

bool PipeClient::write_message(const Message& msg)
{
    m_pipe_stream << msg;
    if (m_pipe_stream.fail()) {
        std::cerr << "PID " << m_pid << ": Error while writing message. " << strerror(errno) << '\n';
        return false;
    }
    return true;
}

}
