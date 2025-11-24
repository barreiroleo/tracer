#include <args.hpp>
#include <print>
#include <string_view>

struct Options {
    std::string pipename { "/var/lock/pipename" };
};

static std::string_view help_msg = R"(
Usage: client [--pipe <pipename>]
Options:
    --pipe <pipename>   Specify the named pipe to use (default: /var/lock/pipename)
    --help              Show this help message
)";

inline auto command_handler(std::string_view key, std::string_view value, Options& parser_out) -> Args::Result
{
    using Code = Args::Result::Code;

    if (key == "--pipe") {
        if (value.empty()) {
            return { Code::ERROR, "Error: --pipe requires a pipename argument." };
        }
        parser_out.pipename = value;
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
    const auto options = Args::parse<Options>(argc, argv, command_handler);

    std::println("Using named pipe: {}", options.pipename);

    return 0;
}
