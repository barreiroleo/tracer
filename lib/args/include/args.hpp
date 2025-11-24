#pragma once

#include <cstdint>
#include <functional>
#include <print>
#include <string_view>

namespace Args {

namespace Colors {
    [[maybe_unused]] constexpr std::string_view RED { "\033[31m" };
    [[maybe_unused]] constexpr std::string_view GREEN { "\033[32m" };
    [[maybe_unused]] constexpr std::string_view NC { "\033[0m" };
} // namespace Colors

/// @brief Result of command line argument parsing
/// @note Used by CommandHandler to indicate the result of processing a command line argument
struct Result {
    enum class Code : uint8_t {
        OK,
        ERROR,
        UNHANDLED,
    };
    Code status { Code::UNHANDLED };
    std::string message {};
};

/// @brief Type alias for command handler function
template <typename T>
using CommandHandler = std::function<Result(std::string_view key, std::string_view value, T& parsed_data)>;

/// @brief Process the result of a command handler
/// @param key The command line argument key
/// @param handler_result The result from the command handler
/// @return true if processing was successful, false otherwise
static inline auto process_result(std::string_view key, Result handler_result) -> bool
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

/// @brief Parse command line arguments
/// @tparam T Type of the parsed data
/// @param argc Argument count
/// @param argv Argument vector
/// @param handler Command handler function
/// @return true if parsing was successful, false otherwise
template <typename T>
static inline auto parse(int argc, char* argv[], CommandHandler<T> handler) -> T
{
    const auto args = std::span<char*>(argv, argc);
    if (argc <= 1) {
        std::println("{}Error: No arguments provided{}", Colors::RED, Colors::NC);
        std::exit(1);
    }

    T parsed_data {};
    for (auto it = args.cbegin() + 1; it != args.cend(); ++it) {
        const std::string_view key { *it };

        std::string_view value = "";
        if (std::next(it) != args.cend()) {
            if (const std::string_view next = *std::next(it); !next.starts_with("-")) {
                value = *std::next(it);
                ++it;
            }
        }
        // std::println("{}Processing key: '{}', value: '{}'{}", Colors::GREEN, key, value, Colors::NC);

        Result result = handler(key, value, parsed_data);
        if (!process_result(key, result)) {
            std::exit(1);
        }
    }
    return parsed_data;
}

} // namespace Args
