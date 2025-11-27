#pragma once

#include <Args/args.hpp>

namespace IPC {

struct message {
    int pid;
    int counter;
};

} // namespace IPC

struct Options {
    std::string pipename = "/var/lock/pipename";
};

inline Args::Result command_handler(std::string_view key, std::string_view value, Options& options)
{
    if (key == "--pipe" && !value.empty()) {
        options.pipename = value;
        return { Args::Result::Code::OK };
    }
    return { Args::Result::Code::UNHANDLED };
}
