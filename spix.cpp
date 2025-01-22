#include "include/optparse.hpp"
#include "include/spinners.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

using namespace spinners;

void print_map(const std::map<const string, const string> &m) {
  for (const auto &[key, value] : m) {
    cout << key << " <---> " << value << endl;
  }
}

class SystemTermux {
  int static executeCommand(const string &command, bool quiet) {
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
  int static run_command(const std::string &command, bool quiet) {
    std::thread t(executeCommand, command, quiet);
    t.join(); // Espera a que termine el hilo
    return 0;
  }
};

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
  parser.add_option("-q", "--quiet")
      .dest("quiet")
      .help("Run quietly, suppressing output")
      .action("store_true");

  const optparse::Values options = parser.parse_args(argc, argv);
  const std::vector<std::string> args = parser.args();

  // Una propiedad unica
  std::unique_ptr<Spinner> spinner = std::make_unique<Spinner>();
  Spinner::setupSignalHandlers();

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
    int result = SystemTermux::run_command(options["process"], options.is_set("quiet"));
    if (result != 0) {
      return 1;
    }
  } else {
    sleep(5);
  }

  if (spinner) {
    spinner->stop();
    spinner.reset();
  }
  return 0;
}
