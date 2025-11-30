#pragma once

#include <cstring>
#include <format>
#include <iostream>
#include <string>

namespace IPC {
using namespace std::literals::string_view_literals;

struct message {
    int pid;
    int counter;
    size_t length;
    char body[1024];

    /// @brief Calculate the actual size of the message based on body length
    /// @return Size in bytes needed to transmit this message
    constexpr size_t size() const noexcept
    {
        return offsetof(message, body) + length;
    }

    [[nodiscard]] bool set_body(std::string_view msg) noexcept
    {
        if (msg.length() > sizeof(IPC::message::body)) {
            return false;
        }
        std::memcpy(body, msg.data(), sizeof(IPC::message::body));
        length = msg.length();
        return true;
    }
};

inline std::string to_string(const message& msg)
{
    return std::format("PID: {}, Counter: {}, Length: {}, Body: {}", msg.pid, msg.counter, msg.length, msg.body);
}

inline std::ostream& operator<<(std::ostream& out, message& msg)
{
    out << msg.pid << msg.counter << msg.length << std::string_view(msg.body);
    return out;
}

inline std::istream& operator>>(std::istream& in, message& msg)
{
    in >> msg.pid >> msg.counter >> msg.length;

    if (msg.length > sizeof(msg.body)) {
        std::cerr << "Warning: message length exceeds max body size\n"sv;
        msg.length = sizeof(msg.body);
    }
    in.read(msg.body, msg.length);

    return in;
}

} // namespace IPC
