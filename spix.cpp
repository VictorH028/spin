#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include "include/systerm.hpp"


using namespace spinners;

/**
 *
 */
template <typename Container>
void print_map(Container &m)
{
    for (const auto& [key, value] : m) {
        cout << key << " <---> " << value << endl;
    }
}

int main(int argc, char* argv[])
{
    optparse::OptionParser parser = optparse::OptionParser();
    parser.add_option("-v", "--version")
        .dest("version")
        .action("store_true")
        .help("Show version");
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

    if (options.is_set("version")) {
      cout << "Beta:👣v0.2   " << endl;
    }
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
        SystemTermux::run_command(options["process"], options.is_set("quiet"));
    } else {
        sleep(5);
    }

    if (spinner) {
        spinner->stop();
        spinner.reset();
    }
    return 0;
}
