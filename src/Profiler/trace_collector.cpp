#include <Args/args.hpp>
#include <IPC/message.hpp>
#include <IPC/server.hpp>

#include <chrome_event.hpp>
#include <exporters/file_exporter.hpp>
#include <serialization.hpp>

#include <sstream>

void run(IPC::PipeServer& server, std::string_view output_file)
{
    // File Exporter
    Tracer::FileExporter& exporter = Tracer::FileExporter::instance(output_file);

    // TODO(lbarreiro): Buffer events and flush periodically to avoid high memory usage
    std::vector<std::string> raw_events {};
    raw_events.reserve(1024);

    const auto message_handler = [&raw_events](const IPC::Message& msg) {
        // std::println("Received message:\n{}", IPC::to_string(msg));
        raw_events.emplace_back(msg.body);
    };

    const auto stop_handler = [&exporter, &raw_events]() {
        std::println("Trace collector shutdown complete");
        for (const auto& raw_event : raw_events) {
            Tracer::ChromeEvent event {};
            std::stringstream ss(raw_event);
            ss >> event;
            exporter.push_trace(event);
        }
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
