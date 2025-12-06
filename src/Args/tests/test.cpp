#include <args.hpp>

#include <print>
#include <string_view>

struct ArgsOpts {
    std::string pipe_path { "/var/lock/pipename" };
};

static std::string_view help_msg = R"(
Usage: client [--pipe <pipename>]
Options:
    --pipe <pipename>   Specify the named pipe to use (default: /var/lock/pipename)
    --help              Show this help message
)";

auto command_handler(std::string_view key, std::string_view value, ArgsOpts& parser_out) -> Args::Result
{
    using Code = Args::Result::Code;

    if (key == "--pipe") {
        if (value.empty()) {
            return { Code::ERROR, "Error: --pipe requires a pipename argument." };
        }
        parser_out.pipe_path = value;
        return { Code::OK };
    }
    if (key == "--help") {
        std::println("{}", help_msg);
        std::exit(0);
    }
    return { Code::UNHANDLED };
};

int main(int argc, char* argv[])
{
    const auto options = Args::parse<ArgsOpts>(argc, argv, command_handler);

    std::println("Using named pipe: {}", options.pipe_path);

    return 0;
}
