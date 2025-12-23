#pragma once

#include <IPC/client.hpp>
#include <Profiler/chrome_event.hpp>

#include <mutex>

namespace Tracer {

class IPCExporter {
public:
    static IPCExporter& instance(const char* pipe_path = "/tmp/trace.pipe");

    void push_trace(const ChromeEvent& result);

private:
    IPCExporter(const char* pipe_path);

    ~IPCExporter();

private:
    std::mutex m_lock;
    IPC::PipeClient m_pipe;
};

} // namespace Tracer
