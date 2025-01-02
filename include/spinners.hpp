#ifndef _SPINNERS_HPP_
#define _SPINNERS_HPP_

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

namespace spinners {
enum class Color { Red, Green, Blue, Yellow, Cyan, Magenta, White, Default };
std::string colorToAnsi(Color color) {
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

std::map<const std::string, const std::string> spinnerType = {
    {"dots", u8"⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"},
    {"dots2", u8"⣾⣽⣻⢿⡿⣟⣯⣷"},
    {"dots3", u8"⠋⠙⠚⠞⠖⠦⠴⠲⠳⠓"},
    {"dots4", u8"⠄⠆⠇⠋⠙⠸⠰⠠⠰⠸⠙⠋⠇⠆"},
    {"dots5", u8"⠋⠙⠚⠒⠂⠂⠒⠲⠴⠦⠖⠒⠐⠐⠒⠓⠋"},
    {"dots6", u8"⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠴⠲⠒⠂⠂⠒⠚⠙⠉⠁"},
    {"dots7", u8"⠈⠉⠋⠓⠒⠐⠐⠒⠖⠦⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈"},
    {"dots8", u8"⠁⠁⠉⠙⠚⠒⠂⠂⠒⠲⠴⠤⠄⠄⠤⠠⠠⠤⠦⠖⠒⠐⠐⠒⠓⠋⠉⠈⠈"},
    {"dots9", u8"⢹⢺⢼⣸⣇⡧⡗⡏"},
    {"dots10", u8"⢄⢂⢁⡁⡈⡐⡠"},
    {"dots11", u8"⠁⠂⠄⡀⢀⠠⠐⠈"},
    {"pipe", u8"┤┘┴└├┌┬┐"},
    {"star", u8"✶✸✹✺✹✷"},
    {"flip", u8"___-`'´-___"},
    {"hamburger", u8"☱☲☴"},
    {"growVertical", u8"▁▃▄▅▆▇▆▅▄▃"},
    {"growHorizontal", u8"▏▎▍▌▋▊▉▊▋▌▍▎"},
    {"balloon", u8" . o O @ * "},
    {"balloon2", u8".oO°Oo."},
    {"noise", u8"▓▒░"},
    {"bounce", u8"⠁⠂⠄⠂"},
    {"boxBounce", u8"▖▘▝▗"},
    {"boxBounce2", u8"▌▀▐▄"},
    {"triangle", u8"◢◣◤◥"},
    {"arc", u8"◜◠◝◞◡◟"},
    {"circle", u8"◡⊙◠"},
    {"squareCorners", u8"◰◳◲◱"},
    {"circleQuarters", u8"◴◷◶◵"},
    {"circleHalves", u8"◐◓◑◒"},
    {"squish", u8"╫╪"},
    {"toggle", u8"⊶⊷"},
    {"toggle5", u8"▮▯"},
    {"toggle6", u8"ဝ၀"},
    {"toggle7", u8"⦾⦿"},
    {"toggle8", u8"◍◌"},
    {"toggle9", u8"◉◎"},
    {"toggle10", u8"㊂㊀㊁"},
    {"toggle11", u8"⧇⧆"},
    {"toggle12", u8"☗☖"},
    {"arrow", u8"←↖↑↗→↘↓↙"},
    {"arrow2", u8"➞➟➠➡➠➟"},
    {"triangle", u8"⬖⬘⬗⬙"},
};

const std::string &getSpinner(const std::string &key) {
  auto search = spinnerType.find(key);
  if (search != spinnerType.end()) {
    return search->second;
  }
  return spinnerType["circleHalves"];
}

class Spinner {
public:
  Spinner()
      : interval(80), text(""),
        symbols(std::make_shared<std::string>(getSpinner("circleHalves"))),
        stop_spinner(false), color(stringToColor("blur")) {}

  Spinner(int _interval, const std::string &_text, const std::string &_symbols,
          const std::string color_str)
      : interval(_interval), text(_text),
        symbols(std::make_shared<std::string>(getSpinner(_symbols))),
        stop_spinner(false), color(stringToColor(color_str)) {}

  ~Spinner() { stop(); }

  void setInterval(int _interval) { interval = _interval; }
  void setText(const std::string &_text) { text = _text; }
  void setSymbols(const std::string &_symbols) {
    symbols = std::make_shared<std::string>(getSpinner(_symbols));
  }
  void setColor(const std::string colorName) {
    color = stringToColor(colorName);
  };

  void startSpinner() {
    const size_t len = symbols->size();
    size_t i = 0;

    hideCursor();
    try {
      while (!stop_spinner.load()) {
        // Obtener un carácter basado en UTF-8
        std::string utf8_char = symbols->substr(i, 3); // 3 bytes por símbolo
        i = (i + 3) % len;
        std::cout << utf8_char << colorToAnsi(color) << " " << text << " \r"
                  << reset() << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
      }
    } catch (...) {
      showCursor();
      throw;
    }
    showCursor();
  }

  void start() { t = std::thread(&Spinner::startSpinner, this); }

  void stop() {
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

  // Configurar el manejador de señales
  static void setupSignalHandlers() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
  }

private:
  int interval;
  std::string text;
  std::shared_ptr<std::string> symbols;
  std::atomic<bool> stop_spinner;
  Color color;
  std::thread t;

  Color stringToColor(const std::string &colorName) {
    static const std::map<std::string, Color> colorMap = {
        {"red", Color::Red},     {"green", Color::Green},
        {"blue", Color::Blue},   {"yellow", Color::Yellow},
        {"cyan", Color::Cyan},   {"magenta", Color::Magenta},
        {"white", Color::White}, {"default", Color::Default}};

    std::string lowerColor = colorName;
    std::transform(lowerColor.begin(), lowerColor.end(), lowerColor.begin(),
                   ::tolower);
    auto it = colorMap.find(lowerColor);
    return (it != colorMap.end()) ? it->second : Color::Default;
  }

  std::string reset() { return "\033[0m"; }

  void hideCursor() { std::cout << "\u001b[?25l" << std::flush; }

  void showCursor() { std::cout << "\u001b[?25h" << std::flush; }

  // Función para restaurar el cursor cuando se interrumpe el programa
  static void handleSignal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\u001b[?25h" << std::flush << "\nSIGINT (Ctrl+C) was received. Exiting the program cleanly...\n";
        std::exit(signal); // Salida limpia
    }
  }
};

} // namespace spinners

#endif // _SPINNERS_HPP_
