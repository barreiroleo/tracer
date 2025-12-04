#include <Profiler/chrome_event.hpp>

#include <iostream>
#include <limits>

int main(int /* argc */, char* /* argv */[])
{
    // const int64_t ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const int64_t ts = std::numeric_limits<int64_t>::max();
    const __pid_t pid = std::numeric_limits<pid_t>::max();
    const int tid = std::numeric_limits<int>::max();

    Tracer::ChromeEvent event {};
    event.name = "Test Event";
    event.cat = "default";
    event.ph = 'X';
    event.ts = ts;
    event.pid = pid;
    event.tid = tid;
    event.dur = 1000;

    std::string event_json = Tracer::serialize_to_json(event);

    static constexpr std::string_view expected_json {
        R"({"name":"Test Event","cat":"default","ph":"X","ts":9223372036854775807,"pid":2147483647,"tid":2147483647,"dur":1000})"
    };

    if (event_json.compare(expected_json) != 0) {
        std::cerr << "TraceEvent: " << event_json << '\n';
        std::cerr << "Expected: " << expected_json << '\n';
        throw std::logic_error("Validation failed: JSON output does not match expected output");
    }
    return 0;
}
