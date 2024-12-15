#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include <sys/wait.h>

using namespace spinners;

int main(int argc, char *argv[]) {

  optparse::OptionParser parser = optparse::OptionParser();
  parser.add_option("-t", "--text")
      .dest("text")
      .help("Texto a mostrar ")
      .metavar("TEXT");
  parser.add_option("-i", "--interval")
      .dest("interval")
      .type("int")
      .help("Intervalo en milisegundos")
      .metavar("INT");
  parser.add_option("-s", "--symbols")
      .dest("symbols")
      .help("Simbolo de spinner")
      .metavar("STRING");
  parser.add_option("-p", "--process")
      .dest("process")
      .help("Comando a ejecutar")
      .metavar("COMMAND");

  const optparse::Values options = parser.parse_args(argc, argv);
  const std::vector<std::string> args = parser.args();

  Spinner *spinner = new Spinner();

  if (options.is_set("text")) {
    spinner->setText(options["text"]);
  }
  if (options.is_set("symbols")) {
      spinner->setSymbols("arc");
  }

  spinner->start();

  if (options.is_set("process")) {
    std::string command = options["process"];
    int pid = fork();
    if (pid == 0) { // Proceso hijo
      execl("/bin/sh", "sh", "-c", command.c_str(),
            (char *)NULL); // Ejecuta el comando
      exit(1);             // Si execl falla
    } else if (pid > 0) {  // Proceso padre
      int status;
      waitpid(pid, &status, 0); // Espera a que el proceso hijo termine.

    } else {
      perror("fork failed");
      return 1;
    }
  } else {
    sleep(5);
  }

  spinner->stop();
  delete spinner;
  return 0;
}
