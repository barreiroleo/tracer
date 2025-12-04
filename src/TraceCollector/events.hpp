#pragma once

#include <IPC/message.hpp>
#include <IPC/server.hpp>
#include <Profiler/chrome_event.hpp>
#include <Profiler/exporters/file_exporter.hpp>

#include <print>
#include <thread>

inline void flush_events(Tracer::FileExporter& exporter, std::vector<std::string> events)
{
    for (const auto& raw_event : events) {
        Tracer::ChromeEvent event {};
        std::stringstream ss(raw_event);
        ss >> event;
        exporter.push_trace(event);
    }
}

inline void async_flush_events(Tracer::FileExporter& exporter, std::vector<std::string>&& raw_events)
{
    std::vector<std::string> events_to_flush;
    events_to_flush.reserve(raw_events.size());
    events_to_flush.swap(raw_events);

    std::thread([&exporter, events_to_flush = std::move(events_to_flush)]() {
        flush_events(exporter, std::move(events_to_flush));
    }).detach();
}

inline void run(IPC::PipeServer& server, std::string_view output_file)
{
    Tracer::FileExporter& exporter = Tracer::FileExporter::instance(output_file);

    std::vector<std::string> raw_events {};
    raw_events.reserve(100);

    const auto message_handler = [&](const IPC::Message& msg) {
        // std::println("Received message:\n{}", IPC::to_string(msg));
        raw_events.emplace_back(msg.body);
        if (raw_events.size() >= raw_events.capacity()) {
            async_flush_events(exporter, std::move(raw_events));
        }
    };

    const auto stop_handler = [&]() {
        if (!raw_events.empty()) {
            flush_events(exporter, raw_events);
        }
        std::println("Trace collector shutdown complete");
    };

    server.run(message_handler, stop_handler);
}
