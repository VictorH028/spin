#ifndef SYSTERM
#define SYSTERM

// ============================================================================
// systerm.hpp — Ejecución de comandos en paralelo para Termux (Android).
//
// Header-only, C++17, solo POSIX (popen/posix_spawn/flock/sched_setaffinity).
// La configuración es 100% delegada al usuario vía `Options` (builder fluido):
//   - maxConcurrent: 0 = autodetectar (núcleos online acotados por límite de
//     procesos del sistema: RLIMIT_NPROC / _SC_CHILD_MAX). En Android los
//     hilos también cuentan contra RLIMIT_NPROC.
//   - wakeLockThreshold: nº de comandos en cola que activan wake lock (<1
//     desactiva). Sin él, Android suspende el device a mitad de lote.
//   - pinWorkers: fija cada worker a un núcleo distinto (big.LITTLE).
//   - logPath: archivo de registro con rotación.
// ============================================================================

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <spawn.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

// POSIX gems: environ es externo, no lo declara POSIX ni Bionic como API estandarizada.
extern char** environ;

struct CommandResult {
    int total = 0;
    int succeeded = 0;
    int failed = 0;
    bool success() const { return failed == 0; }
};

// Topología visible del sistema (leída, no adivinada).
struct SystemInfo {
    unsigned int onlineCores = 0;
    unsigned int hardwareThreads = 0; // std::thread::hardware_concurrency()
    long processLimit = 0; // RLIMIT_NPROC (Bionic/hilos incluidos)
    long maxConcurrentSafe = 0; // workers recomendados por límite de procs
    std::vector<unsigned int> coreMHz; // freq máx por núcleo (cpufreq)
};

class SystemTermux {
public:
    // ------------------------------------------------------------------
    // Opciones: el usuario delega su configuración aquí (builder fluido).
    // ------------------------------------------------------------------
    struct Options {
        int maxConcurrent = 0; // 0 = autodetectar (núcleos ⇢ cap proceso-límite)
        int wakeLockThreshold = 3; // <1 desactiva wake lock por completo
        bool allowWakeLock = true; // interruptor maestro
        bool pinWorkers = false; // afinity por worker (cada uno a un núcleo)
        std::string logPath; // vacío = $HOME/.spin.log

        Options& setMaxConcurrent(int n)
        {
            maxConcurrent = n < 0 ? 0 : n;
            return *this;
        }
        Options& setWakeLockThreshold(int n)
        {
            wakeLockThreshold = n;
            return *this;
        }
        Options& setAllowWakeLock(bool b)
        {
            allowWakeLock = b;
            return *this;
        }
        Options& setPinWorkers(bool b)
        {
            pinWorkers = b;
            return *this;
        }
        Options& setLogPath(std::string p)
        {
            logPath = std::move(p);
            return *this;
        }
    };

    explicit SystemTermux(const Options& opts)
        : opts_(refine(opts))
    {
    }

    SystemTermux()
        : SystemTermux(Options {})
    {
    }

    const Options& options() const { return opts_; }

    // Reconfigura y devuelve *this para mantener el estilo fluido.
    SystemTermux& configure(const Options& opts)
    {
        opts_ = refine(opts);
        return *this;
    }

    // ------------------------------------------------------------------
    // Ejecución: hasta opts_.maxConcurrent workers consumen de una cola.
    // ------------------------------------------------------------------
    CommandResult run_commands(const std::vector<std::string>& commands,
        bool quiet = false)
    {
        CommandResult result;
        result.total = static_cast<int>(commands.size());
        if (commands.empty())
            return result;

        const bool needsWakeLock = opts_.allowWakeLock
            && opts_.wakeLockThreshold > 0
            && static_cast<int>(commands.size()) >= opts_.wakeLockThreshold;
        WakeLock wakeLock(needsWakeLock);

        LogSink log(opts_.logPath);

        std::atomic<int> succeeded { 0 };
        std::atomic<int> failed { 0 };

        std::mutex qMutex;
        std::deque<std::string> queue(commands.begin(), commands.end());

        const int nWorkers = std::min<int>(
            opts_.maxConcurrent, static_cast<int>(commands.size()));

        // La cola se precarga y nadie produce más items: no hace falta
        // condition_variable (con una, los workers quedaban bloqueados
        // para siempre al vaciarse la cola y join() nunca retornaba).
        auto worker = [&](int workerId) {
            if (opts_.pinWorkers)
                pinToCore(workerId);
            for (;;) {
                std::string cmd;
                {
                    std::lock_guard<std::mutex> lk(qMutex);
                    if (queue.empty())
                        return;
                    cmd = std::move(queue.front());
                    queue.pop_front();
                }
                const int rc = dispatch(cmd, quiet, log);
                if (rc == 0)
                    ++succeeded;
                else
                    ++failed;
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(static_cast<size_t>(nWorkers));
        for (int i = 0; i < nWorkers; ++i)
            threads.emplace_back(worker, i);

        for (auto& t : threads)
            if (t.joinable())
                t.join();

        result.succeeded = succeeded.load();
        result.failed = failed.load();
        return result;
    }

    // static utility: separa un string por comas en una lista de comandos.
    static std::vector<std::string> splitCommands(const std::string& input)
    {
        std::vector<std::string> commands;
        std::stringstream ss(input);
        std::string command;
        while (std::getline(ss, command, ',')) {
            const auto first = command.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                continue;
            const auto last = command.find_last_not_of(" \t\r\n");
            commands.push_back(command.substr(first, last - first + 1));
        }
        return commands;
    }

    // ------------------------------------------------------------------
    // Topología real del hardware y límites del entorno Termux/Android.
    // (complementa a bench.c: mismos datos vía sysconf + RLIMIT_NPROC)
    // ------------------------------------------------------------------
    static SystemInfo info()
    {
        SystemInfo info;

        // hardware_concurrency puede devolver 0 en ciertos entornos: no fiarse.
        info.hardwareThreads = std::thread::hardware_concurrency();
        if (info.hardwareThreads == 0) {
            long n = sysconf(_SC_NPROCESSORS_ONLN);
            info.hardwareThreads = n > 0 ? static_cast<unsigned int>(n) : 1;
        }

        info.onlineCores = info.hardwareThreads;

        // Frecuencia máxima por núcleo, para distinguir núcleos grandes de
        // núcleos de eficiencia al planificar (mismas fuentes que bench: /sys).
        for (unsigned int c = 0; c < info.onlineCores; ++c) {
            std::ifstream freq("/sys/devices/system/cpu/cpu"
                + std::to_string(c) + "/cpufreq/cpuinfo_max_freq");
            if (!freq.is_open())
                break;
            unsigned int khz = 0;
            if (freq >> khz) // kHz → MHz (redondeo arriba)
                info.coreMHz.push_back((khz + 999) / 1000);
        }
        if (info.coreMHz.size() < static_cast<size_t>(info.onlineCores))
            info.coreMHz.clear(); // info parcial no sirve: preferir vacío

        // En Android/Bionic RLIMIT_NPROC cuenta hilos también: cada worker
        // usa 1 hilo del proceso + sh + comando (≈3 unidades en total).
        // Colchón conservador: cap = (límite - 8) / 4. Nunca crear lock.
        struct rlimit rl;
        if (getrlimit(RLIMIT_NPROC, &rl) == 0
            && rl.rlim_cur != RLIM_INFINITY && rl.rlim_cur > 0) {
            info.processLimit = static_cast<long>(rl.rlim_cur);
        } else {
            info.processLimit = sysconf(_SC_CHILD_MAX); // fallback POSIX
        }
        info.maxConcurrentSafe = clampToProcessLimit(info.hardwareThreads,
            info.processLimit);

        return info;
    }

private:
    Options opts_;

    // Rellena maxConcurrent=0 con lo óptimo del hardware:
    //   min(núcleos online, workers "seguros" según límite de procesos).
    static Options refine(const Options& raw)
    {
        Options opts = raw;
        if (opts.maxConcurrent <= 0) {
            const SystemInfo sysInfo = info();
            opts.maxConcurrent = static_cast<int>(sysInfo.maxConcurrentSafe);
        }
        if (opts.maxConcurrent < 1)
            opts.maxConcurrent = 1;
        return opts;
    }

    static long clampToProcessLimit(long n, long procLimit)
    {
        if (procLimit <= 0)
            return n > 0 ? n : 1;
        // 4 procesos-hilo por worker y un margen de 8 para el sistema.
        const long safe = (procLimit - 8) / 4;
        long v = std::min(n, safe);
        return v > 0 ? v : 1;
    }

    // ------------------------------------------------------------------
    // Afinidad por worker (big.LITTLE le saca provecho directo: los
    // A76 procesan más y los A55 ayudan al resto). Fallo silencioso: es
    // un ajuste, no un requisito.
    // ------------------------------------------------------------------
    static void pinToCore(int id)
    {
        long cores = sysconf(_SC_NPROCESSORS_ONLN);
        if (cores <= 0)
            return;
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(id % static_cast<int>(cores), &set);
        ::sched_setaffinity(0, sizeof(set), &set); // aplica al hilo llamante
    }

    // ------------------------------------------------------------------
    // Logging: ofstream por SystemTermux, flush periódico, rotación 1 MiB.
    // ------------------------------------------------------------------
    class LogSink {
    public:
        explicit LogSink(std::string path)
            : path_(path.empty() ? defaultPath() : std::move(path))
        {
        }

        void write(const std::string& command, int exitCode)
        {
            std::lock_guard<std::mutex> lk(m_);
            if (!ensureOpen())
                return;

            std::time_t now = std::time(nullptr);
            std::tm tmBuf {};
            localtime_r(&now, &tmBuf);
            char timeBuf[32];
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tmBuf);

            file_ << "========================================\n"
                  << "Fecha: " << timeBuf << "\n"
                  << "PID: " << getpid() << "\n"
                  << "Comando: " << command << "\n"
                  << "Exit code: " << exitCode << "\n"
                  << "Estado: " << (exitCode == 0 ? "SUCCESS" : "FAILED") << "\n";

            if (++count_ % 16 == 0)
                file_.flush();
        }

    private:
        static std::string defaultPath()
        {
            const char* home = std::getenv("HOME");
            return home ? std::string(home) + "/.spin.log" : ".spin.log";
        }

        void rotateIfNeeded()
        {
            struct stat st {};
            if (::stat(path_.c_str(), &st) != 0)
                return;
            if (static_cast<size_t>(st.st_size) < 1024 * 1024)
                return;

            const std::string old = path_ + ".1";
            ::unlink(old.c_str());
            ::rename(path_.c_str(), old.c_str());
        }

        bool ensureOpen()
        {
            if (file_.is_open())
                return true;
            rotateIfNeeded();
            file_.open(path_, std::ios::app);
            return file_.is_open();
        }

        std::mutex m_;
        std::string path_;
        std::ofstream file_;
        size_t count_ = 0;
    };

    // ------------------------------------------------------------------
    // Wake lock vía posix_spawn.
    // ------------------------------------------------------------------
    class WakeLock {
    public:
        explicit WakeLock(bool enable)
            : active_(enable)
        {
            if (active_)
                spawnHelper("termux-wake-lock");
        }
        ~WakeLock()
        {
            if (active_)
                spawnHelper("termux-wake-unlock");
        }
        WakeLock(const WakeLock&) = delete;
        WakeLock& operator=(const WakeLock&) = delete;

    private:
        static void spawnHelper(const char* name)
        {
            pid_t pid = -1;
            char* argv[] = { const_cast<char*>(name), nullptr };

            const char* prefix = std::getenv("PREFIX");
            std::string absPath;
            if (prefix) {
                absPath = std::string(prefix) + "/bin/" + name;
                char* argvAbs[] = { absPath.data(), nullptr };
                if (posix_spawn(&pid, absPath.c_str(), nullptr, nullptr,
                        argvAbs, environ)
                        == 0
                    && pid > 0) {
                    // No esperamos con waitpid: `am startservice` puede
                    // colgarse y arrastrar a spin. El zombi se recoge al salir.
                    return;
                }
            }
            posix_spawnp(&pid, name, nullptr, nullptr, argv, environ);
        }

        bool active_;
    };

    // ------------------------------------------------------------------
    // Detección precisa de apt/pkg (por primer token).
    // ------------------------------------------------------------------
    static bool isAptOrPkg(const std::string& cmd)
    {
        size_t i = cmd.find_first_not_of(" \t");
        if (i == std::string::npos)
            return false;
        std::string_view s(cmd.data() + i, cmd.size() - i);
        size_t end = s.find_first_of(" \t");
        if (end == std::string_view::npos)
            end = s.size();
        std::string_view token = s.substr(0, end);

        return token == "apt" || token == "apt-get" || token == "apt-cache" || token == "apt-mark" || token == "pkg" || token == "aptitude";
    }

    // ------------------------------------------------------------------
    // flock sobre el lock de dpkg.
    // ------------------------------------------------------------------
    class DpkgLock {
    public:
        DpkgLock()
        {
            const char* prefix = std::getenv("PREFIX");
            if (!prefix)
                return;
            std::string p = std::string(prefix) + "/var/lib/dpkg/lock";
            // Sin O_CREAT: si el lock no existe, no hacemos nada, nunca
            // creamos archivos en el árbol de dpkg.
            fd_ = ::open(p.c_str(), O_RDWR);
            if (fd_ >= 0) {
                if (::flock(fd_, LOCK_EX) != 0) {
                    ::close(fd_);
                    fd_ = -1;
                }
            }
        }
        ~DpkgLock()
        {
            if (fd_ >= 0) {
                ::flock(fd_, LOCK_UN);
                ::close(fd_);
            }
        }
        DpkgLock(const DpkgLock&) = delete;
        DpkgLock& operator=(const DpkgLock&) = delete;

    private:
        int fd_ = -1;
    };

    // ------------------------------------------------------------------
    // popen con stderr unido al pipe y limpieza de línea del spinner.
    // ------------------------------------------------------------------
    static int executeWithPopen(const std::string& command, bool quiet,
        LogSink& log)
    {
        std::string finalCommand;
        if (quiet) {
            finalCommand = "(" + command + ") >/dev/null 2>&1";
        } else {
            // Unir stderr al pipe: así el error de `sh` no se pega al
            // frame del spinner, pasa por el mismo canal y lo limpiamos.
            finalCommand = "(" + command + ") 2>&1";
        }

        FILE* pipe = popen(finalCommand.c_str(), "r");
        if (!pipe) {
            log.write(command, -1);
            return -1;
        }

        if (!quiet) {
            char buffer[256];
            bool first = true;
            while (std::fgets(buffer, sizeof(buffer), pipe)) {
                if (first) {
                    // Borra el frame del spinner (vive en stderr) antes de
                    // imprimir la salida; stdout queda limpio para pipear.
                    std::fputs("\033[2K\r", stderr);
                    first = false;
                }
                std::fputs(buffer, stdout);
            }
            if (!first) {
                std::fflush(stdout);
                std::fflush(stderr);
            }
        }

        const int rawStatus = pclose(pipe);
        int exitCode = -1;
        if (rawStatus != -1) {
            if (WIFEXITED(rawStatus))
                exitCode = WEXITSTATUS(rawStatus);
            else if (WIFSIGNALED(rawStatus))
                exitCode = 128 + WTERMSIG(rawStatus);
        }
        log.write(command, exitCode);
        return exitCode;
    }

    // ------------------------------------------------------------------
    // posix_spawn para modo quiet (sin captura de salida).
    // ------------------------------------------------------------------
    static int executeWithSpawn(const std::string& command, LogSink& log)
    {
        pid_t pid = -1;
        char* argv[] = {
            const_cast<char*>("sh"),
            const_cast<char*>("-c"),
            const_cast<char*>(command.c_str()),
            nullptr
        };

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO,
            "/dev/null", O_WRONLY, 0);
        posix_spawn_file_actions_addopen(&actions, STDERR_FILENO,
            "/dev/null", O_WRONLY, 0);

        const int rc = posix_spawnp(&pid, "sh", &actions, nullptr, argv, environ);
        posix_spawn_file_actions_destroy(&actions);

        if (rc != 0 || pid <= 0) {
            log.write(command, -1);
            return -1;
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            log.write(command, -1);
            return -1;
        }

        int exitCode = -1;
        if (WIFEXITED(status))
            exitCode = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            exitCode = 128 + WTERMSIG(status);

        log.write(command, exitCode);
        return exitCode;
    }

    static int dispatch(const std::string& command, bool quiet, LogSink& log)
    {
        if (isAptOrPkg(command)) {
            DpkgLock lock;
            return executeWithPopen(command, quiet, log);
        }

        if (quiet)
            return executeWithSpawn(command, log);
        return executeWithPopen(command, quiet, log);
    }
};

#endif // SYSTERM
