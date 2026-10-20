#ifndef SYSTERM
#define SYSTERM

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <spawn.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

extern char** environ;

struct CommandResult {
    int total = 0;
    int succeeded = 0;
    int failed = 0;
    bool success() const { return failed == 0; }
};

class SystemTermux {
public:
    static constexpr int MAX_CONCURRENT = 4;
    static constexpr int WAKE_LOCK_THRESHOLD = 3;

private:
    // ------------------------------------------------------------------
    // Logging: ofstream persistente, flush periódico, rotación a 1 MiB.
    // ------------------------------------------------------------------
    class LogSink {
    public:
        static LogSink& instance()
        {
            static LogSink sink;
            return sink;
        }

        void write(const std::string& command, int exitCode)
        {
            std::lock_guard<std::mutex> lk(m_);
            if (!ensureOpen())
                return;

            std::time_t now = std::time(nullptr);
            std::tm tmBuf {};
            localtime_r(&now, &tmBuf);
            char timeBuf[32];
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tmBuf);

            file_ << "========================================\n"
                  << "Fecha: " << timeBuf << "\n"
                  << "PID: " << getpid() << "\n"
                  << "Comando: " << command << "\n"
                  << "Exit code: " << exitCode << "\n"
                  << "Estado: " << (exitCode == 0 ? "SUCCESS" : "FAILED") << "\n";

            if (++count_ % 16 == 0)
                file_.flush();
        }

        ~LogSink()
        {
            std::lock_guard<std::mutex> lk(m_);
            if (file_.is_open())
                file_.flush();
        }

    private:
        LogSink() = default;

        static std::string path()
        {
            const char* home = std::getenv("HOME");
            return home ? std::string(home) + "/.spin.log" : ".spin.log";
        }

        static void rotateIfNeeded()
        {
            const std::string p = path();
            struct stat st {};
            if (::stat(p.c_str(), &st) != 0)
                return;
            if (static_cast<size_t>(st.st_size) < 1024 * 1024)
                return;

            const std::string old = p + ".1";
            ::unlink(old.c_str());
            ::rename(p.c_str(), old.c_str());
        }

        bool ensureOpen()
        {
            if (file_.is_open())
                return true;
            rotateIfNeeded();
            file_.open(path(), std::ios::app);
            return file_.is_open();
        }

        std::mutex m_;
        std::ofstream file_;
        size_t count_ = 0;
    };

    // ------------------------------------------------------------------
    // Wake lock vía posix_spawn (sin system()).
    // ------------------------------------------------------------------
    class WakeLock {
    public:
        explicit WakeLock(bool enable)
            : active_(enable)
        {
            if (active_)
                spawnHelper("termux-wake-lock");
        }
        ~WakeLock()
        {
            if (active_)
                spawnHelper("termux-wake-unlock");
        }
        WakeLock(const WakeLock&) = delete;
        WakeLock& operator=(const WakeLock&) = delete;

    private:
        static void spawnHelper(const char* name)
        {
            pid_t pid = -1;
            char* argv[] = { const_cast<char*>(name), nullptr };

            const char* prefix = std::getenv("PREFIX");
            std::string absPath;
            if (prefix) {
                absPath = std::string(prefix) + "/bin/" + name;
                char* argvAbs[] = { absPath.data(), nullptr };
                if (posix_spawn(&pid, absPath.c_str(), nullptr, nullptr,
                        argvAbs, environ)
                        == 0
                    && pid > 0) {
                    // No esperamos con waitpid: `am startservice` puede
                    // colgarse y arrastrar a spin. El zombi se recoge al salir.
                    return;
                }
            }
            posix_spawnp(&pid, name, nullptr, nullptr, argv, environ);
        }

        bool active_;
    };

    // ------------------------------------------------------------------
    // Detección precisa de apt/pkg.
    // ------------------------------------------------------------------
    static bool isAptOrPkg(const std::string& cmd)
    {
        size_t i = cmd.find_first_not_of(" \t");
        if (i == std::string::npos)
            return false;
        std::string_view s(cmd.data() + i, cmd.size() - i);
        size_t end = s.find_first_of(" \t");
        if (end == std::string_view::npos)
            end = s.size();
        std::string_view token = s.substr(0, end);

        return token == "apt" || token == "apt-get" || token == "apt-cache" || token == "apt-mark" || token == "pkg" || token == "aptitude";
    }

    // ------------------------------------------------------------------
    // flock sobre el lock de dpkg.
    // ------------------------------------------------------------------
    class DpkgLock {
    public:
        DpkgLock()
        {
            const char* prefix = std::getenv("PREFIX");
            if (!prefix)
                return;
            std::string p = std::string(prefix) + "/var/lib/dpkg/lock";
            // Sin O_CREAT: si el lock no existe, no hacemos nada, nunca
            // creamos archivos en el árbol de dpkg.
            fd_ = ::open(p.c_str(), O_RDWR);
            if (fd_ >= 0) {
                if (::flock(fd_, LOCK_EX) != 0) {
                    ::close(fd_);
                    fd_ = -1;
                }
            }
        }
        ~DpkgLock()
        {
            if (fd_ >= 0) {
                ::flock(fd_, LOCK_UN);
                ::close(fd_);
            }
        }
        DpkgLock(const DpkgLock&) = delete;
        DpkgLock& operator=(const DpkgLock&) = delete;

    private:
        int fd_ = -1;
    };

    // ------------------------------------------------------------------
    // popen con stderr unido al pipe y limpieza de línea del spinner.
    // ------------------------------------------------------------------
    static int executeWithPopen(const std::string& command, bool quiet)
    {
        std::string finalCommand;
        if (quiet) {
            finalCommand = "(" + command + ") >/dev/null 2>&1";
        } else {
            // Unir stderr al pipe: así el error de `sh` no se pega al
            // frame del spinner, pasa por el mismo canal y lo limpiamos.
            finalCommand = "(" + command + ") 2>&1";
        }

        FILE* pipe = popen(finalCommand.c_str(), "r");
        if (!pipe) {
            LogSink::instance().write(command, -1);
            return -1;
        }

        if (!quiet) {
            char buffer[256];
            bool first = true;
            while (std::fgets(buffer, sizeof(buffer), pipe)) {
                if (first) {
                    // Borra el frame del spinner (vive en stderr) antes de
                    // imprimir la salida; stdout queda limpio para pipear.
                    std::fputs("\033[2K\r", stderr);
                    first = false;
                }
                std::fputs(buffer, stdout);
            }
            if (!first) {
                std::fflush(stdout);
                std::fflush(stderr);
            }
        }

        const int rawStatus = pclose(pipe);
        int exitCode = -1;
        if (rawStatus != -1) {
            if (WIFEXITED(rawStatus))
                exitCode = WEXITSTATUS(rawStatus);
            else if (WIFSIGNALED(rawStatus))
                exitCode = 128 + WTERMSIG(rawStatus);
        }
        LogSink::instance().write(command, exitCode);
        return exitCode;
    }

    // ------------------------------------------------------------------
    // posix_spawn para modo quiet (sin captura de salida).
    // ------------------------------------------------------------------
    static int executeWithSpawn(const std::string& command)
    {
        pid_t pid = -1;
        char* argv[] = {
            const_cast<char*>("sh"),
            const_cast<char*>("-c"),
            const_cast<char*>(command.c_str()),
            nullptr
        };

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO,
            "/dev/null", O_WRONLY, 0);
        posix_spawn_file_actions_addopen(&actions, STDERR_FILENO,
            "/dev/null", O_WRONLY, 0);

        const int rc = posix_spawnp(&pid, "sh", &actions, nullptr, argv, environ);
        posix_spawn_file_actions_destroy(&actions);

        if (rc != 0 || pid <= 0) {
            LogSink::instance().write(command, -1);
            return -1;
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            LogSink::instance().write(command, -1);
            return -1;
        }

        int exitCode = -1;
        if (WIFEXITED(status))
            exitCode = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            exitCode = 128 + WTERMSIG(status);

        LogSink::instance().write(command, exitCode);
        return exitCode;
    }

    static int dispatch(const std::string& command, bool quiet)
    {
        if (isAptOrPkg(command)) {
            DpkgLock lock;
            return executeWithPopen(command, quiet);
        }

        if (quiet)
            return executeWithSpawn(command);
        return executeWithPopen(command, quiet);
    }

public:
    static std::vector<std::string> splitCommands(const std::string& input)
    {
        std::vector<std::string> commands;
        std::stringstream ss(input);
        std::string command;
        while (std::getline(ss, command, ',')) {
            const auto first = command.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                continue;
            const auto last = command.find_last_not_of(" \t\r\n");
            commands.push_back(command.substr(first, last - first + 1));
        }
        return commands;
    }

    static CommandResult run_commands(const std::vector<std::string>& commands,
        bool quiet = false)
    {
        CommandResult result;
        result.total = static_cast<int>(commands.size());
        if (commands.empty())
            return result;

        const bool needsWakeLock = static_cast<int>(commands.size()) >= WAKE_LOCK_THRESHOLD;
        WakeLock wakeLock(needsWakeLock);

        std::atomic<int> succeeded { 0 };
        std::atomic<int> failed { 0 };

        std::mutex qMutex;
        std::deque<std::string> queue(commands.begin(), commands.end());

        const int nWorkers = std::min<int>(
            MAX_CONCURRENT, static_cast<int>(commands.size()));

        // La cola se precarga y nadie produce más items: no hace falta
        // condition_variable (con una, los workers quedaban bloqueados
        // para siempre al vaciarse la cola y join() nunca retornaba).
        auto worker = [&]() {
            for (;;) {
                std::string cmd;
                {
                    std::lock_guard<std::mutex> lk(qMutex);
                    if (queue.empty())
                        return;
                    cmd = std::move(queue.front());
                    queue.pop_front();
                }
                const int rc = dispatch(cmd, quiet);
                if (rc == 0)
                    ++succeeded;
                else
                    ++failed;
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(nWorkers);
        for (int i = 0; i < nWorkers; ++i)
            threads.emplace_back(worker);

        for (auto& t : threads)
            if (t.joinable())
                t.join();

        result.succeeded = succeeded.load();
        result.failed = failed.load();
        return result;
    }
};

#endif // SYSTERM
