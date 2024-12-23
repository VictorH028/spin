#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include <sys/wait.h>

using namespace spinners;

void print_map(const std::map<const std::string, const std::string> &m) {
  for (const auto &[key, value] : m) {
    std::cout << key << " <---> " << value << std::endl;
  }
}

int main(int argc, char *argv[]) {

  optparse::OptionParser parser = optparse::OptionParser();
  parser.add_option("-t", "--text")
      .dest("text")
      .help("Text to show ")
      .metavar("TEXT");
  parser.add_option("-i", "--interval")
      .dest("interval")
      .type("int")
      .help("Interval in milliseconds")
      .metavar("INT");
  parser.add_option("-s", "--symbols")
      .dest("symbols")
      .help("Symbols the spinner")
      .metavar("STRING");
  parser.add_option("-p", "--process")
      .dest("process")
      .help("Command to execute")
      .metavar("COMMAND");
  parser.add_option("-l", "--list_symbols")
      .dest("list")
      .help("List of symbols")
      .action("store_false");
  parser.add_option("-c", "--color")
      .dest("color")
      .help("Change text color")
      .metavar("COLOR");

  const optparse::Values options = parser.parse_args(argc, argv);
  const std::vector<std::string> args = parser.args();

  // Una propiedad unica
  std::unique_ptr<Spinner> spinner = std::make_unique<Spinner>();

  if (options.is_set("color")) {
    spinner->setColor(options["color"]);
  }
  if (options.is_set("interval")) {
    spinner->setInterval(std::stoi(options["interval"]));
  }
  if (options.is_set("text")) {
    spinner->setText(options["text"]);
  }
  if (options.is_set("symbols")) {
    spinner->setSymbols(options["symbols"]);
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
      exit(1);
    } else if (pid > 0) { // Proceso padre
      int status;
      waitpid(pid, &status, 0); // Espera a que el proceso hijo termine.

    } else {
      perror("fork failed");
      return 1;
    }
  } else {
    sleep(5);
  }

  if (spinner) {
    spinner->stop();
    spinner.reset(); // Libera la memoria
  }
  return 0;
}
