# Spin v5.1 — cambios

- `SystemTermux` reescrito con API de instancia y builder fluido `Options`.
- Concurrencia autodetectada: workers = min(núcleos online, límite seguro por RLIMIT_NPROC); antes fijo en 4.
- Nueva opción `-j/--jobs` para fijar la concurrencia manualmente (0 = autodetectar).
- Nueva opción `--pin-workers` para fijar cada worker a un núcleo (big.LITTLE).
- Wake lock configurable: `--wake-threshold=N` (<1 desactiva) y `--no-wake-lock`.
- Nueva opción `--log=PATH` para elegir el archivo de registro (por defecto `$HOME/.spin.log`).
- Nueva opción `--show-system`: muestra núcleos, límite de procesos y frecuencias del hardware.
- Alias con guion bajo eliminados (`--show_colors` → `--show-colors`, etc.); solo grafía con guion medio.
- Corregido deadlock en la cola de workers que colgaba todo `--cmd`.
- La salida del comando ya no se contamina con secuencias ANSI del spinner (stdout limpio para pipear).
- `nice(10)` eliminado: degradaba el proceso entero de forma acumulativa.
- `DpkgLock` ya no crea archivos en el árbol de dpkg si el lock no existe.
- Wake lock sin `waitpid` bloqueante (evita colgones con `am startservice`).
- Sin `--cmd` ahora avisa y devuelve rc=1 (antes salía en silencio con rc=0).
- Tests: 42 → 57 (opciones de ejecución, `--show-system`, logs).

# Spin v5.0 — cambios

- Headers faltantes corregidos.
- Protección con mutex para el texto.
- Intervalo validado para evitar valores <= 0.
- Color ANSI validado (0-255).
- `--frames` para pasar frames personalizados desde Shell.
- `--position before|after` para colocar el cargador antes/después del texto.
- `--text-style` para normal, bold, dim, italic, underline y reverse.
- Nuevas opciones `--show_text_styles` y `--show_positions`.
- Lectura correcta del `popen()` para evitar bloqueos del pipe.
- `.spin.log` se guarda de forma consistente en `$HOME`.
