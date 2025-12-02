#pragma once

#include <Args/args.hpp>
#include <string>

struct ArgsOpts {
    std::string pipe_path = "/tmp/tracer.pipe";
};

inline Args::Result command_handler(std::string_view key, std::string_view value, ArgsOpts& options)
{
    if (key == "--pipe" && !value.empty()) {
        options.pipe_path = value;
        return { Args::Result::Code::OK };
    }
    return { Args::Result::Code::UNHANDLED };
}
