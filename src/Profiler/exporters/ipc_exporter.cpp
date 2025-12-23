#include "ipc_exporter.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>

namespace Tracer {

IPCExporter& IPCExporter::instance(const char* pipe_path)
{
    static IPCExporter instance { pipe_path };
    return instance;
}

void IPCExporter::push_trace(const ChromeEvent& result)
{
    std::stringstream ss;
    ss << result;

    IPC::Message msg {
        /* kind */ IPC::MessageKind::DATA,
        /* pid  */ result.pid,
        /* body */ ss.str(),
    };

    std::lock_guard<std::mutex> lock(m_lock);
    if (!m_pipe.write_message(msg)) {
        std::cerr << "Failed to send message..\n";
        return;
    }
}

IPCExporter::IPCExporter(const char* pipe_path)
    : m_pipe(pipe_path)
{
    if (!m_pipe.init()) {
        std::exit(EXIT_FAILURE);
    }
}

IPCExporter::~IPCExporter()
{
    IPC::Message msg {
        /* kind */ IPC::MessageKind::STOP,
        /* pid  */ getpid(),
        /* body */ {},
    };
    std::ignore = m_pipe.write_message(msg);
}

} // namespace Tracer
