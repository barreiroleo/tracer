#include "common_args.hpp"

#include "message.hpp"
#include <server.hpp>

#include <thread>

void run_listener(IPC::PipeServer& server)
{
    std::println("Listener started");
    bool running = true;

    while (running) {
        const std::optional<IPC::Message> msg = server.read_message<IPC::Message>();
        if (!msg.has_value()) {
            std::println("PID {}; Message reception failed", getpid());
        }
        std::println("PID {}; Received msg: {}", getpid(), IPC::to_string(msg.value()));

        if (msg->kind == IPC::MessageKind::STOP) {
            running = false;
            std::println("Listener stopping as per STOP message");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char** argv)
{
    const Options options = Args::parse<Options>(argc, argv, command_handler);
    const auto pipename = std::move(options.pipename);

    IPC::PipeServer server { pipename };
    if (!server.init().has_value()) {
        std::exit(EXIT_FAILURE);
    }

    run_listener(server);

    return 0;
}
