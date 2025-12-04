/**
 * Tracer - Single Header C++17 version
 *
 * A lightweight, header-only C++ profiling library that outputs Chrome Trace Event Format.
 *
 * @author Leo Barreiro - github.com/barreiroleo
 *
 * Usage:
 *   1. Define ENABLE_TRACING before including this header to enable tracing
 *   2. Call TRACE_SETUP("output.json") once at the start of your program
 *   3. Use TRACE_SCOPE(name) or TRACE_FN() to instrument your code
 *
 * Example:
 *   #define ENABLE_TRACING
 *   #include "tracer.hpp"
 *
 *   int main() {
 *       TRACE_SETUP("trace.json");
 *       TRACE_FN();
 *       // your code here
 *       return 0;
 *   }
 *
 * License: See LICENSE file
 * Full implementation with pipe support available at: ./src/Profiler/
 */

#pragma once
#ifndef ENABLE_TRACING
#define ENABLE_TRACING
#endif

#include <algorithm>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

namespace Tracer {
static constexpr std::string_view TRACE_EVENTS { R"({"traceEvents":[)" };
static constexpr std::string_view TRACE_EVENT_BODY { R"(],"displayTimeUnit":"ns"})" };
struct ChromeEvent {
    std::string name;
    std::string cat;
    char ph;
    int64_t ts;
    int pid;
    size_t tid;
    int64_t dur;
};

inline std::string serialize_to_json(const ChromeEvent& event)
{
    std::string name_sanitizer_buf {};
    std::string_view event_name = event.name;
    if (event.name.find('"') != event.name.npos) {
        std::replace_copy(event.name.begin(), event.name.end(), std::back_inserter(name_sanitizer_buf), '"', '\'');
        event_name = name_sanitizer_buf;
    }
    std::stringstream ss;
    ss << R"({"name":")" << event_name << R"(","cat":")" << event.cat << R"(","ph":")" << event.ph
       << R"(","ts":)" << event.ts << R"(,"pid":)" << event.pid << R"(,"tid":)" << event.tid
       << R"(,"dur":)" << event.dur << "}";
    return ss.str();
}

/// @brief Serialize ChromeEvent to stream format
inline std::ostream& serialize_to_stream(std::ostream& out, const ChromeEvent& event)
{
    out << event.name << '\n';
    out << event.cat << '\n';
    out << event.ph << '\n';
    out << event.ts << '\n';
    out << event.pid << '\n';
    out << event.tid << '\n';
    out << event.dur << '\n';
    return out;
}

/// @brief Deserialize ChromeEvent from stream format
inline std::istream& deserialize_from_stream(std::istream& in, ChromeEvent& event)
{
    std::getline(in, event.name);
    std::getline(in, event.cat);
    in >> event.ph;
    in.ignore(); // consume newline after ph
    in >> event.ts >> event.pid >> event.tid >> event.dur;
    return in;
}

inline std::ostream& operator<<(std::ostream& out, const ChromeEvent& event)
{
    return serialize_to_stream(out, event);
}

inline std::istream& operator>>(std::istream& in, ChromeEvent& event)
{
    return deserialize_from_stream(in, event);
}

} // namespace Tracer

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

inline std::ostream& operator<<(std::ostream& os, const Message& msg)
{
    return serialize(os, msg);
}

inline std::istream& operator>>(std::istream& in, Message& msg)
{
    return deserialize(in, msg);
}

inline std::string to_string(const Message& msg)
{
    std::stringstream ss;
    ss << "Size:" << msg.size() << "\n"
       << "{\n"
       << "  kind:" << static_cast<uint8_t>(msg.kind) << ",\n"
       << "  pid:" << msg.pid << ",\n"
       << "  length:" << msg.body.length() << ",\n"
       << "  body: \n"
       << msg.body << "\n"
       << "}\n";
    return ss.str();
}

} // namespace IPC

namespace IPC {
using FileDescriptor = int;

class PipeClient {
public:
    PipeClient(std::string_view path)
        : m_pid(getpid())
        , m_pipe_path(path)
    {
    }

    [[nodiscard]]
    std::optional<std::reference_wrapper<std::ofstream>> init()
    {
        namespace fs = std::filesystem;
        if (!fs::exists(m_pipe_path)) {
            std::cout << "Waiting for pipe to be created at " << m_pipe_path << "...\n";
            while (!fs::exists(m_pipe_path)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        m_pipe_stream.open(m_pipe_path.data(), std::ios::binary | std::ios::out);
        if (!m_pipe_stream.is_open() || m_pipe_stream.fail()) {
            std::cerr << "Failed to open pipe.\n";
            return std::nullopt;
        }
        return m_pipe_stream;
    }

    [[nodiscard]] bool write_message(const Message& msg)
    {
        m_pipe_stream << msg;
        if (m_pipe_stream.fail()) {
            std::cerr << "PID " << m_pid << ": Error while writing message. " << strerror(errno) << "\n";
            return false;
        }
        return true;
    }

    ~PipeClient()
    {
        m_pipe_stream.close();
        if (m_pipe_stream.fail()) {
            std::cerr << "PID " << m_pid << ": Error while closing pipe. " << strerror(errno) << "\n";
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipe_path;
    std::ofstream m_pipe_stream;
};

}

namespace Tracer {

class FileExporter {
public:
    static FileExporter& instance(std::string_view output_file = "trace.json")
    {
        static FileExporter instance { output_file };
        return instance;
    }

    void push_trace(const ChromeEvent& result)
    {
        static std::atomic_bool is_first_event { true };

        const auto json = serialize_to_json(result);

        std::lock_guard<std::mutex> lock(m_lock);
        {
            bool is_first = is_first_event.exchange(false);
            std::ignore = !is_first && m_trace_stream << ',';
            m_trace_stream << '\n'
                           << json;
        }
    }

private:
    FileExporter(std::string_view output_file)
        : m_trace_stream(std::ofstream(output_file.data()))
    {
        if (!m_trace_stream.is_open()) {
            throw std::runtime_error("Failed to open trace output file.");
        }
        m_trace_stream << TRACE_EVENTS;
    }

    ~FileExporter()
    {
        m_trace_stream << '\n'
                       << TRACE_EVENT_BODY;
    }

private:
    std::mutex m_lock;
    std::ofstream m_trace_stream;
};

} // namespace Tracer

namespace Tracer {

class IPCExporter {
public:
    static IPCExporter& instance(std::string_view pipe_path = "/tmp/trace.pipe")
    {
        static IPCExporter instance { pipe_path };
        return instance;
    }

    void push_trace(const ChromeEvent& result)
    {
        std::stringstream ss;
        ss << result;

        IPC::Message msg {
            .kind = IPC::MessageKind::DATA,
            .body = ss.str()
        };

        std::lock_guard<std::mutex> lock(m_lock);
        if (!m_pipe.write_message(msg)) {
            std::cerr << "Failed to send message..\n";
            return;
        }
    }

private:
    IPCExporter(std::string_view pipe_path)
        : m_pipe(pipe_path)
    {
        if (!m_pipe.init().has_value()) {
            std::exit(EXIT_FAILURE);
        }
    }

    ~IPCExporter()
    {
        const IPC::Message msg {
            .kind = IPC::MessageKind::STOP,
            .body = {}
        };
        std::ignore = m_pipe.write_message(msg);
    };

private:
    std::mutex m_lock;
    IPC::PipeClient m_pipe;
};

} // namespace Tracer

namespace Tracer {

template <class ExporterType = FileExporter>
class TraceScope {
public:
    using time_unit = std::chrono::microseconds;
    using high_resolution_clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<high_resolution_clock, time_unit>;

    TraceScope(std::string_view name, std::string_view cat = "Default")
        : m_name(name)
        , m_cat(cat)
        , m_start_time(get_unique_timestamp())
    {
    }

    ~TraceScope()
    {
        try {
            write_trace();
        } catch (...) {
            std::cerr << "Warning: Exception occurred while writing trace event.";
        }
    }

private:
    /// @brief Get a unique timestamp that's guaranteed to be monotonically increasing
    ///
    /// This solves the limitation on events with complete events (X) has the same timestamps
    /// - perf_text_importer_sample_no_frames
    static inline int64_t get_unique_timestamp()
    {
        using namespace std::chrono;
        static thread_local int64_t last_timestamp { 0 };
        auto current = time_point_cast<time_unit>(high_resolution_clock::now()).time_since_epoch().count();

        // Ensure monotonic increase by incrementing if timestamp hasn't advanced
        std::ignore = (current <= last_timestamp) && (current = last_timestamp + 1);

        last_timestamp = current;
        return current;
    }

    void write_trace()
    {
        // ToDo: Make it platform independent.
        // Perfetto needs 4 digits id, but hash for thread::id is 19 digits long.
        // static thread_local auto tid = std::hash<std::thread::id> {}(std::this_thread::get_id());
        static thread_local int tid = static_cast<int>(syscall(SYS_gettid));
        const auto end_time = get_unique_timestamp();

        ChromeEvent m_trace_data {
            .name = std::move(m_name),
            .cat = std::move(m_cat),
            .ph = 'X',
            .ts = m_start_time,
            .pid = getpid(),
            .tid = static_cast<size_t>(tid),
            .dur = (end_time - m_start_time),
        };

        ExporterType::instance().push_trace(m_trace_data);
    }

    std::string m_name;
    std::string m_cat;
    const int64_t m_start_time;
};

// Type aliases for common use cases
using Trace = TraceScope<FileExporter>;

// Type alias for IPC-based tracing
using IPCTrace = TraceScope<IPCExporter>;

} // namespace Tracer

// Macros for file-based tracing (default)
#ifdef ENABLE_TRACING
#define TRACE_SCOPE_CAT(name, cat) Tracer::Trace trace_##__LINE__(name, cat)
#define TRACE_SCOPE(name) Tracer::Trace trace_##__LINE__(name)
#define TRACE_FN_CAT(cat) TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define TRACE_FN() TRACE_SCOPE(__FUNCTION__)
#else
#define TRACE_SCOPE_CAT(name, cat)
#define TRACE_SCOPE(name)
#define TRACE_FN_CAT(cat)
#define TRACE_FN()
#endif // ENABLE_TRACING

// Macros for IPC-based tracing
#ifdef ENABLE_TRACING
#define IPC_TRACE_SETUP(pipe) Tracer::IPCExporter::instance(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat) Tracer::IPCTrace trace_##__LINE__(name, cat)
#define IPC_TRACE_SCOPE(name) Tracer::IPCTrace trace_##__LINE__(name)
#define IPC_TRACE_FN_CAT(cat) IPC_TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define IPC_TRACE_FN() IPC_TRACE_SCOPE(__FUNCTION__)
#else
#define IPC_TRACE_SETUP(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat)
#define IPC_TRACE_SCOPE(name)
#define IPC_TRACE_FN_CAT(cat)
#define IPC_TRACE_FN()
#endif // ENABLE_TRACING

#ifdef ENABLE_TRACING
#define TRACE_SETUP(file) Tracer::FileExporter::instance(file)
#else
#define TRACE_SETUP(file)
#endif // ENABLE_TRACING
