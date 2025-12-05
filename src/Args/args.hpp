#pragma once

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <string>

namespace Args {

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
auto process_result(std::string_view key, Result handler_result) -> bool;

/// @brief Internal helper for parsing command line arguments
/// @param argc Argument count
/// @param argv Argument vector
/// @param handler Command handler function (takes key and value, returns Result)
/// @return true if parsing was successful, false otherwise
auto parse_args_impl(int argc, char* argv[], const std::function<Result(std::string_view, std::string_view)>& handler) -> bool;

/// @brief Parse command line arguments
/// @tparam T Type of the parsed data
/// @param argc Argument count
/// @param argv Argument vector
/// @param handler Command handler function
/// @return Parsed data of type T
template <typename T>
auto parse(int argc, char* argv[], CommandHandler<T> handler) -> T
{
    T parsed_data {};

    auto wrapper = [&](std::string_view key, std::string_view value) -> Result {
        return handler(key, value, parsed_data);
    };

    if (!parse_args_impl(argc, argv, wrapper)) {
        std::exit(1);
    }

    return parsed_data;
}

} // namespace Args
