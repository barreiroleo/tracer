#pragma once

#include <IPC/client.hpp>
#include <IPC/message.hpp>

#include <Profiler/chrome_event.hpp>
#include <Profiler/serialization.hpp>

#include <iostream>
#include <mutex>
#include <sstream>

namespace Tracer {

class IPCExporter {
public:
    static IPCExporter& instance(std::string_view pipe_path = "/tmp/trace.pipe")
    {
        static IPCExporter instance { pipe_path };
        return instance;
    }

    void push_trace(const ChromeEvent& result)
    {
        std::stringstream ss;
        ss << result;

        IPC::Message msg {
            .kind = IPC::MessageKind::DATA,
            .body = ss.str()
        };

        if (!m_pipe.write_message(msg)) {
            std::cerr << "Failed to send message..\n";
            return;
        }
    }

private:
    IPCExporter(std::string_view pipe_path)
        : m_pipe(pipe_path)
    {
        if (!m_pipe.init().has_value()) {
            std::exit(EXIT_FAILURE);
        }
    }

    ~IPCExporter()
    {
        const IPC::Message msg {
            .kind = IPC::MessageKind::STOP,
            .body = {}
        };
        std::ignore = m_pipe.write_message(msg);
    };

private:
    std::mutex m_lock;
    IPC::PipeClient m_pipe;
};

} // namespace Tracer
