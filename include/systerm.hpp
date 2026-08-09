/**
 *  @file systerm.cpp
 *  @brief Ejecutar comamdos en termux.
 */

#ifndef SYSTERM
#define SYSTERM

#include <string>
#include <sys/sysinfo.h>
#include <thread>
#include <vector>

using namespace std;

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
 *  @brief Algunas funcionalidades para termux
 *
 *  @class
 */
class SystemTermux {
   // static int executeCommand(string& command, bool quiut, bool log)     
    static int executeCommand(string& command, bool quiet)
    {
        if (quiet) {
            command += " > /dev/null 2>&1";
        }
        int result = system(command.c_str());
        if (result != 0) {
            // cerr << "Error executing  " << command << endl;
            return 1;
        }
        return 0;
    }

public:
    static std::vector<std::string> splitCommands(const std::string input)
    {
        std::vector<std::string> commands;
        stringstream ss(input);
        std::string command;
        while (getline(ss, command, ',')) {
            // Elimina espacios en blanco alrededor del comando
            command.erase(0, command.find_first_not_of(' '));
            command.erase(command.find_last_not_of(' ') + 1);
            if (!command.empty()) {
                commands.push_back(command);
            }
        }

        return commands;
    }

    static CommandResult run_commands(std::vector<std::string>& commands,
        bool quiet = false)
    {
        std::mutex aptMutex;
        std::vector<std::thread> threads;

        std::atomic<int> succeeded { 0 };
        std::atomic<int> failed { 0 };

        CommandResult result;
        result.total = commands.size();

        for (auto& command : commands) {

            if (command.find("apt") != std::string::npos || command.find("pkg") != std::string::npos) {

                std::lock_guard<std::mutex> lock(aptMutex);

                if (executeCommand(command, quiet) == 0)
                    ++succeeded;
                else
                    ++failed;

            } else {

                threads.emplace_back([&command, quiet, &succeeded, &failed]() {
                    if (executeCommand(command, quiet) == 0)
                        ++succeeded;
                    else
                        ++failed;
                });
            }
        }

        for (auto& t : threads) {
            if (t.joinable())
                t.join();
        }

        result.succeeded = succeeded.load();
        result.failed = failed.load();

        return result;
    }
}; // Find  SystemTermux

#endif // !SYSTERM
