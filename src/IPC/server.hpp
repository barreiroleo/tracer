#include <optional>
#include <print>
#include <string>

#include <fcntl.h> // open
#include <string.h> // strerror
#include <sys/stat.h> // mkfifo
#include <unistd.h> // write

namespace IPC {
using FileDescriptor = int;

class PipeServer {
public:
    PipeServer(std::string path)
        : m_pid(getpid())
        , m_pipename(std::move(path))
    {
    }

    [[nodiscard]]
    std::optional<FileDescriptor> init()
    {
        // Create the named pipe (fifo) with permission
        if (const auto result = mkfifo(m_pipename.data(), 0666); result < 0) {
            std::println("Error when creating FIFO. {}.", strerror(errno));
            return std::nullopt;
        }

        // Try to open pipe for read only
        m_file_descriptor = open(m_pipename.data(), O_RDONLY);
        if (m_file_descriptor < 0 && errno == ENOENT) {
            std::println("Process {}: Something went wrong creating the pipe", m_pid);
            return std::nullopt;
        }
        return m_file_descriptor;
    }

    ~PipeServer()
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
