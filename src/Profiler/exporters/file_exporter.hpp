#pragma once

#include <Profiler/chrome_event.hpp>

#include <fstream>
#include <mutex>

namespace Tracer {

class FileExporter {
public:
    static FileExporter& instance(const char* output_file = "trace.json");

    void push_trace(const ChromeEvent& result);

private:
    FileExporter(const char* output_file);

    ~FileExporter();

private:
    std::mutex m_lock;
    std::ofstream m_trace_stream;
};

} // namespace Tracer
