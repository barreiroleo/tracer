#include "common_args.hpp"

#include <print>
#include <server.hpp>

#include <thread>
#include <unistd.h>

void run_listener(IPC::PipeServer& server)
{
    std::println("Listener started");

    const auto message_handler = [&](const IPC::Message& msg) {
        std::println("PID {}; Received msg: {}", getpid(), msg.to_json());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    };

    const auto stop_handler = [&]() {
        std::println("Listener stopping as per STOP message");
    };

    server.run(message_handler, stop_handler);
}

int main(int argc, char** argv)
{
    const ArgsOpts options = Args::parse<ArgsOpts>(argc, argv, command_handler);
    std::string_view pipe_path = options.pipe_path;

    IPC::PipeServer server { pipe_path };
    if (!server.init()) {
        std::exit(EXIT_FAILURE);
    }
    run_listener(server);

    return 0;
}
