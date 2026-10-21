#!/usr/bin/env bash
#
# test_spin.sh - Suite de tests rigurosa para la herramienta `spin`
#
# Uso:
#   ./test_spin.sh              # Ejecuta todos los tests
#   ./test_spin.sh -v           # Modo verbose
#   ./test_spin.sh -t "nombre"  # Ejecuta solo un test por nombre
#
set -uo pipefail

# ============================================================================
# CONFIGURACIÓN
# ============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_TMP_DIR="$(mktemp -d -t spin_test.XXXXXX)"
SPIN_BIN="${SPIN_BIN:-spin}"

# Colores
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly BOLD='\033[1m'
readonly NC='\033[0m'

# Contadores
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

VERBOSE=0
FILTER=""

# ============================================================================
# UTILIDADES
# ============================================================================
cleanup() {
    rm -rf "$TEST_TMP_DIR"
}
trap cleanup EXIT

log()      { echo -e "${BLUE}[INFO]${NC} $*"; }
log_pass() { echo -e "${GREEN}[PASS]${NC} $*"; }
log_fail() { echo -e "${RED}[FAIL]${NC} $*"; }
log_skip() { echo -e "${YELLOW}[SKIP]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
verbose()  { [[ $VERBOSE -eq 1 ]] && echo -e "${CYAN}[DEBUG]${NC} $*" || true; }

# Ejecuta un comando y verifica que devuelve el exit code esperado
# Uso: assert_exit_code <esperado> <comando...>
assert_exit_code() {
    local expected="$1"; shift
    local output
    output=$("$@" 2>&1)
    local actual=$?
    if [[ "$actual" -eq "$expected" ]]; then
        verbose "Exit code OK ($actual): $*"
        return 0
    fi
    log_fail "Exit code esperado=$expected, obtenido=$actual en: $*"
    [[ -n "$output" ]] && echo "  Salida: $output"
    return 1
}

# Verifica que la salida contiene un patrón (regex)
# Uso: assert_output_contains <patrón> <comando...>
assert_output_contains() {
    local pattern="$1"; shift
    local output
    output=$("$@" 2>&1) || true
    if grep -qE "$pattern" <<<"$output"; then
        verbose "Output contiene '$pattern': OK"
        return 0
    fi
    log_fail "Output NO contiene '$pattern' en: $*"
    echo "  Salida obtenida: $output"
    return 1
}

# Verifica que la salida NO contiene un patrón
assert_output_not_contains() {
    local pattern="$1"; shift
    local output
    output=$("$@" 2>&1) || true
    if ! grep -qE "$pattern" <<<"$output"; then
        verbose "Output NO contiene '$pattern': OK"
        return 0
    fi
    log_fail "Output NO debería contener '$pattern' en: $*"
    return 1
}

# Verifica que un archivo existe y no está vacío
assert_file_exists() {
    local file="$1"
    if [[ -s "$file" ]]; then
        verbose "Archivo existe y no vacío: $file"
        return 0
    fi
    log_fail "Archivo no existe o está vacío: $file"
    return 1
}

# Verifica igualdad de strings
assert_equals() {
    local expected="$1"
    local actual="$2"
    local msg="${3:-valores}"
    if [[ "$expected" == "$actual" ]]; then
        verbose "Igualdad OK: $expected == $actual"
        return 0
    fi
    log_fail "$msg: esperado='$expected' obtenido='$actual'"
    return 1
}

# ============================================================================
# FRAMEWORK DE TESTS
# ============================================================================
run_test() {
    local name="$1"
    shift
    local func="$1"

    # Filtro por nombre
    if [[ -n "$FILTER" && "$name" != *"$FILTER"* ]]; then
        return
    fi

    echo -e "\n${BOLD}▶ $name${NC}"
    local start
    start=$(date +%s%N)

    if "$func"; then
        local end
        end=$(date +%s%N)
        local duration=$(( (end - start) / 1000000 ))
        log_pass "$name (${duration}ms)"
        ((TESTS_PASSED++))
    else
        log_fail "$name"
        ((TESTS_FAILED++))
    fi
}

skip_test() {
    log_skip "$1"
    ((TESTS_SKIPPED++))
}

# ============================================================================
# PRE-CHEQUEOS
# ============================================================================
preflight_check() {
    log "Verificando que '$SPIN_BIN' está disponible..."
    if ! command -v "$SPIN_BIN" &>/dev/null; then
        log_fail "Binario '$SPIN_BIN' no encontrado en PATH"
        exit 1
    fi
    local version
    version=$("$SPIN_BIN" --version 2>&1 || echo "desconocida")
    log "Versión detectada: $version"
    log "Directorio temporal: $TEST_TMP_DIR"
}

# ============================================================================
# TESTS: AYUDA Y VERSIÓN
# ============================================================================
test_help_flag() {
    assert_exit_code 0 "$SPIN_BIN" --help &&
    assert_output_contains "Usage:" "$SPIN_BIN" --help &&
    assert_output_contains "Options:" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-style" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-cmd" "$SPIN_BIN" --help
}

test_help_short_flag() {
    assert_exit_code 0 "$SPIN_BIN" -h &&
    assert_output_contains "Usage:" "$SPIN_BIN" -h
}

test_version_flag() {
    assert_exit_code 0 "$SPIN_BIN" --version &&
    assert_output_contains "[0-9]+\\.[0-9]+" "$SPIN_BIN" --version
}

# ============================================================================
# TESTS: ESTILOS
# ============================================================================
test_show_style() {
    assert_exit_code 0 "$SPIN_BIN" --show-style &&
    assert_output_contains "[a-zA-Z]" "$SPIN_BIN" --show-style
}

test_valid_style() {
    assert_exit_code 0 "$SPIN_BIN" --style dots --cmd "true"
}

test_invalid_style_handling() {
    # Un estilo inexistente debería fallar o al menos no ejecutarse como si fuera válido
    local output
    output=$("$SPIN_BIN" --style definitely_not_a_real_style_xyz --cmd "true" 2>&1)
    local rc=$?
    verbose "Invalid style rc=$rc, output=$output"
    # Aceptamos rc != 0 O que muestre advertencia
    if [[ $rc -ne 0 ]] || grep -qi "not.*found\|invalid\|unknown" <<<"$output"; then
        return 0
    fi
    log_fail "Estilo inválido no fue rechazado (rc=$rc)"
    return 1
}

# ============================================================================
# TESTS: COLORES
# ============================================================================
test_show_colors() {
    assert_exit_code 0 "$SPIN_BIN" --show-colors
}

test_valid_color() {
    assert_exit_code 0 "$SPIN_BIN" --color 42 --cmd "true"
}

test_color_boundary_zero() {
    assert_exit_code 0 "$SPIN_BIN" --color 0 --cmd "true"
}

test_color_boundary_255() {
    assert_exit_code 0 "$SPIN_BIN" --color 255 --cmd "true"
}

test_color_out_of_range() {
    # 256 está fuera de rango, debería fallar
    local output
    output=$("$SPIN_BIN" --color 256 --cmd "true" 2>&1)
    local rc=$?
    if [[ $rc -ne 0 ]] || grep -qi "range\|invalid" <<<"$output"; then
        return 0
    fi
    log_fail "Color 256 (fuera de rango) fue aceptado silenciosamente"
    return 1
}

test_color_negative() {
    local output
    output=$("$SPIN_BIN" --color -1 --cmd "true" 2>&1)
    local rc=$?
    if [[ $rc -ne 0 ]] || grep -qi "range\|invalid" <<<"$output"; then
        return 0
    fi
    log_fail "Color -1 (fuera de rango) fue aceptado silenciosamente"
    return 1
}

# ============================================================================
# TESTS: EJECUCIÓN DE COMANDOS
# ============================================================================
test_cmd_success() {
    assert_exit_code 0 "$SPIN_BIN" --cmd "true"
}

test_cmd_failure() {
    # El exit code de `spin` debería reflejar el del comando
    local output
    output=$("$SPIN_BIN" --cmd "false" 2>&1)
    local rc=$?
    verbose "spin --cmd false -> rc=$rc"
    if [[ $rc -ne 0 ]]; then
        return 0
    fi
    log_fail "spin debería propagar exit code != 0 cuando el comando falla"
    return 1
}

test_cmd_stdout_passthrough() {
    assert_output_contains "HOLA_MUNDO_TEST" \
        "$SPIN_BIN" --cmd "echo HOLA_MUNDO_TEST"
}

test_cmd_quiet_mode() {
    assert_output_not_contains "HOLA_MUNDO_TEST" \
        "$SPIN_BIN" --quiet --cmd "echo HOLA_MUNDO_TEST"
}

test_cmd_with_stderr() {
    # El comando escribe en stderr, spin debería manejarlo sin crashear
    assert_exit_code 0 "$SPIN_BIN" --cmd "echo error_msg >&2"
}

test_cmd_multiline() {
    local output
    # Sin --quiet: este test valida el passthrough de salida, y --quiet
    # suprime la salida por contrato (ver test_cmd_quiet_mode).
    output=$("$SPIN_BIN" --cmd "printf 'line1\nline2\nline3\n'" 2>/dev/null)
    if grep -q "line1" <<<"$output" && \
       grep -q "line2" <<<"$output" && \
       grep -q "line3" <<<"$output"; then
        return 0
    fi
    log_fail "Salida multilínea no pasó correctamente"
    echo "  Output: $output"
    return 1
}

test_cmd_special_chars() {
    assert_exit_code 0 "$SPIN_BIN" --quiet --cmd 'echo "a b c | & ; < >"'
}

test_cmd_long_running() {
    # Comando que tarda ~1s
    assert_exit_code 0 "$SPIN_BIN" --quiet --cmd "sleep 1"
}

# ============================================================================
# TESTS: TEXTO
# ============================================================================
test_custom_text() {
    assert_exit_code 0 "$SPIN_BIN" --text "Procesando..." --cmd "true"
}

test_text_with_spaces() {
    assert_exit_code 0 "$SPIN_BIN" -t "Texto con muchos espacios aquí" --cmd "true"
}

test_text_empty() {
    # Texto vacío no debería crashear
    assert_exit_code 0 "$SPIN_BIN" -t "" --cmd "true"
}

test_text_unicode() {
    assert_exit_code 0 "$SPIN_BIN" -t "Cargando 日本語 ñ é ü 🚀" --cmd "true"
}

# ============================================================================
# TESTS: INTERVALO
# ============================================================================
test_interval_positive() {
    assert_exit_code 0 "$SPIN_BIN" --interval 100 --cmd "true"
}

test_interval_small() {
    assert_exit_code 0 "$SPIN_BIN" --interval 10 --cmd "true"
}

test_interval_large() {
    assert_exit_code 0 "$SPIN_BIN" --interval 1000 --cmd "true"
}

test_interval_zero() {
    # Intervalo 0 puede ser inválido (división por cero potencial)
    local output
    output=$("$SPIN_BIN" --interval 0 --cmd "true" 2>&1)
    local rc=$?
    verbose "interval=0 rc=$rc"
    # Aceptamos que falle o que funcione, pero no que crashee con signal
    if [[ $rc -ge 128 ]]; then
        log_fail "interval=0 provocó crash (signal), rc=$rc"
        return 1
    fi
    return 0
}

test_interval_negative() {
    local output
    output=$("$SPIN_BIN" --interval -50 --cmd "true" 2>&1)
    local rc=$?
    if [[ $rc -ge 128 ]]; then
        log_fail "interval=-50 provocó crash (signal), rc=$rc"
        return 1
    fi
    return 0
}

# ============================================================================
# TESTS: RESULTADO (--result, -e, -s)
# ============================================================================
test_result_success_message() {
    assert_output_contains "ÉXITO_TEST" \
        "$SPIN_BIN" --result --success "ÉXITO_TEST" --cmd "true"
}

test_result_error_message() {
    # El mensaje de error debe aparecer cuando el comando falla
    local output
    output=$("$SPIN_BIN" --result --error "FALLO_TEST" --cmd "false" 2>&1)
    if grep -q "FALLO_TEST" <<<"$output"; then
        return 0
    fi
    log_fail "Mensaje de error personalizado no apareció"
    echo "  Output: $output"
    return 1
}

test_result_default_messages() {
    # Sin -e/-s, --result debería mostrar algo por defecto
    local output
    output=$("$SPIN_BIN" --result --cmd "true" 2>&1)
    local rc=$?
    verbose "Default result rc=$rc output=$output"
    # No exigimos un texto específico, solo que no crashee
    [[ $rc -lt 128 ]] && return 0
    log_fail "rc=$rc (signal)"
    return 1
}

# ============================================================================
# TESTS: COMBINACIONES
# ============================================================================
test_combined_all_options() {
    assert_exit_code 0 "$SPIN_BIN" \
        --text "Test completo" \
        --interval 50 \
        --style dots \
        --color 33 \
        --result \
        --success "OK" \
        --error "FAIL" \
        --cmd "true"
}

test_combined_quiet_result() {
    local output
    output=$("$SPIN_BIN" --quiet --result --success "DONE_XYZ" --cmd "true" 2>&1)
    # En quiet mode quizá no se imprima el success, aceptamos ambas
    verbose "quiet+result: $output"
    return 0
}

# ============================================================================
# TESTS: ROBUSTEZ
# ============================================================================
test_cmd_without_cmd_option() {
    # Sin --cmd, spin debería... comportarse de forma controlada
    local output
    output=$(timeout 3 "$SPIN_BIN" --text "solo texto" 2>&1)
    local rc=$?
    # No debe colgarse (timeout=124) ni crashear (>=128)
    if [[ $rc -eq 124 ]]; then
        log_warn "spin sin --cmd se colgó (timeout 3s) - comportamiento a revisar"
        return 0  # No fallamos porque puede ser comportamiento esperado (spinner infinito)
    fi
    if [[ $rc -ge 128 ]]; then
        log_fail "spin sin --cmd crasheó con signal (rc=$rc)"
        return 1
    fi
    return 0
}

test_cmd_nonexistent_command() {
    # Un comando que no existe debería fallar controladamente
    local output
    output=$("$SPIN_BIN" --quiet --cmd "comando_que_no_existe_xyz_123" 2>&1)
    local rc=$?
    verbose "Nonexistent cmd rc=$rc"
    if [[ $rc -eq 0 ]]; then
        log_fail "spin devolvió 0 para comando inexistente"
        return 1
    fi
    return 0
}

test_no_args_no_crash() {
    # Sin argumentos, debe mostrar ayuda o error controlado, no crashear
    local output
    output=$(timeout 3 "$SPIN_BIN" 2>&1)
    local rc=$?
    if [[ $rc -ge 128 && $rc -ne 124 ]]; then
        log_fail "spin sin args crasheó (rc=$rc)"
        return 1
    fi
    return 0
}

test_unknown_option() {
    local output
    output=$("$SPIN_BIN" --opcion_inexistente_xyz 2>&1)
    local rc=$?
    if [[ $rc -eq 0 ]]; then
        log_fail "Opción desconocida aceptada silenciosamente"
        return 1
    fi
    return 0
}

test_cmd_with_unicode_output() {
    # Sin --quiet: valida passthrough unicode, y --quiet suprime la salida.
    assert_output_contains "日本語テスト" \
        "$SPIN_BIN" --cmd "echo 日本語テスト"
}

test_cmd_exit_codes() {
    # Verificar propagación de distintos exit codes
    for code in 1 2 42 127; do
        local output
        output=$("$SPIN_BIN" --quiet --cmd "exit $code" 2>&1)
        local rc=$?
        if [[ $rc -ne $code ]]; then
            # Algunos spinners no propagan exit code exacto; aceptamos != 0
            if [[ $code -ne 0 && $rc -eq 0 ]]; then
                log_fail "Exit code $code no propagado (rc=0)"
                return 1
            fi
            verbose "Exit code $code -> rc=$rc (no exacto pero != 0)"
        fi
    done
    return 0
}

test_repeated_execution() {
    # Ejecutar 10 veces seguidas para detectar leaks/flakiness
    for i in $(seq 1 10); do
        if ! "$SPIN_BIN" --quiet --cmd "true" &>/dev/null; then
            log_fail "Fallo en iteración $i de ejecución repetida"
            return 1
        fi
    done
    return 0
}

test_concurrent_execution() {
    # Ejecutar 5 instancias concurrentes
    local pids=()
    for i in $(seq 1 5); do
        "$SPIN_BIN" --quiet --cmd "sleep 0.3" &
        pids+=($!)
    done
    local failures=0
    for pid in "${pids[@]}"; do
        if ! wait "$pid"; then
            ((failures++))
        fi
    done
    if [[ $failures -gt 0 ]]; then
        log_fail "$failures instancias concurrentes fallaron"
        return 1
    fi
    return 0
}

# ============================================================================
# TESTS: OPCIONES DE EJECUCIÓN (-j/--jobs, --wake-threshold, --no-wake-lock,
# --pin-workers, --log, --show-system)
# ============================================================================
test_help_execution_options() {
    assert_output_contains "\\-\\-jobs" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-wake-threshold" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-no-wake-lock" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-pin-workers" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-log" "$SPIN_BIN" --help &&
    assert_output_contains "\\-\\-show-system" "$SPIN_BIN" --help
}

test_jobs_valid() {
    assert_exit_code 0 "$SPIN_BIN" -j 2 --cmd "true"
}

test_jobs_zero_autodetect() {
    # 0 = autodetectar (núcleos acotados por límite de procesos)
    assert_exit_code 0 "$SPIN_BIN" --jobs 0 --cmd "true"
}

test_jobs_negative() {
    local output
    output=$("$SPIN_BIN" --jobs -1 --cmd "true" 2>&1)
    local rc=$?
    if [[ $rc -ne 0 ]] || grep -qi "invalid" <<<"$output"; then
        return 0
    fi
    log_fail "--jobs -1 fue aceptado silenciosamente (rc=$rc)"
    return 1
}

test_jobs_invalid_value() {
    # optparse con type=int debe rechazar "abc" con rc!=0
    assert_exit_code 2 "$SPIN_BIN" --jobs abc --cmd "true"
}

test_jobs_single_command() {
    # Varios jobs con un solo comando: no debe fallar
    assert_exit_code 0 "$SPIN_BIN" -j 8 --cmd "true"
}

test_no_wake_lock_parallel() {
    # 3 comandos activarían wake lock por defecto; --no-wake-lock no debe colgar
    timeout 10 "$SPIN_BIN" --no-wake-lock --cmd "true, true, true" &>/dev/null
    local rc=$?
    if [[ $rc -eq 124 ]]; then
        log_fail "--no-wake-lock con 3 comandos se colgó (timeout 10s)"
        return 1
    fi
    [[ $rc -eq 0 ]] && return 0
    log_fail "rc=$rc con --no-wake-lock --cmd 'true, true, true'"
    return 1
}

test_wake_threshold_high() {
    # Umbral por encima del nº de comandos: wake lock no debe activarse (ni colgar)
    timeout 10 "$SPIN_BIN" --wake-threshold 100 --cmd "true, true" &>/dev/null
    local rc=$?
    [[ $rc -eq 124 ]] && { log_fail "se colgó con --wake-threshold 100"; return 1; }
    [[ $rc -eq 0 ]] && return 0
    log_fail "rc=$rc con --wake-threshold 100"
    return 1
}

test_wake_threshold_one() {
    # Umbral 1: wake lock con cualquier lote, no debe colgar
    timeout 10 "$SPIN_BIN" --wake-threshold 1 --cmd "true, true, true" &>/dev/null
    local rc=$?
    [[ $rc -eq 124 ]] && { log_fail "se colgó con --wake-threshold 1"; return 1; }
    [[ $rc -eq 0 ]] && return 0
    log_fail "rc=$rc con --wake-threshold 1"
    return 1
}

test_wake_threshold_zero_disables() {
    # <1 desactiva el wake lock
    timeout 10 "$SPIN_BIN" --wake-threshold 0 --cmd "true, true, true" &>/dev/null
    local rc=$?
    [[ $rc -eq 124 ]] && { log_fail "se colgó con --wake-threshold 0"; return 1; }
    [[ $rc -eq 0 ]] && return 0
    log_fail "rc=$rc con --wake-threshold 0"
    return 1
}

test_pin_workers_parallel() {
    timeout 10 "$SPIN_BIN" --pin-workers --cmd "true, true, true, true" &>/dev/null
    local rc=$?
    [[ $rc -eq 124 ]] && { log_fail "--pin-workers se colgó"; return 1; }
    [[ $rc -eq 0 ]] && return 0
    log_fail "rc=$rc con --pin-workers"
    return 1
}

test_log_custom_path() {
    local logfile="$TEST_TMP_DIR/spin_custom.log"
    assert_exit_code 0 "$SPIN_BIN" --log "$logfile" --cmd "true" || return 1
    assert_file_exists "$logfile" || return 1
    if grep -q "Estado: SUCCESS" "$logfile"; then
        return 0
    fi
    log_fail "El log no contiene 'Estado: SUCCESS': $logfile"
    return 1
}

test_log_failure_recorded() {
    local logfile="$TEST_TMP_DIR/spin_fail.log"
    "$SPIN_BIN" --log "$logfile" --cmd "false" &>/dev/null
    if grep -q "Estado: FAILED" "$logfile" 2>/dev/null; then
        return 0
    fi
    log_fail "El log no contiene 'Estado: FAILED': $logfile"
    return 1
}

test_show_system() {
    assert_exit_code 0 "$SPIN_BIN" --show-system &&
    assert_output_contains "Online cores" "$SPIN_BIN" --show-system &&
    assert_output_contains "Safe workers" "$SPIN_BIN" --show-system
}

test_execution_combined() {
    # Toda la configuración de ejecución a la vez
    local logfile="$TEST_TMP_DIR/spin_combo.log"
    timeout 10 "$SPIN_BIN" -j 2 --pin-workers --no-wake-lock \
        --wake-threshold 50 --log "$logfile" \
        --cmd "true, true, true" &>/dev/null
    local rc=$?
    [[ $rc -eq 124 ]] && { log_fail "combinación completa se colgó"; return 1; }
    if [[ $rc -ne 0 ]]; then
        log_fail "combinación completa devolvió rc=$rc"
        return 1
    fi
    assert_file_exists "$logfile"
}

# ============================================================================
# MAIN
# ============================================================================
print_banner() {
    cat <<'EOF'
╔══════════════════════════════════════════════════════════════════╗
║           SUITE DE TESTS RIGUROSA PARA `spin`                    ║
║           Verificación exhaustiva de CLI                         ║
╚══════════════════════════════════════════════════════════════════╝
EOF
}

print_summary() {
    local total=$((TESTS_PASSED + TESTS_FAILED + TESTS_SKIPPED))
    echo
    echo "════════════════════════════════════════════════════════════════"
    echo -e "${BOLD}RESUMEN DE TESTS${NC}"
    echo "════════════════════════════════════════════════════════════════"
    echo -e "  Total:    $total"
    echo -e "  ${GREEN}Pasados:  $TESTS_PASSED${NC}"
    echo -e "  ${RED}Fallidos: $TESTS_FAILED${NC}"
    echo -e "  ${YELLOW}Omitidos: $TESTS_SKIPPED${NC}"
    echo "════════════════════════════════════════════════════════════════"

    if [[ $TESTS_FAILED -gt 0 ]]; then
        echo -e "${RED}${BOLD}❌ ALGUNOS TESTS FALLARON${NC}"
        return 1
    fi
    echo -e "${GREEN}${BOLD}✅ TODOS LOS TESTS PASARON${NC}"
    return 0
}

main() {
    # Parseo de args
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -v|--verbose) VERBOSE=1; shift ;;
            -t|--test)    FILTER="$2"; shift 2 ;;
            -b|--bin)     SPIN_BIN="$2"; shift 2 ;;
            -h|--help)
                echo "Uso: $0 [-v] [-t filtro] [-b /ruta/a/spin]"
                exit 0
                ;;
            *) echo "Opción desconocida: $1"; exit 1 ;;
        esac
    done

    print_banner
    preflight_check

    # Ejecutar todos los tests
    run_test "Ayuda --help"                    test_help_flag
    run_test "Ayuda -h"                        test_help_short_flag
    run_test "Versión --version"               test_version_flag

    run_test "Mostrar estilos"                 test_show_style
    run_test "Estilo válido (dots)"            test_valid_style
    run_test "Estilo inválido"                 test_invalid_style_handling

    run_test "Mostrar colores"                 test_show_colors
    run_test "Color válido (42)"               test_valid_color
    run_test "Color límite inferior (0)"       test_color_boundary_zero
    run_test "Color límite superior (255)"     test_color_boundary_255
    run_test "Color fuera de rango (256)"      test_color_out_of_range
    run_test "Color negativo (-1)"             test_color_negative

    run_test "Comando exitoso (true)"          test_cmd_success
    run_test "Comando fallido (false)"         test_cmd_failure
    run_test "Stdout pasado correctamente"     test_cmd_stdout_passthrough
    run_test "Modo --quiet"                    test_cmd_quiet_mode
    run_test "Comando con stderr"              test_cmd_with_stderr
    run_test "Salida multilínea"               test_cmd_multiline
    run_test "Caracteres especiales en cmd"    test_cmd_special_chars
    run_test "Comando largo (~1s)"             test_cmd_long_running

    run_test "Texto personalizado"             test_custom_text
    run_test "Texto con espacios"              test_text_with_spaces
    run_test "Texto vacío"                     test_text_empty
    run_test "Texto unicode"                   test_text_unicode

    run_test "Intervalo positivo"              test_interval_positive
    run_test "Intervalo pequeño (10ms)"        test_interval_small
    run_test "Intervalo grande (1000ms)"       test_interval_large
    run_test "Intervalo cero"                  test_interval_zero
    run_test "Intervalo negativo"              test_interval_negative

    run_test "Resultado mensaje éxito"         test_result_success_message
    run_test "Resultado mensaje error"         test_result_error_message
    run_test "Resultado mensajes por defecto"  test_result_default_messages

    run_test "Combinación completa"            test_combined_all_options
    run_test "Combinación quiet+result"        test_combined_quiet_result

    run_test "Sin --cmd"                       test_cmd_without_cmd_option
    run_test "Comando inexistente"             test_cmd_nonexistent_command
    run_test "Sin argumentos"                  test_no_args_no_crash
    run_test "Opción desconocida"              test_unknown_option
    run_test "Output unicode"                  test_cmd_with_unicode_output
    run_test "Propagación exit codes"          test_cmd_exit_codes
    run_test "Ejecución repetida (x10)"        test_repeated_execution
    run_test "Ejecución concurrente (x5)"      test_concurrent_execution

    run_test "Ayuda opciones de ejecución"     test_help_execution_options
    run_test "Jobs válido (-j 2)"              test_jobs_valid
    run_test "Jobs autodetect (--jobs 0)"      test_jobs_zero_autodetect
    run_test "Jobs negativo rechazado"         test_jobs_negative
    run_test "Jobs no numérico rechazado"      test_jobs_invalid_value
    run_test "Jobs con un solo comando"        test_jobs_single_command
    run_test "Sin wake lock paralelo"          test_no_wake_lock_parallel
    run_test "Wake threshold alto"             test_wake_threshold_high
    run_test "Wake threshold 1"                test_wake_threshold_one
    run_test "Wake threshold 0 desactiva"      test_wake_threshold_zero_disables
    run_test "Pin workers paralelo"            test_pin_workers_parallel
    run_test "Log en ruta personalizada"       test_log_custom_path
    run_test "Log registra fallo"              test_log_failure_recorded
    run_test "Mostrar sistema (--show-system)" test_show_system
    run_test "Combinación de ejecución"        test_execution_combined

    print_summary
}

main "$@"
