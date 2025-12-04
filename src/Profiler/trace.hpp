#pragma once

#include <Profiler/exporters/file_exporter.hpp>
#include <Profiler/exporters/ipc_exporter.hpp>

namespace Tracer {

template <class T = FileExporter>
class TraceScope {
public:
    TraceScope(std::string_view name, std::string_view cat = "Default");
    ~TraceScope();

private:
    void write_trace();

    std::string m_name;
    std::string m_cat;
    const int64_t m_start_time;
};

} // namespace Tracer

