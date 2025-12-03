#include <Args/args.hpp>
#include <IPC/message.hpp>
#include <IPC/server.hpp>

#include <chrome_event.hpp>
#include <exporters/file_exporter.hpp>
#include <serialization.hpp>

#include <sstream>

void run_listener(IPC::PipeServer& server, Tracer::FileExporter& exporter)
{
    std::println("Trace collector started");
    bool running = true;

    std::vector<Tracer::ChromeEvent> events(1000);
    while (running) {
        const std::optional<IPC::Message> msg = server.read_message();

        if (!msg.has_value()) {
            std::println("PID {}; Message reception failed", getpid());
            continue;
        }

        if (msg->kind == IPC::MessageKind::STOP) {
            running = false;
            std::println("Trace collector stopping as per STOP message");
            continue;
        }

        // Deserialize the event from the message body
        std::stringstream ss(msg->body);
        Tracer::ChromeEvent event = events.emplace_back();
        ss >> event;
    }
    for (Tracer::ChromeEvent& event : events) {
        exporter.push_trace(std::move(event));
    }
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

    // Initialize file exporter
    auto& exporter = Tracer::FileExporter::instance(output_file);

    // Run the listener
    run_listener(server, exporter);

    std::println("Trace collector shutdown complete");
    return 0;
}
