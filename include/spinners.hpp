/**
 * @file spinners.hpp
 * @brief Librería para mostrar spinners animados.
 */

#ifndef _SPINNERS_HPP_
#define _SPINNERS_HPP_

#include <chrono>
#include <csignal>
#include <cstring>
#include <format>
#include <iostream>
#include <thread>

#define SHOW_CURSOL "\u001b[?25h"
#define HIDE_CURSOL "\u001b[?25l"

// foreground color   --- ID m 
#define FOREGROUND_COLOR "\x1b[38;5;"
// background color. --- ID m  
#define BACkGROUND_COLOR "\x1b[48;5;" 


namespace spinners {
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
 * @brief Clase que representa un spinner animado.
 */
class Spinner {
public:
    /**
     * @brief Constructor por defecto.
     */
    Spinner()
        : interval(80)
        , text("")
        , symbols(std::make_unique<std::string>(getSpinner("circleHalves")))
        , stop_spinner(false)
        , color(setColor("115"))
    {
        setupSignalHandlers();
    }

    /**
     * @brief Constructor con parámetros.
     *
     * @param _interval Intervalo de tiempo entre cada frame del spinner.
     * @param _text Texto que se muestra junto al spinner.
     * @param _symbols Tipo de spinner a utilizar.
     * @param color_str Color del spinner.
     */
    Spinner(int _interval,
        const std::string& _text,
        const std::string& _symbols,
        const std::string _color)
        : interval(_interval)
        , text(_text)
        , symbols(std::make_unique<std::string>(getSpinner(_symbols)))
        , stop_spinner(false)
        , color(setColor(_color))
        
    {
        setupSignalHandlers();
    }

    /**
     * @brief Devuelve el tipo de spinner solicitado.
     *
     * @param key Nombre del spinner.
     * @return const std::string& La cadena de caracteres que representa el spinner.
     */
    const std::string& getSpinner(const std::string& key)
    {
        auto it = std::find_if(spinnerType.begin(), spinnerType.end(),
            [&key](const auto& pair) { return pair.first == key; });
        if (it != spinnerType.end()) {
            return it->second;
        }
        return spinnerType[0].second;
    }

    /**
     * @brief Destructor.
     */
    ~Spinner() { stop(); }

    /**
     * @brief Establece el intervalo de tiempo entre cada frame del spinner.
     *
     * @param _interval El intervalo de tiempo en milisegundos.
     */
    void setInterval(int _interval) { interval = _interval; }

    /**
     * @brief Establece el texto que se muestra junto al spinner.
     *
     * @param _text El texto a mostrar.
     */
    void setText(const std::string& _text) { text = _text; }

    /**
     * @brief Establece el tipo de spinner a utilizar.
     *
     * @param _symbols El nombre del tipo de spinner.
     */
    void setSymbols(const std::string& _symbols)
    {
        symbols = std::make_unique<std::string>(getSpinner(_symbols));
    }

    /**
     * @brief Establece el color del spinner.
     *
     * @param 
     */
    std:: string setColor(std::string _color)
    {
       return FOREGROUND_COLOR + _color + "m";
    }

    /**
     * @brief Inicia la animación del spinner.
     */
    void startSpinner()
    {
        const size_t len = symbols->size();
        size_t i = 0;
        hideCursor();
        try {
            while (!stop_spinner.load()) {
                // Obtener un carácter basado en UTF-8
                std::string utf8_char = symbols->substr(i, 3); // 3 bytes por símbolo
                i = (i + 3) % len;
                std::cout << std::format("{} {} {} \r", utf8_char, color , text ) << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        } catch (...) {
            hideCursor(false);
            throw;
        }
        hideCursor(false);
    }

    /**
     * @brief Inicia el spinner en un hilo separado.
     */
    void start() { t = std::thread(&Spinner::startSpinner, this); }

    /**
     * @brief Detiene la animación del spinner.
     */
    void stop()
    {
        stop_spinner.store(true);
        if (t.joinable()) {
            try {
                t.join();
            } catch (...) {
                hideCursor(false);
                throw;
            }
        }
        hideCursor(false);
    }

    /**
     * @brief Configura los manejadores de señales para una salida limpia.
     */
    void setupSignalHandlers()
    {
        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);
    }

private:
    int interval; /**< Intervalo de tiempo entre cada frame del spinner. */
    std::string text; /**< Texto que se muestra junto al spinner. */
    std::unique_ptr<std::string> symbols; /**< Símbolos que representan el spinner. */
    std::atomic<bool> stop_spinner; /**< Bandera para detener el spinner. */
    std::string color;
    std::thread t; /**< Hilo en el que se ejecuta el spinner. */ 
    /**
     * @brief Restablece el color del terminal a su valor predeterminado.
     *
     * @return std::string La cadena ANSI para restablecer el color.
     */
    std::string reset() const { return "\033[0m"; }

    /**
     * @brief Oculta el cursor del terminal.
     * @param  s (default)  true
     */
    void hideCursor(bool s = true) const
    {
        if (s) {
            std::cout << HIDE_CURSOL << std::flush;
        } else {
            std::cout << SHOW_CURSOL << std::flush;
        }
    }
    /**
     * @brief Manejador de señales para restaurar el cursor al salir del programa.
     *
     * @param signal La señal recibida.
     */
    static void handleSignal(int signal)
    {
        if (signal == SIGINT) {
            std::cout
                << SHOW_CURSOL << std::flush
                << "\nSIGINT (Ctrl+C) was received. Exiting the program cleanly...\n";
            std::exit(signal); // Salida limpia
        }
    }
};

} // namespace spinners

#endif // _SPINNERS_HPP_
