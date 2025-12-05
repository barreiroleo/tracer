#include <Profiler/macros.hpp>

#include <chrono>
#include <future>
#include <print>
#include <thread>
#include <vector>

// Simple function with TRACE_FN
void simple_function()
{
    TRACE_FN();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Function with custom category
void categorized_function()
{
    TRACE_FN_CAT("computation");
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
}

// Nested function calls
void inner_function()
{
    TRACE_FN_CAT("nested");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void middle_function()
{
    TRACE_FN_CAT("nested");
    {
        TRACE_SCOPE_CAT("middle_processing", "nested");
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        inner_function();
    }
}

void outer_function()
{
    TRACE_FN_CAT("nested");
    {
        TRACE_SCOPE("outer_setup");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    middle_function();
    {
        TRACE_SCOPE("outer_cleanup");
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

// Recursive function
void recursive_fibonacci(int n, int depth = 0)
{
    TRACE_FN_CAT("recursion");
    if (n <= 1 || depth > 5) {
        return;
    }
    recursive_fibonacci(n - 1, depth + 1);
    recursive_fibonacci(n - 2, depth + 1);
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

void test_async_operations()
{
    TRACE_FN_CAT("async");

    std::vector<std::future<int>> futures;

    {
        TRACE_SCOPE_CAT("launching_tasks", "async");
        for (int i = 0; i < 4; ++i) {
            futures.push_back(async_computation(i));
        }
    }

    {
        TRACE_SCOPE_CAT("waiting_for_results", "async");
        for (auto& fut : futures) {
            fut.get();
        }
    }
}

// Thread function
void thread_worker(int /* thread_id */)
{
    TRACE_FN_CAT("threads");

    {
        TRACE_SCOPE_CAT("thread_initialization", "threads");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    for (int i = 0; i < 3; ++i) {
        TRACE_SCOPE("thread_work_iteration");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (i % 2 == 0) {
            simple_function();
        }
    }

    {
        TRACE_SCOPE_CAT("thread_cleanup", "threads");
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

void test_multithreading()
{
    TRACE_FN_CAT("threads");

    std::vector<std::thread> threads;

    {
        TRACE_SCOPE_CAT("spawning_threads", "threads");
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back(thread_worker, i);
        }
    }

    {
        TRACE_SCOPE_CAT("joining_threads", "threads");
        for (auto& t : threads) {
            t.join();
        }
    }
}

// Combined scenario: threads calling recursive functions
void thread_with_recursion(int /* id */)
{
    TRACE_FN_CAT("combined");
    outer_function();
    recursive_fibonacci(6);
}

void test_combined_scenario()
{
    TRACE_FN_CAT("combined");

    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(thread_with_recursion, i);
    }

    for (auto& t : threads) {
        t.join();
    }
}

int main(int /* argc */, char* /* argv */[])
{
    TRACE_SETUP(std::format("trace-{:%Y-%m-%d_%H-%M}.json", std::chrono::system_clock::now()).c_str());

    std::println("Starting comprehensive profiler test...\n");
    TRACE_FN();

    std::println("1. Testing simple functions...");
    simple_function();
    categorized_function();

    std::println("2. Testing nested function calls...");
    outer_function();

    std::println("3. Testing recursive calls...");
    recursive_fibonacci(8);

    std::println("4. Testing functions with multiple scopes...");
    function_with_scopes();

    std::println("5. Testing async operations...");
    test_async_operations();

    std::println("6. Testing multithreading...");
    test_multithreading();

    std::println("7. Testing combined scenario (threads + recursion + nesting)...");
    test_combined_scenario();

    std::println("\nProfiler test complete.");
    return 0;
}
