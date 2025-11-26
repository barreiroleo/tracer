#include <src/IPC/server.hpp>
#include <src/cli_opts.hpp>

void run_listener(IPC::FileDescriptor /*file_descriptor*/)
{
    std::println("Listener running...");
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
