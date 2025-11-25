#include <optional>
#include <print>
#include <string>
#include <thread>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

class PipeClient {
public:
    PipeClient(std::string path)
        : m_pid(getpid())
        , m_pipename(std::move(path))
    {
    }

    [[nodiscard]]
    std::optional<FileDescriptor> init()
    {
        // Try to open pipe for writing only
        int file_descriptor = open(m_pipename.data(), O_WRONLY);
        while (file_descriptor < 0 && errno == ENOENT) {
            std::println("Process {}: Pipe not found. Retrying...", m_pid);
            file_descriptor = open(m_pipename.data(), O_WRONLY);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return file_descriptor;
    }

    ~PipeClient()
    {
        if (close(m_file_descriptor) != 0) {
            std::println("Process {}: Error while closing pipe. {}", m_pid, strerror(errno));
        }
        if (unlink(m_pipename.data()) != 0) {
            std::println("Process {}: Error while unlinking pipe. {}", m_pid, strerror(errno));
        }
    }

private:
    pid_t m_pid {};
    std::string m_pipename;
    FileDescriptor m_file_descriptor { -1 };
};

}
