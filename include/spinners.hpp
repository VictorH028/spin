/** 
 * @file spinners.hpp
 * @brief Librería para mostrar spinners animados.
 */

#ifndef _SPINNERS_HPP_
#define _SPINNERS_HPP_

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <linux/time.h>
#include <map>
#include <memory>
#include <thread>
#include <format>


#define SHOW_CURSOL "\u001b[?25h"
#define HIDE_CURSOL "\u001b[?25l"

namespace spinners { 
/**
 * @enum Color
 * @brief Enumeración de colores disponibles para los spinners.
 */
enum class Color {
    Red, /**< Color rojo */
    Green, /**< Color verde */
    Blue, /**< Color azul */
    Yellow, /**< Color amarillo */
    Cyan, /**< Color cian */
    Magenta, /**< Color magenta */
    White, /**< Color blanco */
    Default /**< Color predeterminado */
};

/**
 * @brief Convierte un valor de la enumeración Color a una cadena ANSI.
 *
 * @param color El color a convertir.
 * @return std::string La cadena ANSI correspondiente al color.
 */
 std::string colorToAnsi(Color color)
{
    switch (color) {
    case Color::Red:
        return "\033[31m"; // Rojo
    case Color::Green:
        return "\033[32m"; // Verde
    case Color::Blue:
        return "\033[34m"; // Azul
    case Color::Yellow:
        return "\033[33m"; // Amarillo
    case Color::Cyan:
        return "\033[36m"; // Cian
    case Color::Magenta:
        return "\033[35m"; // Magenta
    case Color::White:
        return "\033[37m"; // Blanco
    case Color::Default:
        return "\033[0m"; // Predeterminado
    default:
        return "\033[0m"; // Seguridad
    }
}

/**
 * @brief array que contiene los diferentes tipos de spinners disponibles.
 */
const std::array<std::pair<std::string ,std::string>,42> spinnerType = {{
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
}};

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

/**/
/*const std::string& getSpinner(const std::string& key)*/
/*{*/
/*    auto search = spinnerType.find(key);*/
/*    if (search != spinnerType.end()) {*/
/*        return search->second;*/
/*    }*/
/*    return spinnerType["circleHalves"];*/
/*}*/

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
        , color(stringToColor("blur"))
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
        const std::string color_str)
        : interval(_interval)
        , text(_text)
        , symbols(std::make_unique<std::string>(getSpinner(_symbols)))
        , stop_spinner(false)
        , color(stringToColor(color_str))
    {
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
     * @param colorName El nombre del color.
     */
    void setColor(const std::string colorName)
    {
        color = stringToColor(colorName);
    };

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
                std::cout << std::format("{} {} {}\r", utf8_char, colorToAnsi(color), text) << std::flush;
               // std::cout << utf8_char << colorToAnsi(color) << " " << text << " \r"
                //          << reset() << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        } catch (...) {
            showCursor();
            throw;
        }
        showCursor();
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
                showCursor();
                throw;
            }
        }
        showCursor();
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
    Color color; /**< Color del spinner. */
    std::thread t; /**< Hilo en el que se ejecuta el spinner. */

    /**
     * @brief Convierte una cadena de texto a un valor de la enumeración Color.
     *
     * @param colorName El nombre del color.
     * @return Color El valor de la enumeración Color correspondiente.
     */
    Color stringToColor(const std::string& colorName)
    {
        static const std::map<std::string, Color> colorMap = {
            { "red", Color::Red }, { "green", Color::Green },
            { "blue", Color::Blue }, { "yellow", Color::Yellow },
            { "cyan", Color::Cyan }, { "magenta", Color::Magenta },
            { "white", Color::White }, { "default", Color::Default }
        };

        std::string lowerColor = colorName;
        std::transform(lowerColor.begin(), lowerColor.end(), lowerColor.begin(),
            ::tolower);
        auto it = colorMap.find(lowerColor);
        return (it != colorMap.end()) ? it->second : Color::Default;
    }

    /**
     * @brief Restablece el color del terminal a su valor predeterminado.
     *
     * @return std::string La cadena ANSI para restablecer el color.
     */
   std::string reset() const { return "\033[0m"; }

    /**
     * @brief Oculta el cursor del terminal.
     */
    inline void hideCursor() const { std::cout << HIDE_CURSOL << std::flush; }

    /**
     * @brief Muestra el cursor del terminal.
     */
    inline void showCursor() const  { std::cout << SHOW_CURSOL << std::flush; }

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
