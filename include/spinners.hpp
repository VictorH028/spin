#ifndef SPINNERS_HPP
#define SPINNERS_HPP

#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

const std::array<std::pair<std::string, std::string>, 42> spinnerType = { { { "dots", "⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏" },
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
    { "triangle", "⬖⬘⬗⬙" } } };

enum SpinResult {
    SPIN_SUCCESS,
    SPIN_ERROR,
    SPIN_WARNING,
    SPIN_INFO,
    SPIN_NEXT
};

enum class SpinnerPosition { BEFORE,
    AFTER };
enum class NewlineMode { OVERWRITE,
    APPEND };

class Spinner {
private:
    int interval { 100 };
    std::string text;
    std::mutex text_mutex;
    std::atomic<bool> is_paused { false };
    std::atomic<bool> stop_spinner { false };
    std::atomic<bool> cursor_hidden { false };
    int color { 15 };
    std::vector<std::string> frames;
    std::thread t;
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::ostream* stream { &std::cerr };
    bool show_timer { false };
    bool newline_mode { false };
    SpinnerPosition position { SpinnerPosition::BEFORE };
    std::string text_style { "normal" };

    // NUEVO: retraso antes de pintar el primer frame.
    // Si stop() llega antes, no se pinta nada (comandos rápidos no parpadean).
    int warmup_ms { 100 };

    static std::vector<std::string> parseUtf8Frames(const std::string& raw);
    static std::string ansiStyle(const std::string& style);
    void hideCursor(bool hide);

public:
    Spinner();
    ~Spinner();

    Spinner(const Spinner&) = delete;
    Spinner& operator=(const Spinner&) = delete;

    Spinner& updateText(const std::string& new_text);
    Spinner& setInterval(int ms);
    Spinner& setText(const std::string& txt);
    Spinner& setSymbols(const std::string& key);
    Spinner& setCustomFrames(const std::vector<std::string>& custom_frames);
    Spinner& setCustomFrames(const std::string& raw_frames);
    Spinner& setColor(int ansi_color);
    Spinner& setColor(const std::string& ansi_color);
    Spinner& setOutputStream(std::ostream& os);
    Spinner& withTimer(bool enable = true);
    Spinner& setBold(bool enable = true);
    Spinner& setTextStyle(const std::string& style);
    Spinner& setPosition(SpinnerPosition pos);
    Spinner& setPosition(const std::string& pos);
    Spinner& setNewline(bool enable = true);

    // NUEVO: control del warmup. 0 = pintar desde el primer instante.
    Spinner& setWarmup(int ms);

    void pause();
    void resume();
    void start();
    void stop();
    void stopWithStatus(SpinResult result, const std::string& final_text = "");

    static std::vector<std::string> getAvailableSpinners();
    static std::vector<std::string> parseFrames(const std::string& raw);
    static bool isValidColor(int color);
    static bool isValidTextStyle(const std::string& style);
    static bool isValidPosition(const std::string& position);
};

#endif // SPINNERS_HPP
