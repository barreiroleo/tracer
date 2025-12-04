#include "trace.hpp"

#include <chrono>

// Explicit template instantiation
template class Tracer::TraceScope<Tracer::FileExporter>;
template class Tracer::TraceScope<Tracer::IPCExporter>;

namespace Tracer {

/// @brief Get a unique timestamp that's guaranteed to be monotonically increasing
///
/// This solves the limitation on events with complete events (X) has the same timestamps
/// - perf_text_importer_sample_no_frames
int64_t get_unique_timestamp()
{
    static thread_local int64_t last_timestamp { 0 };

    using namespace std::chrono;
    auto current = time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count();

    // Ensure monotonic increase by incrementing if timestamp hasn't advanced
    std::ignore = (current <= last_timestamp) && (current = last_timestamp + 1);

    last_timestamp = current;
    return current;
}

template <class T>
TraceScope<T>::TraceScope(std::string_view name, std::string_view cat)
    : m_name(name)
    , m_cat(cat)
    , m_start_time(get_unique_timestamp()) {};

template <class T>
TraceScope<T>::~TraceScope()
{
    try {
        write_trace();
    } catch (...) {
        std::cerr << "Warning: Exception occurred while writing trace event.";
    }
}

template <class T>
void TraceScope<T>::write_trace()
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

    T::instance().push_trace(m_trace_data);
}

} // namespace Tracer
