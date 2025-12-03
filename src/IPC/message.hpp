// TODO(lbarreiro): Use explicit endianness for serialization/deserialization.
// reinterpret_cast<char*> is not portable between little-endian/big-endian.
// It will block network communication support.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <iostream>
#include <sstream>

namespace IPC {

struct Message;
std::ostream& operator<<(std::ostream& os, const Message& msg);
std::istream& operator>>(std::istream& in, Message& msg);
std::string to_string(const Message& msg);

enum class MessageKind : uint8_t {
    DATA,
    STOP,
};

struct Message {
    MessageKind kind {};
    int pid {};
    std::string body {};

    void inspect() const
    {
        std::cout << to_string(*this);
    }

    constexpr size_t size() const
    {
        return sizeof(kind) + sizeof(pid) + body.length();
    }
};

inline std::ostream& operator<<(std::ostream& os, const Message& msg)
{
    size_t length = msg.body.length();
    os.write(reinterpret_cast<const char*>(&msg.kind), sizeof(msg.kind));
    os.write(reinterpret_cast<const char*>(&msg.pid), sizeof(msg.pid));
    os.write(reinterpret_cast<const char*>(&length), sizeof(length));
    os.write(msg.body.data(), length);
    return os;
}

inline std::istream& operator>>(std::istream& in, Message& msg)
{
    size_t length {};
    in.read(reinterpret_cast<char*>(&msg.kind), sizeof(msg.kind));
    in.read(reinterpret_cast<char*>(&msg.pid), sizeof(msg.pid));
    in.read(reinterpret_cast<char*>(&length), sizeof(length));
    msg.body.resize(length);
    in.read(msg.body.data(), length);
    return in;
}

inline std::string to_string(const Message& msg)
{
    std::stringstream ss;
    ss << "Size:" << msg.size() << "\n"
       << "{\n"
       << "  kind:" << static_cast<uint8_t>(msg.kind) << ",\n"
       << "  pid:" << msg.pid << ",\n"
       << "  length:" << msg.body.length() << ",\n"
       << "  body:" << msg.body << "\n"
       << "}\n";
    return ss.str();
}

} // namespace IPC
