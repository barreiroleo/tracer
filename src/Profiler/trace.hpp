#pragma once

#include <Profiler/chrome_event.hpp>
#include <Profiler/exporters/file_exporter.hpp>
#include <Profiler/exporters/ipc_exporter.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <unistd.h>

namespace Tracer {

template <class ExporterType = FileExporter>
class TraceScope {
public:
    using time_unit = std::chrono::microseconds;
    using high_resolution_clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<high_resolution_clock, time_unit>;

    TraceScope(std::string_view name, std::string_view cat = "Default")
        : m_name(name)
        , m_cat(cat)
        , m_start_time(get_unique_timestamp())
    {
    }

    ~TraceScope()
    {
        try {
            write_trace();
        } catch (...) {
            std::cerr << "Warning: Exception occurred while writing trace event.";
        }
    }

private:
    /// @brief Get a unique timestamp that's guaranteed to be monotonically increasing
    ///
    /// This solves the limitation on events with complete events (X) has the same timestamps
    /// - perf_text_importer_sample_no_frames
    static inline int64_t get_unique_timestamp()
    {
        static thread_local int64_t last_timestamp { 0 };

        auto current = time_point_cast<time_unit>(high_resolution_clock::now())
                           .time_since_epoch()
                           .count();

        // Ensure monotonic increase by incrementing if timestamp hasn't advanced
        std::ignore = (current <= last_timestamp) && (current = last_timestamp + 1);

        last_timestamp = current;
        return current;
    }

    void write_trace()
    {
        // ToDo: Make it platform independent.
        // Perfetto needs 4 digits id, but hash for thread::id is 19 digits long.
        // static thread_local auto tid = std::hash<std::thread::id> {}(std::this_thread::get_id());
        static thread_local int tid = static_cast<int>(syscall(SYS_gettid));
        const auto end_time = get_unique_timestamp();

        ChromeEvent m_trace_data {
            .name = std::move(m_name),
            .cat = std::move(m_cat),
            .ph = 'X',
            .ts = m_start_time,
            .pid = getpid(),
            .tid = static_cast<size_t>(tid),
            .dur = (end_time - m_start_time),
        };

        ExporterType::instance().push_trace(m_trace_data);
    }

    std::string m_name;
    std::string m_cat;
    const int64_t m_start_time;
};

// Type aliases for common use cases
using Trace = TraceScope<FileExporter>;

// Type alias for IPC-based tracing
using IPCTrace = TraceScope<IPCExporter>;

} // namespace Tracer

// Macros for file-based tracing (default)
#ifdef ENABLE_TRACING
#define TRACE_SCOPE_CAT(name, cat) Tracer::Trace trace_##__LINE__(name, cat)
#define TRACE_SCOPE(name) Tracer::Trace trace_##__LINE__(name)
#define TRACE_FN_CAT(cat) TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define TRACE_FN() TRACE_SCOPE(__FUNCTION__)
#else
#define TRACE_SCOPE_CAT(name, cat)
#define TRACE_SCOPE(name)
#define TRACE_FN_CAT(cat)
#define TRACE_FN()
#endif // ENABLE_TRACING

// Macros for IPC-based tracing
#ifdef ENABLE_TRACING
#define IPC_TRACE_SETUP(pipe) Tracer::IPCExporter::instance(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat) Tracer::IPCTrace trace_##__LINE__(name, cat)
#define IPC_TRACE_SCOPE(name) Tracer::IPCTrace trace_##__LINE__(name)
#define IPC_TRACE_FN_CAT(cat) IPC_TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define IPC_TRACE_FN() IPC_TRACE_SCOPE(__FUNCTION__)
#else
#define IPC_TRACE_SETUP(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat)
#define IPC_TRACE_SCOPE(name)
#define IPC_TRACE_FN_CAT(cat)
#define IPC_TRACE_FN()
#endif // ENABLE_TRACING
