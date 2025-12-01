#pragma once

#include <algorithm>
#include <string>

#ifdef __cpp_lib_format
#include <format>
#else
#include <sstream>
#endif

namespace Tracer {

// Chrome Trace Event format
// {
//   "traceEvents": [
//     {"name": "Asub", "cat": "PERF", "ph": "B", "pid": 22630, "tid": 22630, "ts": 829},
//     {"name": "Asub", "cat": "PERF", "ph": "E", "pid": 22630, "tid": 22630, "ts": 833}
//   ],
//   "displayTimeUnit": "ns",
//   "systemTraceEvents": "SystemTraceData",
//   "otherData": {
//     "version": "My Application v1.0"
//   },
//   "stackFrames": {...}
//   "samples": [...],
// }

static constexpr std::string_view TRACE_EVENTS {
    R"({"traceEvents":[)"
};

static constexpr std::string_view TRACE_EVENT_BODY {
    R"(],"displayTimeUnit":"ns"})"
};

/// @brief Catapult - Trace Event Format
/// @ref https://chromium.googlesource.com/catapult/+/HEAD/docs/trace-event-format.md
///
/// @docs Cite from catapult docs:
/// # Complete Events:
///   Each complete event logically combines a pair of duration (B and E) events. The complete
///   events are designated by the X phase type. In a trace that most of the events are duration
///   events, using complete events to replace the duration events can reduce the size of the trace
///   to about half.
///
///   There is an extra parameter dur to specify the tracing clock duration of complete events in
///   microseconds. All other parameters are the same as in duration events. The ts parameter
///   indicate the time of the start of the complete event. Unlike duration events, the timestamps
///   of complete events can be in any order.
///
///   An optional parameter tdur specifies the thread clock duration of complete events in
///   microseconds.
struct ChromeEvent {

    /// @var name The name of the event, as displayed in Trace Viewer
    const std::string name;

    /// @var cat The event categories. This is a comma separated list of categories for the event.
    /// The categories can be used to hide events in the Trace Viewer UI.
    const std::string cat;

    /// @var ph The event type. This is a single character which changes depending on the type of
    /// event being output.
    const char ph;

    /// @var ts The tracing clock timestamp of the event. The timestamps are provided at us.
    const int64_t ts;

    /// @var pid The process ID for the process that output this event.
    const int pid;

    /// @var tid The thread ID for the thread that output this event.
    const size_t tid;

    /// @var dur to specify the tracing clock duration of complete events in microseconds
    const int64_t dur;

    /// @var tts? The thread clock timestamp of the event. The timestamps are provided at
    /// microsecond granularity.
    // const int64_t tts {};
    //
    /// @var args Any arguments provided for the event. Some of the event types have required
    /// argument fields, otherwise, you can put any information you wish in here. The arguments are
    /// displayed in Trace Viewer when you view an event in the analysis section.
    // const char* args;
    //
    /// @var cname? A fixed color name to associate with the event. If provided, cname must be one
    /// of the names listed in trace-viewer's base color scheme's reserved color names list.
    // const char* cname { };
};

inline std::string to_string(const ChromeEvent& event)
{
    std::string_view event_name_sanitized = event.name;
    if (event.name.contains('"')) {
        std::string name_buf {};
        std::replace_copy(event.name.begin(), event.name.end(), std::back_inserter(name_buf), '"', '\'');
        event_name_sanitized = name_buf;
    }
#ifdef __cpp_lib_format
    static constexpr std::string_view json_template {
        R"({{"name":"{}","cat":"{}","ph":"{}","ts":{},"pid":{},"tid":{},"dur":{}}})"
    };
    return std::format(json_template, event_name_sanitized, event.cat, event.ph, event.ts, event.pid, event.tid, event.dur);
#else
    std::stringstream ss;
    ss << R"({"name":")" << event_name_sanitized << "\","
       << R"("cat":")" << event.cat << "\","
       << R"("ph":")" << event.ph << "\","
       << R"("ts":)" << event.ts << ","
       << R"("pid":)" << event.pid << ","
       << R"("tid":)" << event.tid << ","
       << R"("dur":)" << event.dur << "}";
    return ss.str();
#endif
}

} // namespace Tracer
