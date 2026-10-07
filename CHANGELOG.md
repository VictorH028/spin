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
