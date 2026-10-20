#include "../include/spinners.hpp"
#include <algorithm>
#include <csignal>
#include <cstring>
#include <sstream>
#include <unistd.h>

namespace {
std::string trim(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool hasFrameSeparator(const std::string& raw)
{
    return raw.find(',') != std::string::npos || raw.find('|') != std::string::npos;
}

// ---------- Restauración de terminal ante señales ----------
inline void writeAllFd(int fd, const char* s, size_t n)
{
    while (n > 0) {
        const ssize_t w = ::write(fd, s, n);
        if (w <= 0)
            return;
        s += static_cast<size_t>(w);
        n -= static_cast<size_t>(w);
    }
}

void restoreTerminalOnSignal(int sig)
{
    static const char kShowCursor[] = "\033[?25h";
    static const char kResetSgr[] = "\033[0m";
    static const char kNewline[] = "\n";

    // El spinner vive en stderr: restaurar ahí, no en stdout.
    writeAllFd(STDERR_FILENO, kShowCursor, sizeof(kShowCursor) - 1);
    writeAllFd(STDERR_FILENO, kResetSgr, sizeof(kResetSgr) - 1);
    writeAllFd(STDERR_FILENO, kNewline, sizeof(kNewline) - 1);

    _exit(128 + sig);
}

struct SignalGuard {
    SignalGuard()
    {
        std::signal(SIGINT, restoreTerminalOnSignal);
        std::signal(SIGTERM, restoreTerminalOnSignal);
        std::signal(SIGHUP, restoreTerminalOnSignal);
    }
};

inline void ensureSignalGuardInstalled()
{
    static SignalGuard guard;
    (void)guard;
}
}

std::vector<std::string> Spinner::getAvailableSpinners()
{
    std::vector<std::string> names;
    names.reserve(spinnerType.size());
    for (const auto& item : spinnerType)
        names.push_back(item.first);
    return names;
}

Spinner::Spinner()
{
    if (!spinnerType.empty())
        frames = parseUtf8Frames(spinnerType[0].second);
}

Spinner::~Spinner()
{
    stop();
    // Solo restaurar si realmente se ocultó (los comandos instantáneos
    // con warmup nunca ocultan el cursor).
    if (stream && cursor_hidden.load())
        *stream << "\033[?25h" << std::flush;
}

void Spinner::hideCursor(bool hide)
{
    cursor_hidden.store(hide);
    if (stream)
        *stream << (hide ? "\033[?25l" : "\033[?25h") << std::flush;
}

Spinner& Spinner::setInterval(int ms)
{
    interval = std::max(1, ms);
    return *this;
}
Spinner& Spinner::setText(const std::string& txt)
{
    std::lock_guard<std::mutex> lock(text_mutex);
    text = txt;
    return *this;
}
Spinner& Spinner::updateText(const std::string& new_text) { return setText(new_text); }

bool Spinner::isValidColor(int value) { return value >= 0 && value <= 255; }

Spinner& Spinner::setColor(int ansi_color)
{
    if (isValidColor(ansi_color))
        color = ansi_color;
    return *this;
}
Spinner& Spinner::setColor(const std::string& ansi_color)
{
    try {
        size_t consumed = 0;
        const int value = std::stoi(ansi_color, &consumed);
        if (consumed == ansi_color.size())
            setColor(value);
    } catch (...) {
    }
    return *this;
}
Spinner& Spinner::setOutputStream(std::ostream& os)
{
    stop();
    stream = &os;
    return *this;
}
Spinner& Spinner::withTimer(bool enable)
{
    show_timer = enable;
    return *this;
}
Spinner& Spinner::setBold(bool enable)
{
    text_style = enable ? "bold" : "normal";
    return *this;
}

bool Spinner::isValidTextStyle(const std::string& style)
{
    return style == "normal" || style == "bold" || style == "dim" || style == "italic" || style == "underline" || style == "reverse";
}
std::string Spinner::ansiStyle(const std::string& style)
{
    if (style == "bold")
        return "\033[1m";
    if (style == "dim")
        return "\033[2m";
    if (style == "italic")
        return "\033[3m";
    if (style == "underline")
        return "\033[4m";
    if (style == "reverse")
        return "\033[7m";
    return "";
}
Spinner& Spinner::setTextStyle(const std::string& style)
{
    if (isValidTextStyle(style))
        text_style = style;
    return *this;
}
bool Spinner::isValidPosition(const std::string& pos)
{
    return pos == "before" || pos == "after";
}
Spinner& Spinner::setPosition(const std::string& pos)
{
    if (pos == "before")
        position = SpinnerPosition::BEFORE;
    else if (pos == "after")
        position = SpinnerPosition::AFTER;
    return *this;
}
Spinner& Spinner::setPosition(SpinnerPosition pos)
{
    position = pos;
    return *this;
}
Spinner& Spinner::setNewline(bool enable)
{
    newline_mode = enable;
    return *this;
}

Spinner& Spinner::setCustomFrames(const std::vector<std::string>& custom_frames)
{
    frames.clear();
    for (const auto& frame : custom_frames)
        if (!frame.empty())
            frames.push_back(frame);
    return *this;
}
Spinner& Spinner::setCustomFrames(const std::string& raw_frames)
{
    return setCustomFrames(parseFrames(raw_frames));
}
Spinner& Spinner::setSymbols(const std::string& key)
{
    const auto it = std::find_if(
        spinnerType.begin(), spinnerType.end(),
        [&key](const auto& item) { return item.first == key; });
    if (it != spinnerType.end())
        frames = parseUtf8Frames(it->second);
    return *this;
}

std::vector<std::string> Spinner::parseUtf8Frames(const std::string& raw)
{
    std::vector<std::string> result;
    for (size_t i = 0; i < raw.size();) {
        const unsigned char c = static_cast<unsigned char>(raw[i]);
        size_t len = 1;
        if ((c & 0x80) == 0)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        else {
            result.emplace_back(1, raw[i]);
            ++i;
            continue;
        }

        if (i + len > raw.size()) {
            result.emplace_back(1, raw[i]);
            ++i;
            continue;
        }
        result.push_back(raw.substr(i, len));
        i += len;
    }
    return result;
}

std::vector<std::string> Spinner::parseFrames(const std::string& raw)
{
    if (raw.empty())
        return {};

    if (hasFrameSeparator(raw)) {
        std::vector<std::string> result;
        std::string current;
        for (char c : raw) {
            if (c == ',' || c == '|') {
                current = trim(current);
                if (!current.empty())
                    result.push_back(current);
                current.clear();
            } else
                current += c;
        }
        current = trim(current);
        if (!current.empty())
            result.push_back(current);
        return result;
    }
    return parseUtf8Frames(raw);
}

void Spinner::start()
{
    stop();

    ensureSignalGuardInstalled();

    stop_spinner = false;
    is_paused = false;
    start_time = std::chrono::steady_clock::now();

    t = std::thread([this]() {
        // ---- Warmup: si stop() llega antes, no pintamos NADA ----
        if (warmup_ms > 0) {
            const auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(warmup_ms);
            while (!stop_spinner && std::chrono::steady_clock::now() < deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            if (stop_spinner)
                return; // comando instantáneo: salir sin tocar terminal
        }

        hideCursor(true);

        size_t frame_idx = 0;
        try {
            while (!stop_spinner) {
                if (is_paused) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                if (!frames.empty() && stream) {
                    std::string current_text;
                    {
                        std::lock_guard<std::mutex> lock(text_mutex);
                        current_text = text;
                    }

                    std::ostringstream ss;
                    if (!newline_mode)
                        ss << "\033[2K\r";

                    const std::string style = ansiStyle(text_style);
                    const std::string spinner = frames[frame_idx];

                    ss << "\033[38;5;" << color << "m";
                    if (position == SpinnerPosition::BEFORE) {
                        ss << spinner;
                        if (!current_text.empty())
                            ss << " ";
                        ss << style << current_text << "\033[0m";
                    } else {
                        ss << style << current_text << "\033[0m";
                        if (!current_text.empty())
                            ss << " ";
                        ss << spinner << "\033[0m";
                    }

                    if (show_timer) {
                        const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::steady_clock::now() - start_time)
                                                 .count();
                        ss << " \033[90m[" << elapsed << "s]\033[0m";
                    }

                    if (newline_mode)
                        ss << '\n';
                    *stream << ss.str() << std::flush;
                    frame_idx = (frame_idx + 1) % frames.size();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        } catch (...) {
        }

        if (stream && !newline_mode)
            *stream << "\033[2K\r" << std::flush;
        hideCursor(false);
    });
}

void Spinner::stop()
{
    stop_spinner = true;
    if (t.joinable())
        t.join();
}

void Spinner::stopWithStatus(SpinResult result, const std::string& final_text)
{
    stop();
    if (!stream)
        return;

    std::string icon = "✔";
    if (result == SPIN_ERROR)
        icon = "✖";
    else if (result == SPIN_WARNING)
        icon = "⚠";
    else if (result == SPIN_INFO)
        icon = "ℹ";

    std::string msg;
    {
        std::lock_guard<std::mutex> lock(text_mutex);
        msg = final_text.empty() ? text : final_text;
    }

    *stream << "\033[38;5;" << color << "m"
            << ansiStyle(text_style) << icon << "\033[0m ";
    *stream << ansiStyle(text_style) << msg << "\033[0m\n"
            << std::flush;
}

Spinner& Spinner::setWarmup(int ms)
{
    warmup_ms = (ms < 0) ? 0 : ms;
    return *this;
}
