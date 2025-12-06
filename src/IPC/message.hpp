#pragma once

#include <cstdint>
#include <iostream>

namespace IPC {

enum class MessageKind : uint8_t {
    DATA,
    STOP,
};

struct Message {
    MessageKind kind;
    int pid;
    std::string body;

    size_t size() const;
    void inspect() const;
    std::string to_json() const;
};

std::ostream& serialize(std::ostream& os, const Message& msg);

std::istream& deserialize(std::istream& in, Message& msg);

inline std::ostream& operator<<(std::ostream& os, const Message& msg)
{
    return serialize(os, msg);
}

inline std::istream& operator>>(std::istream& in, Message& msg)
{
    return deserialize(in, msg);
}

} // namespace IPC
