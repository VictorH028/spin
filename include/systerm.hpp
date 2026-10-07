#ifndef SYSTERM
#define SYSTERM

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <vector>

struct CommandResult {
    int total = 0;
    int succeeded = 0;
    int failed = 0;
    bool success() const { return failed == 0; }
};

class SystemTermux {
private:
    inline static std::mutex logMutex;

    static std::string getLogFilePath(const std::string& name) {
        const char* homeDir = std::getenv("HOME");
        return homeDir ? std::string(homeDir) + "/" + name : name;
    }

    static void saveLog(const std::string& command, int exitCode) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::ofstream logFile(getLogFilePath(".spin.log"), std::ios::app);
        if (!logFile.is_open()) return;

        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr.back() == '\n') timeStr.pop_back();

        logFile << "========================================\n"
                << "Fecha: " << timeStr << "\n"
                << "Comando: " << command << "\n"
                << "Exit code: " << exitCode << "\n"
                << "Estado: " << (exitCode == 0 ? "SUCCESS" : "FAILED") << "\n";
    }

    static int executeCommand(const std::string& command, bool quiet) {
        std::string finalCommand = command;
        if (quiet) finalCommand += " > /dev/null 2>&1";

        FILE* pipe = popen(finalCommand.c_str(), "r");
        if (!pipe) {
            saveLog(finalCommand, -1);
            return -1;
        }

        char buffer[256];
        if (!quiet) {
            while (fgets(buffer, sizeof(buffer), pipe))
                std::fputs(buffer, stdout);
        }

        const int rawStatus = pclose(pipe);
        if (rawStatus == -1) {
            saveLog(finalCommand, -1);
            return -1;
        }

        int exitCode = -1;
        if (WIFEXITED(rawStatus)) exitCode = WEXITSTATUS(rawStatus);
        else if (WIFSIGNALED(rawStatus)) exitCode = 128 + WTERMSIG(rawStatus);

        saveLog(finalCommand, exitCode);
        return exitCode;
    }

public:
    static std::vector<std::string> splitCommands(const std::string& input) {
        std::vector<std::string> commands;
        std::stringstream ss(input);
        std::string command;

        while (std::getline(ss, command, ',')) {
            const auto first = command.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            const auto last = command.find_last_not_of(" \t\r\n");
            commands.push_back(command.substr(first, last - first + 1));
        }
        return commands;
    }

    static CommandResult run_commands(const std::vector<std::string>& commands, bool quiet = false) {
        std::mutex aptMutex;
        std::vector<std::thread> threads;
        std::atomic<int> succeeded{0};
        std::atomic<int> failed{0};

        CommandResult result;
        result.total = static_cast<int>(commands.size());

        for (const auto& command : commands) {
            const bool isAptOrPkg =
                command.find("apt") != std::string::npos ||
                command.find("pkg") != std::string::npos;

            threads.emplace_back([command, quiet, isAptOrPkg,
                                  &aptMutex, &succeeded, &failed]() {
                int res;
                if (isAptOrPkg) {
                    std::lock_guard<std::mutex> lock(aptMutex);
                    res = executeCommand(command, quiet);
                } else {
                    res = executeCommand(command, quiet);
                }
                if (res == 0) ++succeeded;
                else ++failed;
            });
        }

        for (auto& thread : threads)
            if (thread.joinable()) thread.join();

        result.succeeded = succeeded.load();
        result.failed = failed.load();
        return result;
    }
};

#endif // SYSTERM
