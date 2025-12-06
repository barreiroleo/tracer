#include <Profiler/macros.hpp>

#include <chrono>
#include <future>
#include <print>
#include <thread>
#include <unistd.h>
#include <vector>

// Simple function with IPC_TRACE_FN
void simple_function()
{
    IPC_TRACE_FN();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Function with custom category
void categorized_function()
{
    IPC_TRACE_FN_CAT("computation");
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
}

// Nested function calls
void inner_function()
{
    IPC_TRACE_FN_CAT("nested");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void middle_function()
{
    IPC_TRACE_FN_CAT("nested");
    {
        IPC_TRACE_SCOPE_CAT("middle_processing", "nested");
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        inner_function();
    }
}

void outer_function()
{
    IPC_TRACE_FN_CAT("nested");
    {
        IPC_TRACE_SCOPE("outer_setup");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    middle_function();
    {
        IPC_TRACE_SCOPE("outer_cleanup");
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

// Recursive function
void recursive_fibonacci(int n, int depth = 0)
{
    IPC_TRACE_FN_CAT("recursion");
    if (n <= 1 || depth > 5) {
        return;
    }
    recursive_fibonacci(n - 1, depth + 1);
    recursive_fibonacci(n - 2, depth + 1);
}

// Function with multiple scopes
void function_with_scopes()
{
    IPC_TRACE_FN_CAT("scopes");

    {
        IPC_TRACE_SCOPE("initialization_phase");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        IPC_TRACE_SCOPE_CAT("processing_phase", "scopes");
        for (int i = 0; i < 3; ++i) {
            IPC_TRACE_SCOPE("loop_iteration");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    {
        IPC_TRACE_SCOPE_CAT("finalization_phase", "scopes");
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}

// Async function
std::future<int> async_computation(int value)
{
    return std::async(std::launch::async, [value]() {
        IPC_TRACE_SCOPE_CAT("async_task", "async");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return value * 2;
    });
}

void test_async_operations()
{
    IPC_TRACE_FN_CAT("async");

    std::vector<std::future<int>> futures;

    {
        IPC_TRACE_SCOPE_CAT("launching_tasks", "async");
        for (int i = 0; i < 4; ++i) {
            futures.push_back(async_computation(i));
        }
    }

    {
        IPC_TRACE_SCOPE_CAT("waiting_for_results", "async");
        for (auto& fut : futures) {
            fut.get();
        }
    }
}

// Thread function
void thread_worker(int /* thread_id */)
{
    IPC_TRACE_FN_CAT("threads");

    {
        IPC_TRACE_SCOPE_CAT("thread_initialization", "threads");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    for (int i = 0; i < 3; ++i) {
        IPC_TRACE_SCOPE("thread_work_iteration");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (i % 2 == 0) {
            simple_function();
        }
    }

    {
        IPC_TRACE_SCOPE_CAT("thread_cleanup", "threads");
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

void test_multithreading()
{
    IPC_TRACE_FN_CAT("threads");

    std::vector<std::thread> threads;

    {
        IPC_TRACE_SCOPE_CAT("spawning_threads", "threads");
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back(thread_worker, i);
        }
    }

    {
        IPC_TRACE_SCOPE_CAT("joining_threads", "threads");
        for (auto& t : threads) {
            t.join();
        }
    }
}

// Combined scenario: threads calling recursive functions
void thread_with_recursion(int /* id */)
{
    IPC_TRACE_FN_CAT("combined");
    outer_function();
    recursive_fibonacci(6);
}

void test_combined_scenario()
{
    IPC_TRACE_FN_CAT("combined");

    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(thread_with_recursion, i);
    }

    for (auto& t : threads) {
        t.join();
    }
}

void test_fork()
{
    IPC_TRACE_FN_CAT("Fork");
    pid_t pid = fork();
    if (pid < 0) {
        return std::println(stderr, "Fork failed");
    }
    const auto child_process = []() {
        std::println("Child process running: {}", getpid());
    };
    const auto parent_process = [pid]() {
        std::println("Parent process ({}), child pid: {}", getpid(), pid);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    };
    (pid == 0) ? child_process() : parent_process();
}

int main(int argc, char* argv[])
{
    const char* pipe_path = (argc < 2) ? "/tmp/tracer-trace_pipe_test.pipe" : argv[2];

    std::println("Starting profiler test...\n");
    IPC_TRACE_SETUP(pipe_path);

    IPC_TRACE_FN();
    test_fork();

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
