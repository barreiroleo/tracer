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

enum class MessageKind : uint8_t {
    DATA,
    STOP,
};

struct Message {
    MessageKind kind {};
    int pid {};
    std::string body {};

    size_t size() const
    {
        return sizeof(kind) + sizeof(pid) + body.length();
    }

    void inspect() const
    {
        std::cout << "Size: " << size() << '\n'
                  << to_json() << '\n';
    }

    std::string to_json() const
    {
        std::stringstream ss;
        ss << "{\n"
           << "  kind:" << static_cast<uint8_t>(kind) << ",\n"
           << "  pid:" << pid << ",\n"
           << "  length:" << body.length() << ",\n"
           << "  body: \n"
           << body << "\n"
           << "}";
        return ss.str();
    }
};

inline std::ostream& serialize(std::ostream& os, const Message& msg)
{
    size_t length = msg.body.length();
    os.write(reinterpret_cast<const char*>(&msg.kind), sizeof(msg.kind));
    os.write(reinterpret_cast<const char*>(&msg.pid), sizeof(msg.pid));
    os.write(reinterpret_cast<const char*>(&length), sizeof(length));
    os.write(msg.body.data(), length);
    return os;
}

inline std::istream& deserialize(std::istream& in, Message& msg)
{
    size_t length {};
    in.read(reinterpret_cast<char*>(&msg.kind), sizeof(msg.kind));
    in.read(reinterpret_cast<char*>(&msg.pid), sizeof(msg.pid));
    in.read(reinterpret_cast<char*>(&length), sizeof(length));
    msg.body.resize(length);
    in.read(msg.body.data(), length);
    return in;
}

inline std::ostream& operator<<(std::ostream& os, const Message& msg) { return serialize(os, msg); }

inline std::istream& operator>>(std::istream& in, Message& msg) { return deserialize(in, msg); }

} // namespace IPC
