#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include "include/systerm.hpp"

namespace {
void print_colors()
{
    for (int row = 0; row < 2; ++row) {
        for (int g = 0; g < 6; ++g) {
            for (int col = 0; col < 3; ++col) {
                for (int b = 0; b < 6; ++b) {
                    const int r = row * 3 + col;
                    const int i = 16 + 36 * r + 6 * g + b;
                    std::cout << "\033[38;5;" << i << 'm' << std::setw(4) << i;
                }
                std::cout << "\033[0m";
                if (col < 2)
                    std::cout << "   ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
    for (int i = 232; i < 256; ++i)
        std::cout << "\033[38;5;" << i << "m██";
    std::cout << "\033[0m\n\n";
}
void print_text_styles()
{
    std::cout << "Available text styles:\n"
              << "  - normal\n  - bold\n  - dim\n  - italic\n"
              << "  - underline\n  - reverse\n";
}
void print_positions()
{
    std::cout << "Available positions:\n"
              << "  - before (default): spinner text\n"
              << "  - after: text spinner\n";
}
}

int main(int argc, char* argv[])
{
    optparse::OptionParser parser;
    parser.version("Beta:👣v5.0");

    parser.add_option("-t", "--text").dest("text").help("Text to show").metavar("TEXT");
    parser.add_option("-i", "--interval").dest("interval").type("int").help("Interval in milliseconds").metavar("INT");
    parser.add_option("--style").dest("style").help("Built-in spinner style name").metavar("NAME");
    parser.add_option("--frames").dest("frames").help("Custom frames: \"⠋⠙⠚\" or \"one,two,three\"").metavar("FRAMES");
    parser.add_option("--position").dest("position").help("Spinner position: before or after").metavar("POSITION");
    parser.add_option("--text-style").dest("text_style").help("Text style: normal,bold,dim,italic,underline,reverse").metavar("STYLE");
    parser.add_option("--cmd").dest("cmd").help("Command to execute").metavar("COMMAND");
    parser.add_option("-c", "--color").dest("color").help("ANSI 256 color (0-255)").metavar("COLOR");
    parser.add_option("-q", "--quiet").dest("quiet").help("Suppress command output").action("store_true");

    optparse::OptionGroup result_group("Result", "Display status after completion");
    result_group.add_option("--result").dest("result").help("Display message after execution finishes").action("store_true");
    result_group.add_option("-e", "--error").dest("error").help("Custom error message").metavar("TEXT");
    result_group.add_option("-s", "--success").dest("success").help("Custom success message").metavar("TEXT");

    optparse::OptionGroup info_group("Information", "Show available options");
    info_group.add_option("--show_colors").dest("show_colors").help("Show list of 256 colors").action("store_true");
    info_group.add_option("--show_style").dest("show_style").help("Show built-in spinner styles").action("store_true");
    info_group.add_option("--show_text_styles").dest("show_text_styles").help("Show available text styles").action("store_true");
    info_group.add_option("--show_positions").dest("show_positions").help("Show spinner positions").action("store_true");

    parser.add_option_group(info_group);
    parser.add_option_group(result_group);

    const optparse::Values options = parser.parse_args(argc, argv);

    if (options.is_set("show_colors")) {
        print_colors();
        return 0;
    }
    if (options.is_set("show_style")) {
        for (const auto& name : Spinner::getAvailableSpinners())
            std::cout << "  - " << name << '\n';
        return 0;
    }
    if (options.is_set("show_text_styles")) {
        print_text_styles();
        return 0;
    }
    if (options.is_set("show_positions")) {
        print_positions();
        return 0;
    }

    Spinner spinner;

    if (argc <= 1) {
        std::string sms = "Usage: spin --cmd";
        spinner.stopWithStatus(SPIN_WARNING, sms);
        return 1;
    }

    if (options.is_set("color"))
        spinner.setColor(options["color"]);
    if (options.is_set("interval"))
        spinner.setInterval(std::stoi(options["interval"]));
    if (options.is_set("text"))
        spinner.setText(options["text"]);
    if (options.is_set("style"))
        spinner.setSymbols(options["style"]);
    if (options.is_set("frames"))
        spinner.setCustomFrames(options["frames"]);
    if (options.is_set("position"))
        spinner.setPosition(options["position"]);
    if (options.is_set("text_style"))
        spinner.setTextStyle(options["text_style"]);

    spinner.start();
    CommandResult result;

    if (options.is_set("cmd")) {
        const auto commands = SystemTermux::splitCommands(options["cmd"]);
        result = SystemTermux::run_commands(commands, options.is_set("quiet"));
    }

    if (options.is_set("result")) {
        if (result.success()) {
            const std::string msg = options.is_set("success")
                ? options["success"]
                : std::format("{} succeeded", result.succeeded);
            spinner.stopWithStatus(SPIN_SUCCESS, msg);
        } else {
            const std::string msg = options.is_set("error")
                ? options["error"]
                : std::format("{} succeeded, {} failed", result.succeeded, result.failed);
            spinner.stopWithStatus(SPIN_ERROR, msg);
        }
    } else {
        spinner.stop();
    }

    return result.success() ? 0 : 1;
}
