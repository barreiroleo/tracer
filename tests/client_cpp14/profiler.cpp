#include <Profiler/macros.hpp>

#include <future>
#include <thread>

// Simple function with TRACE_FN
void simple_function()
{
    TRACE_FN();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Function with multiple scopes
void function_with_scopes()
{
    TRACE_FN_CAT("scopes");
    {
        TRACE_SCOPE("initialization_phase");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
        TRACE_SCOPE_CAT("processing_phase", "scopes");
        for (int i = 0; i < 3; ++i) {
            TRACE_SCOPE("loop_iteration");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    {
        TRACE_SCOPE_CAT("finalization_phase", "scopes");
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}

// Async function
std::future<int> async_computation(int value)
{
    return std::async(std::launch::async, [value]() {
        TRACE_SCOPE_CAT("async_task", "async");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return value * 2;
    });
}

int main(int /* argc */, char* /* argv */[])
{
    TRACE_SETUP("trace_cpp14.json");
    std::cout << "Starting comprehensive profiler test...\n";
    TRACE_FN();

    std::cout << "1. Testing simple functions...\n";
    simple_function();

    std::cout << "2. Testing functions with multiple scopes...\n";
    function_with_scopes();

    std::cout << "3. Testing async operations...\n";

    std::cout << "\nProfiler test complete.\n";
    return 0;
}
