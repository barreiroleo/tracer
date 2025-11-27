# Tracer

Lightweight C++23 profiler that generates Chrome Trace Event Format traces for visualization in Perfetto and Firefox Profiler.

## Features

- Zero-overhead when disabled via `#ifdef ENABLE_TRACING`
- Thread-safe with automatic thread ID tracking
- RAII-based scope tracing
- Chrome Trace Event Format (JSON) output
- Compatible with [Perfetto](https://ui.perfetto.dev) and [Firefox Profiler](https://profiler.firefox.com)

## Quick Start

```cpp
#def ENABLE_TRACING
#include <src/Profiler/trace.hpp>

std::future<int> async_computation(int value)
{
    TRACE_FN_CAT("async");
    return std::async(std::launch::async, [value]() {
        TRACE_SCOPE_CAT("async_task", "async");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return value * 2;
    });
}

int main() {
    // Initialize profiler with output file
    TRACE_SETUP("trace.json");

    // Trace entire function
    TRACE_FN();
    {
        // Trace specific scope
        TRACE_SCOPE("initialization");
    }
    {
        // Trace with custom category
        TRACE_SCOPE_CAT("computation");
    }

    return 0;
}
```

## Visualization

### Perfetto

Load at [ui.perfetto.dev](https://ui.perfetto.dev) for advanced analysis:

**Flamegraph view:**
![Perfetto Flamegraph](assets/perfetto-slices-flamegraph.png)

**SQL queries** for filtering by category:
![Perfetto SQL](assets/perfetto-slices-sql.png)

**Synchronized view** with Linux perf data:
![Perfetto Sync](assets/perfetto-slices-tabs-sync.png)


### Firefox Profiler

Load `trace.json` at [profiler.firefox.com](https://profiler.firefox.com):

![Firefox Profiler](assets/firefox-slices.png)


## Output Format

Generates Chrome Trace Event Format with complete events (`ph: "X"`):

```json
{
  "traceEvents": [
    {"name": "main", "cat": "Default", "ph": "X", "ts": 1234567890, "pid": 1234, "tid": 1234, "dur": 5000},
    {"name": "process_data", "cat": "computation", "ph": "X", "ts": 1234568000, "pid": 1234, "tid": 1234, "dur": 2000}
  ]
}
```
