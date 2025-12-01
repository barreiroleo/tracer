#pragma once

#include <cstring>
#include <iostream>
#include <sstream>

namespace IPC {

enum class MessageKind : uint8_t {
    DATA,
    STOP,
};

struct Message {
    MessageKind kind {};
    int pid {};
    size_t length {};
    char body[1024] {};

    /// @brief Calculate the actual size of the message based on body length
    /// @return Size in bytes needed to transmit this message
    constexpr size_t size() const noexcept
    {
        return offsetof(Message, body) + length;
    }

    [[nodiscard]]
    bool set_body(std::string_view msg) noexcept;
};

[[nodiscard]]
inline bool Message::set_body(std::string_view msg) noexcept
{
    if (msg.length() > sizeof(IPC::Message::body)) {
        return false;
    }
    std::memcpy(body, msg.data(), msg.length());
    length = msg.length();
    return true;
}

[[nodiscard]]
inline std::string to_string(const Message& msg)
{
    // return std::format("PID: {}, Length: {}, Body: {}", msg.pid, msg.length, msg.body);
    std::stringstream ss;
    ss << "PID: " << msg.pid << ", Length: " << msg.length << ", Body: " << std::string_view(msg.body);
    return ss.str();
}

inline std::ostream& operator<<(std::ostream& out, Message& msg)
{
    out << msg.pid << msg.length << std::string_view(msg.body);
    return out;
}

inline std::istream& operator>>(std::istream& in, Message& msg)
{
    in >> msg.pid >> msg.length;
    if (msg.length > sizeof(msg.body)) {
        msg.length = sizeof(msg.body);
        std::cerr << "Warning: message length exceeds max size\n";
    }
    in.read(msg.body, msg.length);
    return in;
}

} // namespace IPC
