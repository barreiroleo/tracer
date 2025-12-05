#pragma once

#include "message.hpp"

#include <fstream>
#include <string>

namespace IPC {

class PipeClient {
public:
    PipeClient(const char* path);
    ~PipeClient();

    bool init();

    bool write_message(const Message& msg);

private:
    pid_t m_pid {};
    std::string m_pipe_path;
    std::ofstream m_pipe_stream;
};

} // namespace IPC
