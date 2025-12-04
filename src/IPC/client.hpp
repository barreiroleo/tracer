#pragma once

#include "message.hpp"

#include <fstream>
#include <optional>
#include <string>

namespace IPC {

class PipeClient {
public:
    PipeClient(std::string_view path);
    ~PipeClient();

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ofstream>> init();

    [[nodiscard]]
    bool write_message(const Message& msg);

private:
    pid_t m_pid {};
    std::string m_pipe_path;
    std::ofstream m_pipe_stream;
};

} // namespace IPC
