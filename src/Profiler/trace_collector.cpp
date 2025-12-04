#include <Args/args.hpp>
#include <IPC/message.hpp>
#include <IPC/server.hpp>

#include <chrome_event.hpp>
#include <exporters/file_exporter.hpp>

#include <sstream>
#include <thread>

void flush_events(Tracer::FileExporter& exporter, std::vector<std::string> events)
{
    for (const auto& raw_event : events) {
        Tracer::ChromeEvent event {};
        std::stringstream ss(raw_event);
        ss >> event;
        exporter.push_trace(event);
    }
}

void async_flush_events(Tracer::FileExporter& exporter, std::vector<std::string>&& raw_events)
{
    std::vector<std::string> events_to_flush;
    events_to_flush.reserve(raw_events.size());
    events_to_flush.swap(raw_events);

    std::thread([&exporter, events_to_flush = std::move(events_to_flush)]() {
        flush_events(exporter, std::move(events_to_flush));
    }).detach();
}

void run(IPC::PipeServer& server, std::string_view output_file)
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

struct ArgsOpts {
    std::string pipe_path = "/tmp/tracer.pipe";
    std::string output_file = "trace.json";
};

inline Args::Result command_handler(std::string_view key, std::string_view value, ArgsOpts& options)
{
    if (key == "--pipe" && !value.empty()) {
        options.pipe_path = value;
        return { Args::Result::Code::OK };
    }
    if (key == "--output" && !value.empty()) {
        options.output_file = value;
        return { Args::Result::Code::OK };
    }
    return { Args::Result::Code::UNHANDLED };
}

int main(int argc, char** argv)
{
    const ArgsOpts options = Args::parse<ArgsOpts>(argc, argv, command_handler);
    std::string_view pipe_path = options.pipe_path;
    std::string_view output_file = options.output_file;

    std::println("Starting trace collector:");
    std::println("  Pipe: {}", pipe_path);
    std::println("  Output: {}", output_file);

    // Initialize server
    IPC::PipeServer server { pipe_path };
    if (!server.init().has_value()) {
        std::println(stderr, "Failed to initialize server");
        std::exit(EXIT_FAILURE);
    }

    run(server, output_file);

    return 0;
}
