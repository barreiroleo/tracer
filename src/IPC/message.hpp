#pragma once

#include <cstddef>
#include <cstdint>

namespace IPC {

enum class MessageKind : uint8_t {
    DATA,
    STOP,
};

struct Message {
    MessageKind kind {};
    size_t length {};
    char body[1024] {};

    /// @brief Calculate the actual size of the message based on body length
    /// @return Size in bytes needed to transmit this message
    constexpr size_t size() const noexcept
    {
        return offsetof(Message, body) + length;
    }
};

} // namespace IPC
