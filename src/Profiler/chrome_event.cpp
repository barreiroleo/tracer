#include "chrome_event.hpp"

#include <algorithm>
#include <sstream>

namespace Tracer {

std::string serialize_to_json(const ChromeEvent& event)
{
    std::string event_name = event.name;
    if (event.name.find('"') != std::string::npos) {
        std::string sanitized;
        std::replace_copy(event.name.begin(), event.name.end(), std::back_inserter(sanitized), '"', '\'');
        event_name = std::move(sanitized);
    }

    std::stringstream ss;
    ss << R"({"name":")" << event_name << R"(","cat":")" << event.cat << R"(","ph":")" << event.ph
       << R"(","ts":)" << event.ts << R"(,"pid":)" << event.pid << R"(,"tid":)" << event.tid
       << R"(,"dur":)" << event.dur << "}";
    return ss.str();
}

std::ostream& serialize_to_stream(std::ostream& out, const ChromeEvent& event)
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
std::istream& deserialize_from_stream(std::istream& in, ChromeEvent& event)
{
    std::getline(in, event.name);
    std::getline(in, event.cat);
    in >> event.ph;
    in.ignore(); // consume newline after ph
    in >> event.ts >> event.pid >> event.tid >> event.dur;
    return in;
}

} // namespace Tracer
