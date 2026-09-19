#include "../include/spinners.hpp"

#include <array>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <thread>

Spinner::Spinner()
{
    symbols = std::make_unique<std::string>(spinnerType[0].second);
    start_time = std::chrono::steady_clock::now();
    setupSignalHandlers();
}

Spinner::~Spinner()
{
    stop();
    hideCursor(false);
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
        hideCursor(true); // Ocultar cursor al iniciar
        start_time = std::chrono::steady_clock::now();

        // Si usas customFrames o std::vector<std::string>, es más seguro iterar frames completos:
        while (!stop_spinner) {
            std::string frame = symbols->substr(i, 3); // Ojo si usas caracteres de 1 o 4 bytes
            i = (i + 3) % symbols->size();

            // \033[2K limpia toda la línea actual antes de reescribir
            std::cerr << "\033[2K\r"
                      << FOREGROUND_COLOR << color << "m"
                      << frame << " " << text << "\033[0m"
                      << std::flush;

            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

        std::cerr << "\033[2K\r" << std::flush;
        hideCursor(false); // Mostrar cursor al detener el loop
    });
}

void Spinner::stop()
{
    stop_spinner = true;
    if (t.joinable()) {
        t.join();
    }

    hideCursor(false);
}

std::chrono::milliseconds Spinner::getElapsedTime() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
}

Spinner& Spinner::setCustomFrames(const std::vector<std::string>& frames)
{
    symbols = std::make_unique<std::string>();
    for (const auto& frame : frames) {
        *symbols += frame;
    }
    return *this;
}

std::vector<std::string> Spinner::getAvailableSpinners()
{
    std::vector<std::string> names;
    for (const auto& [name, _] : spinnerType) {
        names.push_back(name);
    }
    return names;
}

Spinner& Spinner::setColor(const std::string& _color)
{
    color = _color;
    return *this;
}

void Spinner::showStatus(SpinResult type, const std::string& text)
{
    const auto& status = defaultStatus[type];

    std::cout
        << FOREGROUND_COLOR << color << "m"
        << status.icon << " "
        << (text.empty() ? status.text : text)
        << "\033[0m"
        << std::endl;
}

// Función estática/global para manejar la interrupción brusca
void Spinner::handleSignal(int signal)
{
    // Restaurar el cursor inmediatamente antes de terminar el proceso
    std::cout << "\033[?25h\033[0m\n"
              << std::flush;
    std::exit(signal);
}

void Spinner::setupSignalHandlers()
{
    std::signal(SIGINT, Spinner::handleSignal); // Interrupción de consola (Ctrl + C)
    std::signal(SIGTERM, Spinner::handleSignal); // Solicitud de terminación
}
