#include "../include/spinners.hpp"

#include <array>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <thread>

/**
 * @brief array que contiene los diferentes tipos de spinners disponibles.
 */
const std::array<std::pair<std::string, std::string>, 42> spinnerType = { {
    { "dots", "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏" },
    { "dots2", "⣾⣽⣻⢿⡿⣟⣯⣷" },
    { "dots3", "⠋⠙⠚⠞⠖⠦⠴⠲⠳⠓" },
    { "dots4", "⠄⠆⠇⠋⠙⠸⠰⠠⠰⠸⠙⠋⠇⠆" },
    { "dots5", "⠋⠙⠚⠒⠂⠂⠒⠲⠴⠦⠖⠒⠐⠐⠒⠓⠋" },
    { "dots6", "⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠴⠲⠒⠂⠂⠒⠚⠙⠉⠁" },
    { "dots7", "⠈⠉⠋⠓⠒⠐⠐⠒⠖⠦⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈" },
    { "dots8", "⠁⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈⠈" },
    { "dots9", "⢹⢺⢼⣸⣇⡧⡗⡏" },
    { "dots10", "⢄⢂⢁⡁⡈⡐⡠" },
    { "dots11", "⠁⠂⠄⡀⢀⠠⠐⠈" },
    { "pipe", "┤┘┴└├┌┬┐" },
    { "star", "✶✸✹✺✹✷" },
    { "flip", "___-`'´-___" },
    { "hamburger", "☱☲☴" },
    { "growVertical", "▁▃▄▅▆▇▆▅▄▃" },
    { "growHorizontal", "▏▎▍▌▋▊▉▊▋▌▍▎" },
    { "balloon", " . o O @ * " },
    { "balloon2", ".oO°Oo." },
    { "noise", "▓▒░" },
    { "bounce", "⠁⠂⠄⠂" },
    { "boxBounce", "▖▘▝▗" },
    { "boxBounce2", "▌▀▐▄" },
    { "triangle", "◢◣◤◥" },
    { "arc", "◜◠◝◞◡◟" },
    { "circle", "◡⊙◠" },
    { "squareCorners", "◰◳◲◱" },
    { "circleQuarters", "◴◷◶◵" },
    { "circleHalves", "◐◓◑◒" },
    { "squish", "╫╪" },
    { "toggle", "⊶⊷" },
    { "toggle5", "▮▯" },
    { "toggle6", "ဝ၀" },
    { "toggle7", "⦾⦿" },
    { "toggle8", "◍◌" },
    { "toggle9", "◉◎" },
    { "toggle10", "㊂㊀㊁" },
    { "toggle11", "⧇⧆" },
    { "toggle12", "☗☖" },
    { "arrow", "←↖↑↗→↘↓↙" },
    { "arrow2", "➞➟➠➡➠➟" },
    { "triangle", "⬖⬘⬗⬙" },
} };

Spinner::Spinner()
{
    symbols = std::make_unique<std::string>(spinnerType[0].second);
    start_time = std::chrono::steady_clock::now();
}

Spinner::~Spinner()
{
    stop();
}

void Spinner::hideCursor(bool hide)
{
    std::cout << (hide ? HIDE_CURSOR : SHOW_CURSOR) << std::flush;
}

Spinner& Spinner::setInterval(int ms)
{
    interval = ms;
    return *this;
}

Spinner& Spinner::setText(const std::string& txt)
{
    text = txt;
    return *this;
}

Spinner& Spinner::setSymbols(const std::string& key)
{
    auto it = std::find_if(spinnerType.begin(), spinnerType.end(),
        [&key](const auto& pair) { return pair.first == key; });
    symbols = std::make_unique<std::string>(it != spinnerType.end() ? it->second : spinnerType[0].second);
    return *this;
}

void Spinner::start()
{
    stop_spinner = false;
    t = std::thread([this]() {
        size_t i = 0;
        hideCursor();
        start_time = std::chrono::steady_clock::now();

        while (!stop_spinner) {
            std::string frame = symbols->substr(i, 3); // UTF-8 characters
            i = (i + 3) % symbols->size(); // 3 bite por char

            std::cout << std::format("{} {} {}\r", frame, color, text) << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

        hideCursor(false);
    });
}

void Spinner::stop()
{
    stop_spinner = true;
    if (t.joinable()) {
        t.join();
    }
   hideCursor();
}

std::chrono::milliseconds Spinner::getElapsedTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
}

Spinner& Spinner::setCustomFrames(const std::vector<std::string>& frames) {
    symbols = std::make_unique<std::string>();
    for (const auto& frame : frames) {
        *symbols += frame;
    }
    return *this;
}

std::vector<std::string> Spinner::getAvailableSpinners() {
    std::vector<std::string> names;
    for (const auto& [name, _] : spinnerType) {
        names.push_back(name);
    }
    return names;
}


Spinner& Spinner::setColor(const std::string& _color) {
    color = _color;
    return *this;
}
void Spinner::showSymbols()
{
    for (const auto& [key, value] : spinnerType) {
        std::cout << key << " <---> " << value << std::endl;
    }
}

/**/
/*void Spinner::handleSignal(int signal) {*/
/*    if (signal == SIGINT || signal == SIGTERM) {*/
/*        std::cout << SHOW_CURSOR << "\nSignal received. Cleaning up...\n";*/
/*        std::exit(signal);*/
/*    }*/
/*}*/
/**/

/*void Spinner::setupSignalHandlers()*/
/*{*/
/*    std::signal(SIGINT, handleSignal);*/
/*    std::signal(SIGTERM, handleSignal);*/
/*}*/
/**/
