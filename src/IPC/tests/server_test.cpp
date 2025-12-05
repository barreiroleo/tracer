#include "common_args.hpp"

#include <server.hpp>
#include <print>

#include <thread>
#include <unistd.h>

void run_listener(IPC::PipeServer& server)
{
    std::println("Listener started");
    bool running = true;

    while (running) {
        const std::optional<IPC::Message> msg = server.read_message();
        if (!msg.has_value()) {
            std::println("PID {}; Message reception failed", getpid());
        }
        std::println("PID {}; Received msg: {}", getpid(), msg.value().to_json());

        if (msg->kind == IPC::MessageKind::STOP) {
            running = false;
            std::println("Listener stopping as per STOP message");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char** argv)
{
    const ArgsOpts options = Args::parse<ArgsOpts>(argc, argv, command_handler);
    std::string_view pipe_path = options.pipe_path;

    IPC::PipeServer server { pipe_path };
    if (!server.init().has_value()) {
        std::exit(EXIT_FAILURE);
    }

    run_listener(server);

    return 0;
}
