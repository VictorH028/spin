/**
 * @file spinners.hpp
 * @brief Librería para mostrar spinners animados en terminales.
 */

#ifndef _SPINNERS_HPP_
#define _SPINNERS_HPP_

#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#define SHOW_CURSOR "\u001b[?25h"
#define HIDE_CURSOR "\u001b[?25l"

#define FOREGROUND_COLOR "\x1b[38;5;" // ID m
#define BACKGROUND_COLOR "\x1b[48;5;" // ID m

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

/**
 * @class Spinner
 * @brief Clase para mostrar animaciones de spinner en la terminal.
 */
class Spinner {
private:
    int interval = 100; /**< Intervalo entre frames (ms) */
    std::string text; /**< Texto acompañante */
    std::unique_ptr<std::string> symbols; /**< Símbolos del spinner */
    std::atomic<bool> stop_spinner { false }; /**< Control de ejecución */
    std::string color; /**< Color del spinner */
    std::thread t; /**< Hilo de animación */
    std::chrono::time_point<std::chrono::steady_clock> start_time; /**< Tiempo de inicio */

    void hideCursor(bool hide = true);

public:
    Spinner();
    ~Spinner();

    // Control básico
    void start();
    void stop();

    // Configuración
    Spinner& setInterval(int); // ms
    Spinner& setText(const std::string&); // text
    Spinner& setSymbols(const std::string&); // key
    Spinner& setColor(const std::string&); // color
    Spinner& setBackgroundColor(const std::string&); // color
    Spinner& setStyle(const std::string&); // style
    Spinner& setCustomFrames(const std::vector<std::string>&); // frames
    Spinner& setPosition(int, int);
    /*Spinner& suffixText();*/

    // Consulta
    std::string getColor() const;
    bool isRunning() const;
    std::chrono::milliseconds getElapsedTime() const;
    void showSymbols();
    static std::vector<std::string> getAvailableSpinners();

    // Utilidades
    static void handleSignal(int signal);
};

#endif // _SPINNERS_HPP_
