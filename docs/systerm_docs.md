# SystemTermux

Librería header-only (C++17, solo POSIX) para ejecutar comandos en paralelo dentro de **Termux (Android)**. Un solo archivo: copia `systerm.hpp` a tu proyecto y listo.

```bash
g++ -std=c++17 archivo.cpp   # std::thread enlaza pthread implícitamente
```

## Uso rápido

```cpp
#include "systerm.hpp"

int main()
{
    // 1. Ejecución simple con config por defecto (todo autodetectado)
    SystemTermux sys;
    CommandResult r = sys.run_commands({
        "pkg update",
        "echo hola",
        "df -h"
    });
    printf("total=%d ok=%d fallidos=%d\n", r.total, r.succeeded, r.failed);
}
```

## Configuración delegada al usuario (`Options`, builder fluido)

```cpp
SystemTermux sys(
    SystemTermux::Options{}
        .setMaxConcurrent(6)        // workers paralelos (0 = autodetectar, default)
        .setPinWorkers(true)        // fija cada worker a un núcleo (big.LITTLE)
        .setWakeLockThreshold(2)    // >=N comandos en cola → wake lock (default 3)
        .setAllowWakeLock(true)     // interruptor maestro del wake lock (default true)
        .setLogPath("/sdcard/.spin.log") // ruta del registro (default $HOME/.spin.log)
);
sys.run_commands({...});
sys.configure(SystemTermux::Options{}.setMaxConcurrent(8)); // reconfigurar después
```

| Opción | Default | Qué hace |
|---|---|---|
| `setMaxConcurrent(n)` | `0` | Nº de workers. `0` = `hardware_concurrency()` (8 en este equipo) capado por `RLIMIT_NPROC`. Mínimo forzado: 1. |
| `setPinWorkers(b)` | `false` | `sched_setaffinity` round-robin: 2 workers en los A76 @2.2GHz y el resto en A55 @2.0GHz. Fallo silencioso. |
| `setWakeLockThreshold(n)` | `3` | Si la cola tiene ≥ N comandos se pide wake lock. `n < 1` desactiva por umbral. |
| `setAllowWakeLock(b)` | `true` | Desactiva el wake lock por completo. |
| `setLogPath(path)` | `$HOME/.spin.log` | Ruta del log de comandos. Rotación a 1 MiB (`.spin.log.1`). |

## El resto de la API

### `SystemTermux::info()`

Topología real del sistema (no adivinada): CPUs online, `hardware_concurrency`, límite de procesos (`RLIMIT_NPROC`, fallback `_SC_CHILD_MAX`), workers recomendados y frecuencia máx por núcleo — las mismas fuentes que `bench.c` usa vía `sysconf`. Útil para decidir `maxConcurrent` tú mismo:

```cpp
SystemTermux::SystemInfo i = SystemTermux::info();
SystemTermux sys(SystemTermux::Options{}.setMaxConcurrent((int)i.maxConcurrentSafe));
```

### `SystemTermux::splitCommands(std::string)`

```cpp
auto cmds = SystemTermux::splitCommands("echo a, echo b , echo c");
// → {"echo a", "echo b", "echo c"}  (trim en cada parte)
```

**Cuidado**: separa por comas sin escape. Un comando que contenga una coma se rompe; en ese caso usa directamente `run_commands(std::vector<std::string>)`.

### `run_commands(commands, quiet)`

- `quiet = false` (default): sale por `stdout` en tiempo real via `popen` (`2>&1`), limpia el frame del spinner con `\033[2K\r` antes de imprimir.
- `quiet = true`: via `posix_spawn`, stdout/stderr a `/dev/null`. Solo te queda el exit code y el log.

Devuelve `CommandResult { total, succeeded, failed, success() }`.

## Comportamiento en Termux (lo que hace solo)

- **Wake lock**: con ≥ `wakeLockThreshold` comandos lanza `$PREFIX/bin/termux-wake-lock` y al terminar `termux-wake-unlock`, vía `posix_spawn`. Requiere `$PREFIX` definido; sin él, se omite silenciosamente.
- **apt/pkg**: comandos cuyo primer token sea `apt|apt-get|apt-cache|apt-mark|pkg|aptitude` se ejecutan en exclusión mutua bajo `flock` sobre `$PREFIX/var/lib/dpkg/lock` — evita corromper dpkg si dos workers instalan a la vez.
- **Límite de procesos**: Android cuenta hilos y procesos juntos (`RLIMIT_NPROC`); el autodetectado respeta ese límite con margen para que el sistema respire.
- **Log**: todo comando queda en `$HOME/.spin.log` (fecha, exit code, estado SUCCESS/FAILED) — primera parada para depurar fallos.


## Limitaciones conocidas

- Solo POSIX (Bionic/Linux). Sin soporte Windows.
- No hay cancelación: un lote corre hasta completarse (un comando colgado cuelga el lote).
- El log no captura la salida del comando, solo su exit code.
- `setPinWorkers` no distingue big/LITTLE en scheduling: más workers que núcleos grandes = colas naturales por núcleo.
