/**
 *  @file systerm.cpp
 *  @brief Ejecutar comamdos en termux.
 */

#ifndef SYSTERM
#define SYSTERM

#include <iostream>
#include <thread>

using namespace std;

/** 
 *  @brief Algunas funcionalidades para termux
 *
 *  @class 
 */
class SystemTermux { 
    int static executeCommand(const string& command, bool quiet)
    {
        std::string adjustedCommand = command;
        if (quiet) {
            adjustedCommand += " > /dev/null 2>&1";
        }
        int result = system(adjustedCommand.c_str());
        if (result != 0) {
            cerr << "Error executing  " << command << endl;
            return 1;
        }
        return 0;
    }

public:
    /**
     *  @param  string comando_nombre
     *  @param  bool salida del comando  
     */
    int static run_command(const std::string& command, bool quiet = false)
    {
        std::thread t(executeCommand, command, quiet);
        t.join(); /**< Espera a que termine el hilo */
        return 0;
    }
}; // Find  SystemTermux

#endif // !SYSTERM
#define SYSTERM
