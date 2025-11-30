#include "common_args.hpp"
#include "common_message.hpp"

#include <server.hpp>

#include <thread>

void listen_next(IPC::PipeServer& server)
{
    const std::optional<IPC::message> msg = server.read_message<IPC::message>();
    if (!msg.has_value()) {
        std::println("PID {}; Message reception failed", getpid());
    }
    std::println("PID {}; Received msg: {}", getpid(), IPC::to_string(msg.value()));
}

void run_listener(IPC::PipeServer& server)
{
    std::println("Listener started");
    for (int i = 0; i < 5; i++) {
        listen_next(server);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
