#include "include/optparse.hpp"
#include "include/spinners.hpp"
#include "include/systerm.hpp"
#include <algorithm>

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
void print_system()
{
    const SystemInfo info = SystemTermux::info();
    std::cout << "System:\n"
              << "  Online cores:      " << info.onlineCores << '\n'
              << "  Hardware threads:  " << info.hardwareThreads << '\n'
              << "  Process limit:     " << info.processLimit << '\n'
              << "  Safe workers:      " << info.maxConcurrentSafe << '\n';
    if (!info.coreMHz.empty()) {
        std::cout << "  Max frequency:     ";
        for (size_t i = 0; i < info.coreMHz.size(); ++i) {
            if (i != 0)
                std::cout << ' ';
            std::cout << "cpu" << i << '=' << info.coreMHz[i] << "MHz";
        }
        std::cout << '\n';
    }
}
// Entero estricto: toda la cadena debe consumirse (no admite "42abc").
bool parse_int(const std::string& raw, int& out)
{
    try {
        size_t consumed = 0;
        out = std::stoi(raw, &consumed);
        return consumed == raw.size();
    } catch (...) {
        return false;
    }
}
}

int main(int argc, char* argv[])
{
    optparse::OptionParser parser;
    parser.version("Beta:👣v5.1");

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

    optparse::OptionGroup exec_group("Execution", "Parallel command execution");
    exec_group.add_option("-j", "--jobs").dest("jobs").type("int").help("Max parallel jobs (0 = auto-detect from CPU/process limit)").metavar("N");
    exec_group.add_option("--wake-threshold").dest("wake_threshold").type("int").help("Commands in queue that take the Android wake lock (<1 disables)").metavar("N");
    exec_group.add_option("--no-wake-lock").dest("no_wake_lock").help("Never take the Android wake lock").action("store_true");
    exec_group.add_option("--pin-workers").dest("pin_workers").help("Pin each worker to its own CPU core").action("store_true");
    exec_group.add_option("--log").dest("log").help("Log file path (default: $HOME/.spin.log)").metavar("PATH");

    optparse::OptionGroup info_group("Information", "Show available options");
    info_group.add_option("--show-colors").dest("show_colors").help("Show list of 256 colors").action("store_true");
    info_group.add_option("--show-style").dest("show_style").help("Show built-in spinner styles").action("store_true");
    info_group.add_option("--show-text-styles").dest("show_text_styles").help("Show available text styles").action("store_true");
    info_group.add_option("--show-positions").dest("show_positions").help("Show spinner positions").action("store_true");
    info_group.add_option("--show-system").dest("show_system").help("Show detected CPU topology and execution limits").action("store_true");

    parser.add_option_group(exec_group);
    parser.add_option_group(info_group);
    parser.add_option_group(result_group);

    const optparse::Values options = parser.parse_args(argc, argv);

    // is_set() consulta por dest, no por el nombre del flag.
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
    if (options.is_set("show_system")) {
        print_system();
        return 0;
    }
    Spinner spinner;

    if (argc <= 1) {
        std::string sms = "Usage: spin --cmd";
        spinner.stopWithStatus(SPIN_WARNING, sms);
        return 1;
    }

    if (options.is_set("color")) {
        const std::string raw = options["color"];
        int value = -1;
        if (!parse_int(raw, value) || !Spinner::isValidColor(value)) {
            spinner.stopWithStatus(SPIN_ERROR, "Invalid color: " + raw + " (expected 0-255)");
            return 1;
        }
        spinner.setColor(value);
    }
    if (options.is_set("interval"))
        spinner.setInterval(std::stoi(options["interval"]));
    if (options.is_set("text"))
        spinner.setText(options["text"]);
    if (options.is_set("style")) {
        const std::string style = options["style"];
        const auto names = Spinner::getAvailableSpinners();
        if (std::find(names.begin(), names.end(), style) == names.end()) {
            spinner.stopWithStatus(SPIN_ERROR, "Unknown style: " + style + " (see --show-style)");
            return 1;
        }
        spinner.setSymbols(style);
    }
    if (options.is_set("frames"))
        spinner.setCustomFrames(options["frames"]);
    if (options.is_set("position")) {
        const std::string pos = options["position"];
        if (!Spinner::isValidPosition(pos)) {
            spinner.stopWithStatus(SPIN_ERROR, "Invalid position: " + pos + " (before|after)");
            return 1;
        }
        spinner.setPosition(pos);
    }
    if (options.is_set("text_style")) {
        const std::string style = options["text_style"];
        if (!Spinner::isValidTextStyle(style)) {
            spinner.stopWithStatus(SPIN_ERROR, "Invalid text style: " + style);
            return 1;
        }
        spinner.setTextStyle(style);
    }

    if (!options.is_set("cmd")) {
        spinner.stopWithStatus(SPIN_WARNING, "Usage: spin --cmd COMMAND");
        return 1;
    }

    // Configuración de ejecución delegada a SystemTermux::Options.
    SystemTermux::Options exec_opts;
    if (options.is_set("jobs")) {
        const int jobs = std::stoi(options["jobs"]);
        if (jobs < 0) {
            spinner.stopWithStatus(SPIN_ERROR, "Invalid jobs: " + std::string(options["jobs"]) + " (expected >= 0)");
            return 1;
        }
        exec_opts.setMaxConcurrent(jobs);
    }
    if (options.is_set("wake_threshold"))
        exec_opts.setWakeLockThreshold(std::stoi(options["wake_threshold"]));
    if (options.is_set("no_wake_lock"))
        exec_opts.setAllowWakeLock(false);
    if (options.is_set("pin_workers"))
        exec_opts.setPinWorkers(true);
    if (options.is_set("log"))
        exec_opts.setLogPath(options["log"]);

    CommandResult result;
    {
        const auto commands = SystemTermux::splitCommands(options["cmd"]);
        SystemTermux system(exec_opts);

        // Arrancar SOLO aquí, con trabajo real.
        spinner.start();
        result = system.run_commands(commands, options.is_set("quiet"));
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
