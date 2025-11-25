#include <src/cli_opts.hpp>
#include <src/shared_msg.hpp>

#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

void do_write_routine(pid_t pid, int file_descriptor)
{
    static uint8_t counter {};
    IPC::message msg {
        .pid = pid,
        .counter = ++counter,
    };
    std::println("Process {}: Write {}.", pid, counter);

    if (const auto result = write(file_descriptor, &msg, sizeof(msg)); result < 0)
        std::println("Process {}: Error while writing message. {}", pid, strerror(errno));
}

int main(int argc, char* argv[])
{
    const Options options = Args::parse<Options>(argc, argv, command_handler);
    const auto pipename = std::move(options.pipename);

    pid_t pid = getpid();

    // Try to open pipe for writing only
    int file_descriptor = open(pipename.data(), O_WRONLY);
    while (file_descriptor < 0 && errno == ENOENT) {
        std::println("Process {}: Pipe not found. Retrying...", pid);
        file_descriptor = open(pipename.data(), O_WRONLY);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (int i = 0; i < 5; i++) {
        do_write_routine(pid, file_descriptor);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (close(file_descriptor) != 0) {
        std::println("Process {}: Error while closing pipe. {}", pid, strerror(errno));
    }
    if (unlink(pipename.data()) != 0) {
        std::println("Process {}: Error while unlinking pipe. {}", pid, strerror(errno));
    }

    return 0;
}
