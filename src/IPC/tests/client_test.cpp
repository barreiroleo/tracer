#include "common_args.hpp"

#include <client.hpp>
#include <print>

#include <thread>

std::string MSG_LIST[] = {
    std::string(454, '='),
    std::string(1024, '='),
    std::string(1138, '=')
};

void send_message(IPC::PipeClient& client, std::string_view content)
{
    IPC::Message msg {
        .kind = IPC::MessageKind::DATA,
        .pid = getpid(),
        .body = content.data(),
    };
    if (!client.write_message(msg)) {
        std::println(stderr, "PID {}: Failed to send message.", msg.pid);
        return;
    }
    std::println("PID {}: Sent {} bytes: Content {}", msg.pid, msg.size(), msg.to_json());
}

void send_stop_message(IPC::PipeClient& client)
{
    static IPC::Message msg {
        .kind = IPC::MessageKind::STOP,
        .pid = getpid(),
    };
    if (!client.write_message(msg)) {
        std::println(stderr, "PID {}: Failed to send message.", msg.pid);
        return;
    }
    std::println("PID {}: Sent {} bytes: Content {}", msg.pid, msg.size(), msg.to_json());
}

void run_writer(IPC::PipeClient& client)
{
    for (size_t i = 0; i < 5; i++) {
        const std::string_view content = MSG_LIST[i % std::size(MSG_LIST)];
        send_message(client, content);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    send_stop_message(client);
}

int main(int argc, char* argv[])
{
    const ArgsOpts options = Args::parse<ArgsOpts>(argc, argv, command_handler);
    std::string_view pipe_path = options.pipe_path;

    IPC::PipeClient client { pipe_path.data() };
    if (!client.init()) {
        std::exit(EXIT_FAILURE);
    }

    run_writer(client);
    return 0;
}
