#include "shared_msg.hpp"

#include <src/IPC/client.hpp>
#include <src/cli_opts.hpp>

#include <thread>

void write_next(const IPC::message& msg, int file_descriptor)
{
    std::println("Process {}: Write {}.", msg.pid, msg.counter);
    if (const auto result = write(file_descriptor, &msg, sizeof(msg)); result < 0)
        std::println("Process {}: Error while writing message. {}", msg.pid, strerror(errno));
}

void run_writer(IPC::FileDescriptor file_descriptor)
{
    pid_t pid = getpid();
    for (int i = 0; i < 5; i++) {
        IPC::message msg {
            .pid = pid,
            .counter = i,
        };
        write_next(msg, file_descriptor);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char* argv[])
{
    const Options options = Args::parse<Options>(argc, argv, command_handler);
    const auto pipename = std::move(options.pipename);

    IPC::PipeClient client { pipename };
    const auto file_descriptor = client.init();
    if (!file_descriptor.has_value()) {
        std::exit(EXIT_FAILURE);
    }

    run_writer(*file_descriptor);

    return 0;
}
