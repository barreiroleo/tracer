#pragma once

#include "chrome_event.hpp"

#include <atomic>
#include <fstream>
#include <mutex>

#ifdef ENABLE_TRACING
#define TRACE_SETUP(file) Tracer::FileExporter::instance(file)
#else
#define TRACE_SETUP(file)
#endif // ENABLE_TRACING

namespace Tracer {

class FileExporter {
public:
    static FileExporter& instance(std::string_view output_file = "trace.json")
    {
        static FileExporter instance { output_file };
        return instance;
    }

    void push_trace(ChromeEvent&& result)
    {
        static std::atomic_bool is_first_event { true };

        const auto json = Tracer::to_string(result);

        std::lock_guard<std::mutex> lock(m_lock);
        {
            bool is_first = is_first_event.exchange(false);
            std::ignore = !is_first && m_trace_stream << ',';
            m_trace_stream << '\n'
                           << json;
        }
    }

private:
    FileExporter(std::string_view output_file)
        : m_trace_stream(std::ofstream(output_file.data()))
    {
        if (!m_trace_stream.is_open()) {
            throw std::runtime_error("Failed to open trace output file.");
        }
        m_trace_stream << TRACE_EVENTS;
    }

    ~FileExporter()
    {
        m_trace_stream << '\n'
                       << TRACE_EVENT_BODY;
    }

private:
    std::mutex m_lock;
    std::ofstream m_trace_stream;
};

} // namespace Tracer
