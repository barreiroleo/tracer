#include "args.hpp"

#include <print>
#include <span>
#include <string_view>

namespace Args {

namespace Colors {
    [[maybe_unused]] constexpr std::string_view RED { "\033[31m" };
    [[maybe_unused]] constexpr std::string_view GREEN { "\033[32m" };
    [[maybe_unused]] constexpr std::string_view NC { "\033[0m" };
} // namespace Colors

/// @brief Process the result of a command handler
/// @param key The command line argument key
/// @param handler_result The result from the command handler
/// @return true if processing was successful, false otherwise
auto process_result(std::string_view key, Result handler_result) -> bool
{
    switch (handler_result.status) {
    case Result::Code::OK:
        return true;
    case Result::Code::ERROR:
        std::println("{}{}{}", Colors::RED, handler_result.message, Colors::NC);
        return false;
    case Result::Code::UNHANDLED:
        std::println("{}Error: Unknown option '{}'{}", Colors::RED, key, Colors::NC);
        return false;
    default:
        std::println("{}Error: Invalid result code for option '{}'{}", Colors::RED, key, Colors::NC);
        return false;
    }
}

/// @brief Internal helper for parsing command line arguments
/// @param argc Argument count
/// @param argv Argument vector
/// @param handler Command handler function (takes key and value, returns Result)
/// @return true if parsing was successful, false otherwise
auto parse_args_impl(int argc, char* argv[], const std::function<Result(std::string_view, std::string_view)>& handler) -> bool
{
    const auto args = std::span<char*>(argv, argc);
    if (argc <= 1) {
        std::println("{}Error: No arguments provided{}", Colors::RED, Colors::NC);
        return false;
    }

    for (auto it = args.begin() + 1; it != args.end(); ++it) {
        const std::string_view key { *it };

        std::string_view value = "";
        if (std::next(it) != args.end()) {
            if (const std::string_view next = *std::next(it); !next.starts_with("-")) {
                value = *std::next(it);
                ++it;
            }
        }
        // std::println("{}Processing key: '{}', value: '{}'{}", Colors::GREEN, key, value, Colors::NC);

        Result result = handler(key, value);
        if (!process_result(key, result)) {
            return false;
        }
    }
    return true;
}

}
