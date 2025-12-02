#pragma once

#include <Profiler/chrome_event.hpp>

namespace Tracer {

/// @brief Abstract interface for trace event exporters
/// All exporters must implement this interface to be compatible with the tracing system
class Exporter {
public:
    virtual ~Exporter() = default;

    /// @brief Export a trace event
    /// @param event The event to export
    virtual void push_trace(ChromeEvent&& event) = 0;
};

} // namespace Tracer
