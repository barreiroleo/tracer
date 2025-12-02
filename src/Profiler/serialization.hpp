#pragma once

#include "chrome_event.hpp"

#include <algorithm>
#include <iostream>
#include <string>

#ifdef __cpp_lib_format
#include <format>
#else
#include <sstream>
#endif

namespace Tracer {

/// @brief Serialize ChromeEvent to JSON format
/// @param event The event to serialize
/// @return JSON string representation
inline std::string serialize_to_json(const ChromeEvent& event)
{
    std::string_view event_name = event.name;
    if (event.name.contains('"')) {
        std::string name_buf {};
        std::replace_copy(event.name.begin(), event.name.end(), std::back_inserter(name_buf), '"', '\'');
        event_name = name_buf;
    }
#ifdef __cpp_lib_format
    return std::format(R"({{"name":"{}","cat":"{}","ph":"{}","ts":{},"pid":{},"tid":{},"dur":{}}})",
        event_name, event.cat, event.ph, event.ts, event.pid, event.tid, event.dur);
#else
    std::stringstream ss;
    ss << R"({"name":")" << event_name << R"(","cat":")" << event.cat << R"(","ph":")" << event.ph
       << R"(","ts":)" << event.ts << R"(,"pid":)" << event.pid << R"(,"tid":)" << event.tid
       << R"(,"dur":)" << event.dur << "}";
    return ss.str();
#endif
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
    in >> event.name;
    in >> event.cat;
    in >> event.ph;
    in >> event.ts;
    in >> event.pid;
    in >> event.tid;
    in >> event.dur;
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
