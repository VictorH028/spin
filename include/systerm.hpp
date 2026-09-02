/**
 *  @file systerm.cpp
 *  @brief Ejecutar comandos en Termux con concurrencia segura.
 */

#ifndef SYSTERM
#define SYSTERM

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
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

    bool success() const
    {
        return failed == 0;
    }
};

/**
 *  @brief Utilidades para ejecución de comandos en Termux
 */
class SystemTermux {
private:
    inline static std::mutex logMutex;

    /**
     * @brief Guarda  logs.
     */
    static std::string getLogFilePath()
    {
        const char* homeDir = std::getenv("HOME");
        if (homeDir) {
            return std::string(homeDir) + "/.spin.log";
        }
        return "spin.log"; // Fallback local si no existe $HOME
    }

    /**
     * @brief Guarda información de los comandos ejecutados.
     */
    static void saveLog(const std::string& command, int exitCode)
    {
        std::lock_guard<std::mutex> lock(logMutex);

        std::ofstream logFile(getLogFilePath(), std::ios::app);
        if (!logFile.is_open())
            return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr.back() == '\n') {
            timeStr.pop_back();
        }

        logFile << "========================================\n";
        logFile << "Fecha: " << timeStr << "\n";
        logFile << "Comando: " << command << "\n";
        logFile << "Exit code: " << exitCode << "\n";
        logFile << "Estado: " << (exitCode == 0 ? "SUCCESS" : "FAILED") << "\n";
    }

    /**
     *  0 : Éxito
     * >0 : Código de error devuelto por la shell
     * -1 : Error al invocar popen
     */
    static int executeCommand(const std::string& command, bool quiet)
    {
        std::string finalCommand = command;
        if (quiet) {
            finalCommand += " > /dev/null 2>&1";
        }

        FILE* pipe = popen(finalCommand.c_str(), "r");
        if (!pipe) {
            if (quiet) {
                saveLog(finalCommand, -1);
            }
            return -1;
        }

        int rawStatus = pclose(pipe);
        if (rawStatus == -1) {
            if (quiet) {
                saveLog(finalCommand, -1);
            }
            return -1;
        }

        int exitCode = -1;
        if (WIFEXITED(rawStatus)) {
            exitCode = WEXITSTATUS(rawStatus);
        } else if (WIFSIGNALED(rawStatus)) {
            exitCode = 128 + WTERMSIG(rawStatus);
        }

        if (quiet) {
            saveLog(finalCommand, exitCode);
        }

        return exitCode;
    }

public:
    static std::vector<std::string> splitCommands(const std::string& input)
    {
        std::vector<std::string> commands;
        std::stringstream ss(input);
        std::string command;
        while (std::getline(ss, command, ',')) {
            command.erase(0, command.find_first_not_of(' '));
            command.erase(command.find_last_not_of(' ') + 1);
            if (!command.empty()) {
                commands.push_back(command);
            }
        }
        return commands;
    }

    static CommandResult run_commands(const std::vector<std::string>& commands, bool quiet = false)
    {
        std::mutex aptMutex;
        std::vector<std::thread> threads;

        std::atomic<int> succeeded { 0 };
        std::atomic<int> failed { 0 };

        CommandResult result;
        result.total = static_cast<int>(commands.size());

        for (const auto& command : commands) {
            // Detectar gestores de paquetes que bloquean la base de datos de dpkg
            bool isAptOrPkg = (command.find("apt") != std::string::npos || command.find("pkg") != std::string::npos);

            // Capturar 'command' POR VALOR para evitar dangling reference
            threads.emplace_back([command, quiet, isAptOrPkg, &aptMutex, &succeeded, &failed]() {
                int res = -1;

                if (isAptOrPkg) {
                    std::lock_guard<std::mutex> lock(aptMutex);
                    res = executeCommand(command, quiet);
                } else {
                    res = executeCommand(command, quiet);
                }

                if (res == 0) {
                    ++succeeded;
                } else {
                    ++failed;
                }
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        result.succeeded = succeeded.load();
        result.failed = failed.load();

        return result;
    }
};

#endif // !SYSTERM
