#include <src/Profiler/chrome_json.hpp>

#include <print>

int main(int /* argc */, char* /* argv */[])
{
    // const int64_t ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const int64_t ts = std::numeric_limits<int64_t>::max();
    const __pid_t pid = std::numeric_limits<pid_t>::max();

    Profiler::TraceEvent event {
        .name = "Test Event",
        .cat = "default",
        .ph = 'X',
        .ts = ts,
        .pid = pid,
        .tid = "2147483648",
        .dur = 1000,
    };
    std::string event_json = to_string(event);

    static constexpr std::string_view expected_json {
        R"({"name":"Test Event","cat":"default","ph":"X","ts":9223372036854775807,"pid":2147483647,"tid":2147483648,"dur":1000})"
    };

    if (event_json.compare(expected_json) != 0) {
        std::println(stderr, "TraceEvent: {}", event_json);
        std::println(stderr, "Expected: {}", expected_json);
        throw std::logic_error("Validation failed: JSON output does not match expected output");
    }
    return 0;
}
