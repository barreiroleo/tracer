#include <Args/args.hpp>
#include <IPC/server.hpp>

struct message {
    int pid;
    int counter;
};

void run_listener(IPC::FileDescriptor /*file_descriptor*/)
{
    std::println("Listener running...");
}

struct Options {
    std::string pipename = "/tmp/tracer.pipe";
};

inline Args::Result command_handler(std::string_view key, std::string_view value, Options& options)
{
    if (key == "--pipe" && !value.empty()) {
        options.pipename = value;
        return { Args::Result::Code::OK };
    }
    return { Args::Result::Code::UNHANDLED };
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
