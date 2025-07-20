#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include "include/systerm.hpp"
#include <csignal>
#include <iomanip>

void print_colors()
{
    /*for (int i = 0; i < 16; i++) {*/
    /*    cout << "\x1b[38;5;" << i << 'm' << std::setw(4) << i;*/
    /*    if (i % 8 == 7) cout << "\x1b[0;0m\n";*/
    /*}*/
    /*cout << '\n';*/
    /**/
    for (int row = 0; row < 2; row++) {
        for (int g = 0; g < 6; g++) {
            for (int col = 0; col < 3; col++) {
                for (int b = 0; b < 6; b++) {
                    int r = row * 3 + col;
                    int i = 16 + 36 * r + 6 * g + b;
                    cout << "\x1b[38;5;" << i << 'm' << std::setw(4) << i;
                }
                cout << "\x1b[0;0m";
                if (col < 2)
                    cout << "   ";
            }
            cout << '\n';
        }
        cout << '\n';
    }

    for (int i = 232; i < 256; i++) {
        cout << "\x1b[38;5;" << i << 'm'; // << std::setw(4) << i;
        if (i == 243)
            cout << "\x1b[0;0m\n";
    }
    cout << "\x1b[0;0m\n\n";
}

int main(int argc, char* argv[])
{
    optparse::OptionParser parser = optparse::OptionParser();
    parser.version("Beta:👣v4.5");

    parser.add_option("-t", "--text")
        .dest("text")
        .help("Text to show ")
        .metavar("TEXT");
    parser.add_option("-i", "--interval")
        .dest("interval")
        .type("int")
        .help("Interval in milliseconds")
        .metavar("INT");
    parser.add_option("-s", "--style")
        .dest("style")
        .help("Style the spinner")
        .metavar("NAME");
    parser.add_option("--cmd")
        .dest("cmd")
        .help("Command to execute")
        .metavar("COMMAND");

    /*parser.add_option("--custom_frame")*/
    /*    .dest("custom")*/
    /*    .type("std::vector<std::string>")*/
    /*    .help("....")*/
    /*    .action("store_false");*/
    parser.add_option("-c", "--color")
        .dest("color")
        .help("Change text color")
        .metavar("COLOR");
    parser.add_option("-q", "--quiet")
        .dest("quiet")
        .help("Run quietly, suppressing output")
        .action("store_true");

    optparse::OptionGroup group = optparse::OptionGroup("Information",
        "To show information");
    group.add_option("--show_colors")
        .dest("show_colors")
        .help("Show list colors")
        .action("store_true");
    group.add_option("--show_style")
        .dest("show_style")
        .help("Show list the symbols")
        .action("store_true");
    // group.add_option("-l", "--list_symbols")
    //     .dest("list")
    //     .help("List of symbols")
    //     .action("store_false");
    //
    parser.add_option_group(group);
    const optparse::Values options = parser.parse_args(argc, argv);
    const std::vector<std::string> args = parser.args();

    Spinner spinner;

    if (options.is_set("color")) {
        spinner.setColor(options["color"]);
    }
    if (options.is_set("interval")) {
        spinner.setInterval(std::stoi(options["interval"]));
    }
    if (options.is_set("text")) {
        spinner.setText(options["text"]);
    }
    if (options.is_set("style")) {
        spinner.setSymbols(options["style"]);
    }
    /*if (options.is_set("custom")) {*/
    /*    spinner.setCustomFrames(options["custom"]);*/
    /*}*/
    if (options.is_set("show_style")) {
        for (const auto& [key, value] : spinnerType) {
            cout << key << " <----> " << value << "\n";
        }
        return 0;
    }
    if (options.is_set("show_colors")) {
        print_colors();
        return 0;
    }

    spinner.start();
    if (options.is_set("cmd")) {
        std::vector<string> commands = SystemTermux::splitCommands(options["cmd"]);
        SystemTermux::run_commands(commands, options.is_set("quiet"));
    }

    spinner.stop();
    return 0;
}
