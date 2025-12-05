#pragma once

#include <IPC/client.hpp>
#include <IPC/message.hpp>

#include <Profiler/chrome_event.hpp>

#include <iostream>
#include <mutex>
#include <sstream>

namespace Tracer {

class IPCExporter {
public:
    static IPCExporter& instance(const char* pipe_path = "/tmp/trace.pipe")
    {
        static IPCExporter instance { pipe_path };
        return instance;
    }

    void push_trace(const ChromeEvent& result)
    {
        std::stringstream ss;
        ss << result;

        IPC::Message msg {};
        msg.kind = IPC::MessageKind::DATA;
        msg.body = ss.str();

        std::lock_guard<std::mutex> lock(m_lock);
        if (!m_pipe.write_message(msg)) {
            std::cerr << "Failed to send message..\n";
            return;
        }
    }

private:
    IPCExporter(const char* pipe_path)
        : m_pipe(pipe_path)
    {
        if (!m_pipe.init()) {
            std::exit(EXIT_FAILURE);
        }
    }

    ~IPCExporter()
    {
        IPC::Message msg;
        msg.kind = IPC::MessageKind::STOP;
        msg.body = {};
        std::ignore = m_pipe.write_message(msg);
    }

private:
    std::mutex m_lock;
    IPC::PipeClient m_pipe;
};

} // namespace Tracer
