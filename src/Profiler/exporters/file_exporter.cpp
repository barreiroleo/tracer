#include "file_exporter.hpp"

#include <atomic>

namespace Tracer {

FileExporter::FileExporter(const char* output_file)
    : m_trace_stream(std::ofstream(output_file))
{
    if (!m_trace_stream.is_open()) {
        throw std::runtime_error("Failed to open trace output file.");
    }
    m_trace_stream << TRACE_EVENTS;
}

FileExporter::~FileExporter()
{
    m_trace_stream << '\n'
                   << TRACE_EVENT_BODY;
}

void FileExporter::push_trace(const ChromeEvent& result)
{
    static std::atomic_bool is_first_event { true };

    const auto json = serialize_to_json(result);

    std::lock_guard<std::mutex> lock(m_lock);
    {
        bool is_first = is_first_event.exchange(false);
        std::ignore = !is_first && m_trace_stream << ',';
        m_trace_stream << '\n'
                       << json;
    }
}

} // namespace Tracer
