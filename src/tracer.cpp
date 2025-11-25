#include "cli_opts.hpp"
#include "shared_msg.hpp"

#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

void do_listen_routine(pid_t pid, int file_descriptor)
{
    IPC::message msg {};
    if (read(file_descriptor, &msg, sizeof(msg)) < 0)
        std::println("Process {}: Error while reading message. {}.", pid, strerror(errno));
    std::println("Process {}: Received value {} from the parent process {}.", pid, msg.counter, msg.pid);
}

int main(int argc, char** argv)
{
    const Options options = Args::parse<Options>(argc, argv, command_handler);
    const auto pipename = std::move(options.pipename);

    // create the named pipe (fifo) with permission
    if (const auto result = mkfifo(pipename.data(), 0666); result < 0)
        std::println("Error when creating FIFO. {}.", strerror(errno));

    pid_t pid = getpid();

    // Try to open pipe for read only
    int file_descriptor = open(pipename.data(), O_RDONLY);
    if (file_descriptor < 0 && errno == ENOENT) {
        std::println("Process {}: Something went wrong creating the pipe", pid);
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 5; i++) {
        do_listen_routine(pid, file_descriptor);
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
