#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include <iterator>
#include <memory>
#include <sys/wait.h>

using namespace spinners;

void print_map(const std::map<const char *, const char *> &m){
    for (const auto& [key , value]: m) {
       std::cout << key << " <---> " << value << std::endl; 
    }
}

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
  parser.add_option("-l", "--list_symbols")
      .dest("list")
      .help("Lista de simbolos ")
      .action("store_false");

  const optparse::Values options = parser.parse_args(argc, argv);
  const std::vector<std::string> args = parser.args();

  // Una propiedad unica   
  std::unique_ptr<Spinner> spinner = std::make_unique<Spinner>();

  if (options.is_set("interval")) {
    spinner->setInterval(std::stoi(options["interval"]));
  }
  if (options.is_set("text")) {
    spinner->setText(options["text"]);
  }
  if (options.is_set("symbols")) {
      spinner->setSymbols(options["symbols"].c_str());
  }
  if (options.is_set("list")) {
      print_map(spinnerType); 
      return 0;
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
  return 0;
}
