/**
 *  @file systerm.cpp
 *  @brief Ejecutar comamdos en termux.
 */

#ifndef SYSTERM
#define SYSTERM

#include <iostream>
#include <string>
#include <sys/sysinfo.h>
#include <thread>
#include <vector>

using namespace std;

/**
 *  @brief Algunas funcionalidades para termux
 *
 *  @class
 */
class SystemTermux {

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

    static void run_commands(std::vector<std::string>& commands, bool quiet = false)
    {

        std::mutex aptMutex; // Apt da error por eso lo sincroniszo
        std::vector<std::thread> threads;

        for (auto& command : commands) {
            if (command.find("apt") != string::npos && command.find("pkg") != string::npos) {
                // Ejecuta `apt` y `pkg` en serie usando el mutex
                std::lock_guard<std::mutex> lock(aptMutex);
                executeCommand(command, quiet);
            } else {
                // Ejecuta otros comandos en hilos separados (paralelo)
                threads.emplace_back(executeCommand, std::ref(command), quiet);
            }
        }

        // Espera a que todos los hilos terminen
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
}; // Find  SystemTermux

#endif // !SYSTERM
