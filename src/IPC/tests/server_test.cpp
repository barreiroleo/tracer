#include "common.hpp"

#include <server.hpp>

#include <thread>

void listen_next(pid_t pid, int file_descriptor)
{
    IPC::message msg {};
    if (read(file_descriptor, &msg, sizeof(msg)) < 0)
        std::println("Process {}: Error while reading message. {}.", pid, strerror(errno));
    std::println("Process {}: Received value {} from the parent process {}.", pid, msg.counter, msg.pid);
}

void run_listener(IPC::FileDescriptor file_descriptor)
{
    std::println("Listener started");
    pid_t pid = getpid();
    for (int i = 0; i < 5; i++) {
        listen_next(pid, file_descriptor);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char** argv)
{
    const Options options = Args::parse<Options>(argc, argv, command_handler);
    const auto pipename = std::move(options.pipename);

    IPC::PipeServer server { pipename };
    const auto file_descriptor = server.init();
    if (!file_descriptor.has_value()) {
        std::exit(EXIT_FAILURE);
    }

    run_listener(*file_descriptor);

    return 0;
}
