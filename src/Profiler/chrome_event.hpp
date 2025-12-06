#pragma once

#include <string>

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

static constexpr const char* TRACE_EVENTS {
    R"({"traceEvents":[)"
};

static constexpr const char* TRACE_EVENT_BODY {
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
    std::string name;

    /// @var cat The event categories. This is a comma separated list of categories for the event.
    /// The categories can be used to hide events in the Trace Viewer UI.
    std::string cat;

    /// @var ph The event type. This is a single character which changes depending on the type of
    /// event being output.
    char ph;

    /// @var ts The tracing clock timestamp of the event. The timestamps are provided at us.
    int64_t ts;

    /// @var pid The process ID for the process that output this event.
    int pid;

    /// @var tid The thread ID for the thread that output this event.
    size_t tid;

    /// @var dur to specify the tracing clock duration of complete events in microseconds
    int64_t dur;

    /// @var tts? The thread clock timestamp of the event. The timestamps are provided at
    /// microsecond granularity.
    // int64_t tts {};
    //
    /// @var args Any arguments provided for the event. Some of the event types have required
    /// argument fields, otherwise, you can put any information you wish in here. The arguments are
    /// displayed in Trace Viewer when you view an event in the analysis section.
    // char* args;
    //
    /// @var cname? A fixed color name to associate with the event. If provided, cname must be one
    /// of the names listed in trace-viewer's base color scheme's reserved color names list.
    // char* cname { };
};

/// @brief Serialize ChromeEvent to JSON format
/// @param event The event to serialize
/// @return JSON string representation
std::string serialize_to_json(const ChromeEvent& event);

/// @brief Serialize ChromeEvent to stream format
std::ostream& serialize_to_stream(std::ostream& out, const ChromeEvent& event);

/// @brief Deserialize ChromeEvent from stream format
std::istream& deserialize_from_stream(std::istream& in, ChromeEvent& event);

inline std::ostream& operator<<(std::ostream& out, const ChromeEvent& event)
{
    return serialize_to_stream(out, event);
}

inline std::istream& operator>>(std::istream& in, ChromeEvent& event)
{
    return deserialize_from_stream(in, event);
}

} // namespace Tracer
