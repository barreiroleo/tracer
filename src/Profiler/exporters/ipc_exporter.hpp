#pragma once

#include <IPC/client.hpp>
#include <IPC/message.hpp>

#include <Profiler/chrome_event.hpp>
#include <Profiler/exporters/exporter.hpp>
#include <Profiler/serialization.hpp>

#include <iostream>
#include <mutex>
#include <sstream>

namespace Tracer {

class IPCExporter : public Exporter {
public:
    static IPCExporter& instance(std::string_view pipename = "/tmp/trace.pipe")
    {
        static IPCExporter instance { pipename };
        return instance;
    }

    void push_trace(ChromeEvent&& result) override
    {
        static std::stringstream ss;
        ss << result;

        IPC::Message msg {
            .kind = IPC::MessageKind::DATA,
            .length = ss.view().length(),
            .body = {}
        };
        std::copy_n(ss.view().data(), ss.view().length(), msg.body);

        if (!m_pipe.write_message(msg)) {
            std::cerr << "Failed to send message..\n";
            return;
        }
        ss.clear();
    }

private:
    IPCExporter(std::string_view pipename)
        : m_pipe(pipename.data())
    {
        if (!m_pipe.init().has_value()) {
            std::exit(EXIT_FAILURE);
        }
    }

    ~IPCExporter()
    {
        const IPC::Message msg {
            .kind = IPC::MessageKind::STOP,
            .length = 0,
            .body = {}
        };
        std::ignore = m_pipe.write_message(msg);
    };

private:
    std::mutex m_lock;
    IPC::PipeClient m_pipe;
};

} // namespace Tracer
