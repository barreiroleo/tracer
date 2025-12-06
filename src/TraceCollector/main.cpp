#include "args.hpp"
#include "events.hpp"

int main(int argc, char** argv)
{
    const ArgsOpts options = Args::parse<ArgsOpts>(argc, argv, command_handler);
    std::string_view pipe_path = options.pipe_path;
    std::string_view output_file = options.output_file;

    std::println("Starting trace collector:");
    std::println("  Pipe: {}", pipe_path);
    std::println("  Output: {}", output_file);

    // Initialize server
    IPC::PipeServer server { pipe_path };
    if (!server.init()) {
        std::println(stderr, "Failed to initialize server");
        std::exit(EXIT_FAILURE);
    }

    run(server, output_file);

    return 0;
}
