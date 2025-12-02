/**
 * Tracer - Single Header
 *
 * A lightweight, header-only C++ profiling library that outputs Chrome Trace Event Format.
 *
 * Usage:
 *   1. Define ENABLE_TRACING before including this header to enable tracing
 *   2. Call TRACE_SETUP("output.json") once at the start of your program
 *   3. Use TRACE_SCOPE(name) or TRACE_FN() to instrument your code
 *
 * Example:
 *   #define ENABLE_TRACING
 *   #include "tracer.hpp"
 *
 *   int main() {
 *       TRACE_SETUP("trace.json");
 *       TRACE_FN();
 *       // your code here
 *       return 0;
 *   }
 *
 * License: See LICENSE file
 * Full implementation with pipe support available at: ./src/Profiler/
 */

#pragma once
#define ENABLE_TRACING

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <unistd.h>

#ifdef __cpp_lib_format
#include <format>
#else
#include <sstream>
#endif

namespace Tracer {

// ============================================================================
// Chrome Event Format Constants and Structures
// ============================================================================

/// @brief Catapult - Trace Event Format
/// @ref https://chromium.googlesource.com/catapult/+/HEAD/docs/trace-event-format.md
struct ChromeEvent {
    std::string name;
    std::string cat;
    char ph;
    int64_t ts;
    int pid;
    size_t tid;
    int64_t dur;
};

// ============================================================================
// Serialization Functions
// ============================================================================

inline std::string serialize_to_json(const ChromeEvent& event)
{
    std::string_view event_name = event.name;
    if (event.name.contains('"')) {
        std::string name_buf {};
        std::replace_copy(event.name.begin(), event.name.end(), std::back_inserter(name_buf), '"', '\'');
        event_name = name_buf;
    }
#ifdef __cpp_lib_format
    return std::format(R"({{"name":"{}","cat":"{}","ph":"{}","ts":{},"pid":{},"tid":{},"dur":{}}})",
        event_name, event.cat, event.ph, event.ts, event.pid, event.tid, event.dur);
#else
    std::stringstream ss;
    ss << R"({"name":")" << event_name << R"(","cat":")" << event.cat << R"(","ph":")" << event.ph
       << R"(","ts":)" << event.ts << R"(,"pid":)" << event.pid << R"(,"tid":)" << event.tid
       << R"(,"dur":)" << event.dur << "}";
    return ss.str();
#endif
}

// ============================================================================
// File Exporter Implementation
// ============================================================================

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
        const auto json = serialize_to_json(result);
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
        m_trace_stream << R"({"traceEvents":[)";
    }

    ~FileExporter()
    {
        m_trace_stream << '\n'
                       << R"(],"displayTimeUnit":"ns"})";
    }

private:
    std::mutex m_lock;
    std::ofstream m_trace_stream;
};

// ============================================================================
// TraceScope - Main Tracing Class
// ============================================================================

template <typename ExporterType = FileExporter>
class TraceScope {
public:
    using time_unit = std::chrono::microseconds;
    using high_resolution_clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<high_resolution_clock, time_unit>;

    TraceScope(std::string&& name, std::string&& cat = "Default")
        : m_name(std::move(name))
        , m_cat(std::move(cat))
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
    static inline int64_t get_unique_timestamp()
    {
        static thread_local int64_t last_timestamp { 0 };
        auto current = time_point_cast<time_unit>(high_resolution_clock::now()).time_since_epoch().count();
        std::ignore = (current <= last_timestamp) && (current = last_timestamp + 1);
        last_timestamp = current;
        return current;
    }

    void write_trace()
    {
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
        ExporterType::instance().push_trace(std::move(m_trace_data));
    }

    std::string m_name;
    std::string m_cat;
    const int64_t m_start_time;
};

// Type aliases for common use cases
using Trace = TraceScope<FileExporter>;

} // namespace Tracer

// ============================================================================
// Macros for Easy Usage
// ============================================================================

// Macros for file-based tracing (default)
#ifdef ENABLE_TRACING
#define TRACE_SETUP(file) Tracer::FileExporter::instance(file)
#define TRACE_SCOPE_CAT(name, cat) Tracer::Trace trace_##__LINE__(name, cat)
#define TRACE_SCOPE(name) Tracer::Trace trace_##__LINE__(name)
#define TRACE_FN_CAT(cat) TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define TRACE_FN() TRACE_SCOPE(__FUNCTION__)
#else
#define TRACE_SETUP(file)
#define TRACE_SCOPE_CAT(name, cat)
#define TRACE_SCOPE(name)
#define TRACE_FN_CAT(cat)
#define TRACE_FN()
#endif // ENABLE_TRACING
