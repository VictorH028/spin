#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include "include/systerm.hpp"
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
        .metavar("NAME");
    parser.add_option("-p", "--process")
        .dest("process")
        .help("Command to execute")
        .metavar("COMMAND");
    parser.add_option("-l", "--list_symbols")
        .dest("list")
        .help("List of symbols")
        .action("store_false");
    parser.add_option("-s","--style")
        .dest("style")
        .help("Show style")
        .action("store_false");
    parser.add_option("-c", "--color")
        .dest("color")
        .help("Change text color")
        .metavar("COLOR");
    parser.add_option("--list_colors")
        .dest("list_colors")
        .help("Show list colors")
        .action("store_true");
    parser.add_option("--procs")
        .dest("procs")
        .help("Number of current processes")
        .action("store_true");
    parser.add_option("-q", "--quiet")
        .dest("quiet")
        .help("Run quietly, suppressing output")
        .action("store_true");

    const optparse::Values options = parser.parse_args(argc, argv);
    const std::vector<std::string> args = parser.args();

    Spinner spinner;

    if (options.is_set("version")) {
        cout << "Beta:👣v4.5   " << endl;
    }
    if (options.is_set("color")) {
        spinner.setColor(FOREGROUND_COLOR + options["color"] + "m" );
    }
    if (options.is_set("interval")) {
        spinner.setInterval(std::stoi(options["interval"]));
    }
    if (options.is_set("text")) {
        spinner.setText(options["text"]);
    }
    if (options.is_set("symbols")) {
        spinner.setSymbols(options["symbols"]);
    }
    if (options.is_set("style")) {
        /*spinner.setStyle(options["style"]);*/
    }
    if (options.is_set("list")) {
        /*spinner->showSymbols();*/
        return 0;
    }
    if (options.is_set("list_colors")) {
        print_colors();
        return 0;
    }


    spinner.start();

    if (options.is_set("process")) {
        std::vector<string> commands = SystemTermux::splitCommands(options["process"]);
        SystemTermux::run_commands(commands, options.is_set("quiet"));
    } 

    spinner.stop();
    return 0;
}
