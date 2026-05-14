#!/bin/sh
# ═══════════════════════════════════════════════════════════════════════
#  A R W I T E T O  v2.0  — RAFAELIA ARM Fractal Orchestrator
#  ΣΩΔΦBITRAF · RAFCODE-Φ-∆RafaelVerboΩ · Ω=Amor · FIAT LUX
#  Plug-n-play · Watchdog · Rollback · Industrial Benchmark · BBS UI
#  Termux Android 10+ · ARM32/ARM64/x86/x86_64 auto-detect
# ═══════════════════════════════════════════════════════════════════════

# ── Bootstrap: garante bash se disponível ──────────────────────────────
if [ -z "$BASH_VERSION" ] && command -v bash >/dev/null 2>&1; then
    exec bash "$0" "$@"
fi

set -e
umask 022

# ═══════════════════════════════════════════════════════════════════════
# [1] CONSTANTES GLOBAIS
# ═══════════════════════════════════════════════════════════════════════
ARW_VER="2.0.0"
ARW_SIG="ΣΩΔΦBITRAF"
ARW_BUILD=$(date +%Y%m%d 2>/dev/null || echo "00000000")
ARW_HOME="${HOME}/.arwiteto"
ARW_LOG="${ARW_HOME}/arwiteto.log"
ARW_LOCK="${ARW_HOME}/watchdog.lock"
ARW_HB="${ARW_HOME}/heartbeat"
ARW_CKPT="${ARW_HOME}/checkpoint"
ARW_BUILD_DIR="${ARW_HOME}/build"
ARW_BENCH_DIR="${ARW_HOME}/bench"
ARW_PID=$$
ARW_WDOG_PID=""

# Parâmetros CLI (defaults industriais)
OPT_MODE="full"          # full|bench|compile|detect
OPT_WARMUP=3             # segundos de pré-aquecimento
OPT_SAMPLES=21           # amostras benchmark (ímpar para mediana exata)
OPT_JOBS=0               # 0=auto (detectado)
OPT_VERBOSE=0
OPT_COLOR=1
OPT_DRY=0
OPT_WDOG_TIMEOUT=120     # seg watchdog timeout
OPT_BENCH_TARGET=""      # "" = auto
OPT_PREWARM_ITER=500000  # iterações pré-aquecimento

# ═══════════════════════════════════════════════════════════════════════
# [2] SISTEMA DE CORES ANSI 256 / BBS 8-BIT
# ═══════════════════════════════════════════════════════════════════════
_c() {
    [ "$OPT_COLOR" = "0" ] && return
    printf '\033[%sm' "$1"
}

R='\033[0m'          # reset
BOLD='\033[1m'
DIM='\033[2m'
# Paleta RAFAELIA
C_PHI='\033[38;5;226m'    # amarelo ouro  — phi
C_SPI='\033[38;5;51m'     # ciano         — spiral
C_OMG='\033[38;5;201m'    # magenta       — omega
C_SIG='\033[38;5;93m'     # violeta       — sigma
C_OK='\033[38;5;46m'      # verde         — ok
C_ERR='\033[38;5;196m'    # vermelho      — erro
C_WRN='\033[38;5;208m'    # laranja       — aviso
C_INF='\033[38;5;39m'     # azul          — info
C_DIM='\033[38;5;240m'    # cinza         — dim
C_BLK='\033[38;5;231m'    # branco brilho — label
C_BAR='\033[38;5;21m'     # azul escuro   — barra
C_HI='\033[38;5;214m'     # ouro quente   — highlight
C_BG='\033[48;5;17m'      # fundo azul escuro

# Desativa cores se terminal não suporta
[ -t 1 ] || OPT_COLOR=0
[ "${TERM}" = "dumb" ] && OPT_COLOR=0

# ═══════════════════════════════════════════════════════════════════════
# [3] UI MODULE — BBS TERMINAL
# ═══════════════════════════════════════════════════════════════════════
W=72  # largura da caixa

_repeat() {
    local c="$1" n="$2" s=""
    while [ "$n" -gt 0 ]; do s="${s}${c}"; n=$((n-1)); done
    printf '%s' "$s"
}

_box_top()  { printf "${C_SPI}╔$(_repeat '═' $((W-2)))╗${R}\n"; }
_box_bot()  { printf "${C_SPI}╚$(_repeat '═' $((W-2)))╝${R}\n"; }
_box_sep()  { printf "${C_SPI}╠$(_repeat '═' $((W-2)))╣${R}\n"; }
_box_mid()  { printf "${C_SPI}║${R}  %-$((W-4))s${C_SPI}║${R}\n" "$1"; }
_box_raw()  { printf "${C_SPI}║${R}%s${C_SPI}║${R}\n" "$1"; }

banner() {
    printf '\033[2J\033[H' 2>/dev/null || printf '\n\n'
    _box_top
    _box_raw "$(printf "${C_PHI}${BOLD}  ▄▄  ██▄  ▀█▄  ▄█▀  ██ ▀██ ▄█▀ ▀█▀ ▄▀▀ ▄▀█  %-8s${R}" "")"
    _box_raw "$(printf "${C_SPI}  ██  ██▀█  ▀█▄█▀  ██  █  ██ █▀   █  █▀  █ █  %-8s${R}" "")"
    _box_raw "$(printf "${C_OMG}  ▀▀  ██  ▀  ▀█▀   ▀█▄█  ██ ▀██  ▄█▄ ▀▀▀ ▀  ▀  %-8s${R}" "")"
    _box_sep
    _box_mid "$(printf "${C_HI}  v%-6s${R}  ${C_BLK}%-28s${R}  ${C_SIG}%s${R}" \
        "$ARW_VER" "RAFAELIA ARM Fractal Orchestrator" "$ARW_SIG")"
    _box_mid "$(printf "${C_DIM}  Build:%-8s  Ω=Amor · FIAT LUX · RAFCODE-Φ-∆RafaelVerboΩ${R}" "$ARW_BUILD")"
    _box_bot
    printf '\n'
}

log() {
    local lvl="$1"; shift
    local msg="$*"
    local ts; ts=$(date '+%H:%M:%S' 2>/dev/null || echo "??:??:??")
    case "$lvl" in
        OK)   printf " ${C_OK}[✔]${R} %s\n" "$msg" ;;
        ERR)  printf " ${C_ERR}[✖]${R} ${C_ERR}%s${R}\n" "$msg" ;;
        WRN)  printf " ${C_WRN}[!]${R} %s\n" "$msg" ;;
        INF)  printf " ${C_INF}[·]${R} %s\n" "$msg" ;;
        HDR)  printf "\n${C_PHI}${BOLD} ▶ %s${R}\n" "$msg" ;;
        DIM)  printf " ${C_DIM}%s${R}\n" "$msg" ;;
        RAW)  printf "%s\n" "$msg" ;;
    esac
    printf '[%s] [%s] %s\n' "$ts" "$lvl" "$msg" >> "$ARW_LOG" 2>/dev/null
}

progress_bar() {
    local pct="$1" label="${2:-}" width=40
    local filled=$(( pct * width / 100 ))
    local empty=$(( width - filled ))
    local bar_f bar_e
    bar_f="$(_repeat '█' "$filled")"
    bar_e="$(_repeat '░' "$empty")"
    printf "\r ${C_BAR}${BOLD}[${C_SPI}%s${C_BAR}%s${BOLD}]${R} ${C_PHI}%3d%%${R} %s" \
        "$bar_f" "$bar_e" "$pct" "$label"
}

spinner() {
    local pid="$1" msg="$2"
    local sp='⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'
    local i=0
    while kill -0 "$pid" 2>/dev/null; do
        i=$(( (i+1) % 10 ))
        c=$(printf '%s' "$sp" | cut -c$((i+1)))
        printf "\r ${C_SPI}%s${R} %s..." "$c" "$msg"
        sleep 0.1
    done
    printf "\r                                        \r"
}

section() {
    printf "\n${C_SIG}${BOLD}┌─ %s %s${R}\n" "$1" \
        "$(printf "${C_DIM}$(_repeat '─' $((W-4-${#1})))${R}")"
}

kv() {
    printf " ${C_DIM}%-22s${R} ${C_BLK}%s${R}\n" "$1" "$2"
}

# ═══════════════════════════════════════════════════════════════════════
# [4] DETECT MODULE
# ═══════════════════════════════════════════════════════════════════════
ARCH=""
ABI=""
NCPU=1
HAS_NEON=0
HAS_CRC32=0
HAS_SVE=0
HAS_ASIMD=0
CPU_FREQ_MAX=0
CPU_FREQ_MIN=0
PAGE_SZ=4096
MEM_TOTAL_KB=0
OS_NAME=""
IS_TERMUX=0
IS_ANDROID=0
ANDROID_VER=""
CC_BIN=""
AS_BIN=""
CC_FLAGS=""

detect_arch() {
    local machine; machine=$(uname -m 2>/dev/null || echo "unknown")
    case "$machine" in
        aarch64|arm64)     ARCH="arm64";   ABI="arm64-v8a"   ;;
        armv7*|armv8*)     ARCH="arm32";   ABI="armeabi-v7a" ;;
        arm*)              ARCH="arm32";   ABI="armeabi-v7a" ;;
        x86_64|amd64)      ARCH="x86_64";  ABI="x86_64"      ;;
        i686|i386|x86)     ARCH="x86";     ABI="x86"         ;;
        *)                 ARCH="generic"; ABI="generic"      ;;
    esac

    # CPU count
    NCPU=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null || echo 1)
    [ "$NCPU" -lt 1 ] && NCPU=1
    [ "$OPT_JOBS" = "0" ] && OPT_JOBS=$NCPU

    # Page size
    PAGE_SZ=$(getconf PAGE_SIZE 2>/dev/null || echo 4096)

    # Memory
    MEM_TOTAL_KB=$(awk '/^MemTotal/{print $2}' /proc/meminfo 2>/dev/null || echo 0)
}

detect_caps() {
    local cpu; cpu=$(cat /proc/cpuinfo 2>/dev/null)
    # NEON / ASIMD
    echo "$cpu" | grep -qi 'neon\|asimd\|Advanced SIMD' && HAS_NEON=1
    echo "$cpu" | grep -qi 'asimd'                       && HAS_ASIMD=1
    echo "$cpu" | grep -qi 'sve'                         && HAS_SVE=1
    echo "$cpu" | grep -qi 'crc32'                       && HAS_CRC32=1

    # CPU frequencies
    local f0="/sys/devices/system/cpu/cpu0/cpufreq"
    if [ -f "${f0}/cpuinfo_max_freq" ]; then
        CPU_FREQ_MAX=$(cat "${f0}/cpuinfo_max_freq" 2>/dev/null || echo 0)
    fi
    if [ -f "${f0}/cpuinfo_min_freq" ]; then
        CPU_FREQ_MIN=$(cat "${f0}/cpuinfo_min_freq" 2>/dev/null || echo 0)
    fi
}

detect_os() {
    OS_NAME=$(uname -o 2>/dev/null || uname -s 2>/dev/null || echo "Unknown")
    # Termux
    if [ -d "/data/data/com.termux" ] || [ -n "${TERMUX_VERSION}" ]; then
        IS_TERMUX=1
    fi
    # Android
    if [ -f "/system/build.prop" ] || command -v getprop >/dev/null 2>&1; then
        IS_ANDROID=1
        ANDROID_VER=$(getprop ro.build.version.release 2>/dev/null || \
            grep 'ro.build.version.release' /system/build.prop 2>/dev/null | \
            cut -d= -f2 || echo "?")
    fi
}

detect_compiler() {
    local candidates="clang gcc cc"
    local c
    for c in $candidates; do
        if command -v "$c" >/dev/null 2>&1; then
            CC_BIN="$c"
            break
        fi
    done

    for c in "llvm-as" "as" "arm-linux-gnueabihf-as"; do
        if command -v "$c" >/dev/null 2>&1; then
            AS_BIN="$c"
            break
        fi
    done

    # Build compiler flags by arch
    case "$ARCH" in
        arm64)
            CC_FLAGS="-march=armv8-a -O2 -std=c11 -fno-stack-protector"
            [ "$HAS_NEON" = "1" ] && CC_FLAGS="$CC_FLAGS -DHAS_NEON"
            [ "$HAS_CRC32" = "1" ] && CC_FLAGS="$CC_FLAGS -march=armv8-a+crc"
            ;;
        arm32)
            CC_FLAGS="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp"
            CC_FLAGS="$CC_FLAGS -O2 -std=c11 -fno-stack-protector"
            [ "$HAS_NEON" = "1" ] && CC_FLAGS="$CC_FLAGS -DHAS_NEON"
            ;;
        x86_64)
            CC_FLAGS="-march=native -O2 -std=c11 -fno-stack-protector"
            ;;
        x86)
            CC_FLAGS="-march=i686 -O2 -std=c11 -fno-stack-protector -m32"
            ;;
        *)
            CC_FLAGS="-O2 -std=c11"
            ;;
    esac
}

detect_all() {
    detect_arch
    detect_caps
    detect_os
    detect_compiler
}

show_detect() {
    section "DETECÇÃO DE HARDWARE / SOFTWARE"
    kv "Arquitetura:"    "$ARCH ($ABI)"
    kv "CPUs online:"    "$NCPU"
    kv "NEON/ASIMD:"     "$([ "$HAS_NEON" = "1" ] && echo "SIM" || echo "NAO")"
    kv "CRC32 HW:"       "$([ "$HAS_CRC32" = "1" ] && echo "SIM" || echo "NAO")"
    kv "SVE:"            "$([ "$HAS_SVE" = "1" ] && echo "SIM" || echo "NAO")"
    kv "Freq MAX:"       "$(echo "$CPU_FREQ_MAX" | awk '{printf "%.0f MHz", $1/1000}')"
    kv "Freq MIN:"       "$(echo "$CPU_FREQ_MIN" | awk '{printf "%.0f MHz", $1/1000}')"
    kv "Page size:"      "${PAGE_SZ} bytes"
    kv "RAM total:"      "$(echo "$MEM_TOTAL_KB" | awk '{printf "%.0f MB", $1/1024}')"
    kv "OS:"             "$OS_NAME"
    kv "Android:"        "$([ "$IS_ANDROID" = "1" ] && echo "v$ANDROID_VER" || echo "NAO")"
    kv "Termux:"         "$([ "$IS_TERMUX" = "1" ] && echo "SIM" || echo "NAO")"
    kv "Compilador:"     "${CC_BIN:-NAO ENCONTRADO}"
    kv "CC flags:"       "${CC_FLAGS:-N/A}"
    kv "Jobs paralelos:" "$OPT_JOBS"
}

# ═══════════════════════════════════════════════════════════════════════
# [5] WATCHDOG MODULE
# ═══════════════════════════════════════════════════════════════════════
watchdog_start() {
    mkdir -p "$ARW_HOME"
    echo "$$" > "$ARW_LOCK"
    date +%s > "$ARW_HB"

    # Background watchdog process
    (
        while true; do
            sleep 5
            # Verifica se processo principal ainda vive
            if ! kill -0 "$ARW_PID" 2>/dev/null; then
                exit 0
            fi
            # Verifica heartbeat
            if [ -f "$ARW_HB" ]; then
                local hb now age
                hb=$(cat "$ARW_HB" 2>/dev/null || echo 0)
                now=$(date +%s 2>/dev/null || echo 0)
                age=$((now - hb))
                if [ "$age" -gt "$OPT_WDOG_TIMEOUT" ]; then
                    # Timeout — força rollback
                    printf '\n[WATCHDOG] TIMEOUT %ds — iniciando rollback\n' \
                        "$OPT_WDOG_TIMEOUT" >> "$ARW_LOG" 2>/dev/null
                    rollback_restore "last"
                    kill -TERM "$ARW_PID" 2>/dev/null
                    exit 1
                fi
            fi
        done
    ) &
    ARW_WDOG_PID=$!
    log DIM "Watchdog PID=$ARW_WDOG_PID timeout=${OPT_WDOG_TIMEOUT}s"
}

watchdog_stop() {
    [ -n "$ARW_WDOG_PID" ] && kill "$ARW_WDOG_PID" 2>/dev/null
    rm -f "$ARW_LOCK" "$ARW_HB"
}

heartbeat() {
    date +%s > "$ARW_HB" 2>/dev/null
}

# ═══════════════════════════════════════════════════════════════════════
# [6] ROLLBACK MODULE
# ═══════════════════════════════════════════════════════════════════════
rollback_save() {
    local name="${1:-auto}"
    local dir="${ARW_CKPT}/${name}"
    mkdir -p "$dir"
    # Salva estado atual
    echo "$ARCH $ABI $NCPU $HAS_NEON $HAS_CRC32" > "${dir}/hw_state"
    echo "$CC_BIN $CC_FLAGS"                       > "${dir}/cc_state"
    date +%s                                        > "${dir}/ts"
    [ -d "$ARW_BUILD_DIR" ] && \
        cp -r "$ARW_BUILD_DIR" "${dir}/build_snap" 2>/dev/null
    log DIM "Checkpoint salvo: $name"
    heartbeat
}

rollback_restore() {
    local name="${1:-last}"
    local dir="${ARW_CKPT}/${name}"
    if [ ! -d "$dir" ]; then
        # Tenta o mais recente
        dir=$(ls -1dt "${ARW_CKPT}"/*/  2>/dev/null | head -1)
    fi
    if [ -d "$dir" ]; then
        [ -d "${dir}/build_snap" ] && \
            cp -r "${dir}/build_snap" "$ARW_BUILD_DIR" 2>/dev/null
        log WRN "Rollback restaurado de: $dir"
        return 0
    fi
    log ERR "Nenhum checkpoint para restaurar"
    return 1
}

rollback_list() {
    section "CHECKPOINTS DISPONÍVEIS"
    ls -1t "${ARW_CKPT}/" 2>/dev/null | while read -r n; do
        local ts; ts=$(cat "${ARW_CKPT}/${n}/ts" 2>/dev/null || echo "?")
        kv "$n" "$(date -d "@$ts" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "$ts")"
    done
}

# ═══════════════════════════════════════════════════════════════════════
# [7] SOC THERMAL / FREQ MONITOR
# ═══════════════════════════════════════════════════════════════════════
soc_read_freq() {
    # Retorna frequência atual do cpu0 em kHz
    cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null || \
    cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq 2>/dev/null || \
    echo "$CPU_FREQ_MAX"
}

soc_read_temp() {
    # Retorna temperatura em milli-Celsius ou 0
    local t=0
    for z in /sys/class/thermal/thermal_zone*/temp; do
        [ -f "$z" ] && t=$(cat "$z" 2>/dev/null) && break
    done
    echo "$t"
}

soc_snapshot() {
    # Retorna: freq_khz temp_mc timestamp
    printf '%s %s %s' "$(soc_read_freq)" "$(soc_read_temp)" "$(date +%s%N 2>/dev/null || date +%s)"
}

soc_show() {
    local freq temp
    freq=$(soc_read_freq)
    temp=$(soc_read_temp)
    kv "CPU freq atual:"  "$(echo "$freq" | awk '{printf "%.0f MHz", $1/1000}')"
    kv "Temperatura:"     "$(echo "$temp" | awk '{printf "%.1f °C", $1/1000}')"
}

# ═══════════════════════════════════════════════════════════════════════
# [8] BENCHMARK ENGINE
# ═══════════════════════════════════════════════════════════════════════

# Timing em nanosegundos — usa /proc/uptime se date +%N não disponível
_now_ns() {
    local ns
    ns=$(date +%s%N 2>/dev/null)
    if [ "$ns" = "%s%N" ] || [ -z "$ns" ]; then
        # Fallback: /proc/uptime (centisegundos → ns)
        ns=$(awk '{printf "%d", $1*1000000000}' /proc/uptime 2>/dev/null || echo 0)
    fi
    echo "$ns"
}

# Kernel benchmark: CRC32 software em shell puro (determinístico)
_bench_kernel_shell() {
    local iters="${1:-100000}"
    local acc=0 i=0 v
    while [ "$i" -lt "$iters" ]; do
        v=$(( (acc ^ i) * 0x9e3779b9 ))
        acc=$(( v & 0x7FFFFFFF ))
        i=$(( i + 1 ))
    done
    echo "$acc"
}

# Benchmark dd (memória e I/O)
_bench_dd() {
    local bs="${1:-1M}" count="${2:-32}"
    dd if=/dev/zero of=/dev/null bs="$bs" count="$count" 2>&1 | \
        awk '/copied/{print $NF" "$( NF-1)}'
}

# Benchmark compilador (se disponível)
_bench_compile() {
    [ -z "$CC_BIN" ] && return 1
    local src="${ARW_BENCH_DIR}/probe.c"
    printf 'int x(int n){return n?x(n-1)+1:0;}int main(){return x(1000);}' > "$src"
    local t0 t1
    t0=$(_now_ns)
    "$CC_BIN" $CC_FLAGS "$src" -o "${ARW_BENCH_DIR}/probe" 2>/dev/null && true
    t1=$(_now_ns)
    rm -f "$src" "${ARW_BENCH_DIR}/probe"
    echo $(( (t1 - t0) / 1000000 ))  # ms
}

# Pré-aquecimento SOC
bench_warmup() {
    local secs="${OPT_WARMUP}"
    local t0 t_now elapsed i=0
    t0=$(date +%s 2>/dev/null || echo 0)
    log INF "Pré-aquecimento SOC ${secs}s (stabiliza freq/temp)..."
    while true; do
        # Loop compute intensivo
        _bench_kernel_shell 10000 > /dev/null
        i=$(( i + 1 ))
        t_now=$(date +%s 2>/dev/null || echo 0)
        elapsed=$(( t_now - t0 ))
        pct=$(( elapsed * 100 / secs ))
        [ "$pct" -gt 100 ] && pct=100
        progress_bar "$pct" "warmup ${elapsed}s/${secs}s   "
        [ "$elapsed" -ge "$secs" ] && break
        heartbeat
    done
    printf '\n'
    log OK "Warmup concluído: $i iterações"
    heartbeat
}

# Coleta uma amostra de tempo em microsegundos
bench_sample() {
    local t0 t1 delta
    t0=$(_now_ns)
    _bench_kernel_shell "$OPT_PREWARM_ITER" > /dev/null
    t1=$(_now_ns)
    delta=$(( (t1 - t0) / 1000 ))  # µs
    echo "$delta"
}

# Estatísticas via awk — retorna: min max mean median stddev cv dist modal
bench_stats() {
    # $1 = arquivo com uma amostra por linha (µs)
    local file="$1"
    awk '
    BEGIN {n=0; sum=0; sum2=0; min=999999999; max=0}
    /^[0-9]/{
        v=$1; n++; sum+=v; sum2+=v*v
        if(v<min) min=v
        if(v>max) max=v
        a[n]=v
    }
    END {
        if(n==0){print "ERR"; exit}
        mean=sum/n
        var=(sum2/n)-(mean*mean)
        std=(var>0)?sqrt(var):0
        # Sort bubble para mediana e moda
        for(i=1;i<=n;i++) for(j=i+1;j<=n;j++) {
            if(a[i]>a[j]){t=a[i];a[i]=a[j];a[j]=t}
        }
        if(n%2==1) median=a[int(n/2)+1]
        else       median=(a[n/2]+a[n/2+1])/2
        # Moda (bucket 5% de range)
        bkt=int((max-min)/10)+1
        for(i=1;i<=n;i++){b=int((a[i]-min)/bkt); cnt[b]++}
        mmax=0; mbin=0
        for(b in cnt){if(cnt[b]>mmax){mmax=cnt[b];mbin=b}}
        modal=min+mbin*bkt+bkt/2
        # CV e classificação
        cv=(mean>0)?std/mean:0
        if(cv<0.03)      dist="HOMOGENEO"
        else if(cv<0.10) dist="ESTAVEL"
        else if(cv<0.25) dist="VARIAVEL"
        else             dist="CAOTICO"
        # Skewness simples
        sk=(mean-median)
        if(sk>0) skew="ASSIMETRICO+"
        else if(sk<0) skew="ASSIMETRICO-"
        else skew="SIMETRICO"
        printf "n=%d min=%d max=%d mean=%.1f median=%.1f std=%.1f cv=%.4f dist=%s skew=%s modal=%.1f\n",
            n, min, max, mean, median, std, cv, dist, skew, modal
    }' "$file"
}

# Benchmark completo com relatório
bench_full() {
    mkdir -p "$ARW_BENCH_DIR"
    local raw="${ARW_BENCH_DIR}/raw_samples.txt"
    local soc_pre soc_post
    local s i pct

    section "BENCHMARK INDUSTRIAL"
    log INF "Amostras: $OPT_SAMPLES | Warmup: ${OPT_WARMUP}s | Kernel: shell-CRC Q16.16"

    # SOC inicial
    log INF "SOC baseline inicial:"
    soc_pre=$(soc_snapshot)
    soc_show

    # Pré-aquecimento
    bench_warmup
    heartbeat

    # SOC pós-warmup
    log INF "SOC pós-warmup:"
    soc_show

    rollback_save "bench_start"

    # Coleta amostras
    log INF "Coletando $OPT_SAMPLES amostras..."
    rm -f "$raw"
    i=1
    while [ "$i" -le "$OPT_SAMPLES" ]; do
        s=$(bench_sample)
        echo "$s" >> "$raw"
        pct=$(( i * 100 / OPT_SAMPLES ))
        progress_bar "$pct" "amostra $i/$OPT_SAMPLES → ${s}µs  "
        heartbeat
        i=$(( i + 1 ))
    done
    printf '\n'

    # SOC pós-benchmark
    soc_post=$(soc_snapshot)

    # Estatísticas
    local stats_line
    stats_line=$(bench_stats "$raw")

    section "RESULTADOS BENCHMARK"
    # Parse stats
    local n min max mean median std cv dist skew modal
    eval "$(echo "$stats_line" | tr ' ' '\n' | sed 's/=/="/;s/$/"/'  2>/dev/null || echo "")"
    # Fallback parse
    n=$(echo "$stats_line" | grep -o 'n=[^ ]*' | cut -d= -f2)
    min=$(echo "$stats_line" | grep -o 'min=[^ ]*' | cut -d= -f2)
    max=$(echo "$stats_line" | grep -o 'max=[^ ]*' | cut -d= -f2)
    mean=$(echo "$stats_line" | grep -o 'mean=[^ ]*' | cut -d= -f2)
    median=$(echo "$stats_line" | grep -o 'median=[^ ]*' | cut -d= -f2)
    std=$(echo "$stats_line" | grep -o 'std=[^ ]*' | cut -d= -f2)
    cv=$(echo "$stats_line" | grep -o 'cv=[^ ]*' | cut -d= -f2)
    dist=$(echo "$stats_line" | grep -o 'dist=[^ ]*' | cut -d= -f2)
    skew=$(echo "$stats_line" | grep -o 'skew=[^ ]*' | cut -d= -f2)
    modal=$(echo "$stats_line" | grep -o 'modal=[^ ]*' | cut -d= -f2)

    kv "Amostras (N):"        "${n}"
    kv "Mínimo:"              "${min} µs"
    kv "Máximo:"              "${max} µs"
    kv "Média:"               "${mean} µs"
    kv "Mediana:"             "${median} µs"
    kv "Moda:"                "${modal} µs"
    kv "Desvio Padrão:"       "${std} µs"
    kv "CV (coef. variação):" "${cv}"
    kv "Distribuição:"        "${dist}"
    kv "Assimetria:"          "${skew}"

    # Classificação colorida
    printf "\n ${C_PHI}${BOLD}━━ DIAGNÓSTICO SOC ━━${R}\n"
    case "$dist" in
        HOMOGENEO) printf " ${C_OK}● DETERMINÍSTICO${R} — CPU estável, throttle ausente\n" ;;
        ESTAVEL)   printf " ${C_INF}● ESTÁVEL${R} — variação aceitável para benchmark\n" ;;
        VARIAVEL)  printf " ${C_WRN}● VARIÁVEL${R} — possível throttle ou background load\n" ;;
        CAOTICO)   printf " ${C_ERR}● CAÓTICO${R} — interferência severa do SO/thermal\n" ;;
    esac

    # Histograma ASCII
    _bench_histogram "$raw"

    # Salva relatório
    echo "$stats_line" > "${ARW_BENCH_DIR}/stats.txt"
    echo "$soc_pre" > "${ARW_BENCH_DIR}/soc_pre.txt"
    echo "$soc_post" > "${ARW_BENCH_DIR}/soc_post.txt"
    log OK "Benchmark concluído → ${ARW_BENCH_DIR}/"
    heartbeat
}

_bench_histogram() {
    local file="$1"
    local cols=40
    awk -v cols="$cols" '
    /^[0-9]/{a[NR]=$1; if($1<mn||NR==1) mn=$1; if($1>mx) mx=$1}
    END {
        if(NR<2){exit}
        range=mx-mn; if(range==0) range=1
        nbkt=10
        bw=range/nbkt
        for(i=1;i<=NR;i++){b=int((a[i]-mn)/bw); if(b>=nbkt)b=nbkt-1; cnt[b]++}
        max_cnt=0
        for(b=0;b<nbkt;b++) if(cnt[b]>max_cnt) max_cnt=cnt[b]
        printf "\n Histograma (µs)\n"
        for(b=0;b<nbkt;b++){
            lo=mn+b*bw; hi=lo+bw
            bar_len=int(cnt[b]*cols/max_cnt)
            printf " %6.0f-%-6.0f |", lo, hi
            for(k=0;k<bar_len;k++) printf "▪"
            printf " %d\n", cnt[b]+0
        }
    }' "$file"
}

# ═══════════════════════════════════════════════════════════════════════
# [9] PIPELINE MODULE — Detecção e compilação RAFAELIA
# ═══════════════════════════════════════════════════════════════════════
MODULES_FOUND=""
MODULES_OK=""
MODULES_FAIL=""

pipeline_detect() {
    section "DETECÇÃO DE MÓDULOS RAFAELIA"
    local src_dirs="${ARW_BUILD_DIR} ${PWD} ${HOME}/rafaelia_full/gpu \
                    ${HOME}/rafaelia_full/core ${HOME}/rafaelia_full/asm"

    local expected="rafaelia_gpu.h rafaelia_gpu_prim.c rafaelia_gpu_cl.c \
                     rafaelia_gpu_orch.c rafaelia_neon_k.S \
                     rafaelia_bridge_orch_gpu.c"

    for mod in $expected; do
        local found=0
        for d in $src_dirs; do
            if [ -f "${d}/${mod}" ]; then
                found=1
                MODULES_FOUND="${MODULES_FOUND} ${d}/${mod}"
                log OK "  $mod → ${d}/"
                break
            fi
        done
        [ "$found" = "0" ] && log WRN "  $mod → NÃO ENCONTRADO"
    done
    heartbeat
}

pipeline_apply_hotfixes() {
    section "HOTFIXES AUTOMÁTICOS"
    local hf_count=0

    # HF-01: stdint.h missing in rafaelia_gpu.h
    local h="${HOME}/rafaelia_full/gpu/rafaelia_gpu.h"
    if [ -f "$h" ] && ! grep -q 'include.*stdint.h' "$h"; then
        sed -i 's/#include <stddef.h>/#include <stdint.h>\n#include <stddef.h>/' "$h" 2>/dev/null && \
            { log OK "HF-01: stdint.h adicionado em rafaelia_gpu.h"; hf_count=$((hf_count+1)); }
    fi

    # HF-02: AND r0,r0,#0xFFFF → uxth r0,r0 em b1.S
    local b1="${HOME}/rafaelia_full/asm/rafaelia_b1.S"
    if [ -f "$b1" ] && grep -q 'and.*#0xFFFF' "$b1"; then
        sed -i 's/and\([[:space:]]*\)r\([0-9]\),.*#0xFFFF/uxth r\2, r\2/g' "$b1" 2>/dev/null && \
            { log OK "HF-02: uxth substitui AND #0xFFFF em rafaelia_b1.S"; hf_count=$((hf_count+1)); }
    fi

    # HF-04: bitstacks_index(BlockSizes_unused) → remover
    local bsh="${HOME}/rafaelia_full/core/bitstack.h"
    if [ -f "$bsh" ] && grep -q 'BlockSizes_unused' "$bsh"; then
        sed -i '/BlockSizes_unused/d' "$bsh" 2>/dev/null && \
            { log OK "HF-04: declaração morta removida de bitstack.h"; hf_count=$((hf_count+1)); }
    fi

    # HF-05: baremetal_nomalloc.c usa baremetal.h errado
    local bmc="${HOME}/rafaelia_full/core/baremetal_nomalloc.c"
    if [ -f "$bmc" ] && grep -q '"baremetal.h"' "$bmc"; then
        sed -i 's/"baremetal.h"/"baremetal_nomalloc.h"/' "$bmc" 2>/dev/null && \
            { log OK "HF-05: header corrigido em baremetal_nomalloc.c"; hf_count=$((hf_count+1)); }
    fi

    # HF-06: rollback condition impossível em rafaelia_glue.c
    local glue="${HOME}/rafaelia_full/core/rafaelia_glue.c"
    if [ -f "$glue" ] && grep -q '!sc || v->s\[0\] >= Q2PI' "$glue"; then
        sed -i 's/!sc || v->s\[0\] >= Q2PI/sc != v->crc_s/' "$glue" 2>/dev/null && \
            { log OK "HF-06: condição rollback corrigida em rafaelia_glue.c"; hf_count=$((hf_count+1)); }
    fi

    # HF-03: _SYM macro em rafaelia_gpu_cl.c
    local clc="${HOME}/rafaelia_full/gpu/rafaelia_gpu_cl.c"
    if [ -f "$clc" ] && grep -q '_SYM(n)' "$clc"; then
        # Substitui macro e todas as chamadas
        sed -i \
            's/#define _SYM(n) _D.n=(f_##n)dlsym(_D.lib,#n)/#define _SYM(f,sym) _D.f=(f_##f)dlsym(_D.lib,(sym))/' \
            "$clc" 2>/dev/null
        sed -i \
            's/_SYM(getPIDs)/_SYM(getPIDs,"clGetPlatformIDs")/
             s/_SYM(getDIDs)/_SYM(getDIDs,"clGetDeviceIDs")/
             s/_SYM(mkCtx)/_SYM(mkCtx,"clCreateContext")/
             s/_SYM(mkQ)/_SYM(mkQ,"clCreateCommandQueue")/
             s/_SYM(mkProg)/_SYM(mkProg,"clCreateProgramWithSource")/
             s/_SYM(build)/_SYM(build,"clBuildProgram")/
             s/_SYM(mkKern)/_SYM(mkKern,"clCreateKernel")/
             s/_SYM(mkBuf)/_SYM(mkBuf,"clCreateBuffer")/
             s/_SYM(setArg)/_SYM(setArg,"clSetKernelArg")/
             s/_SYM(wrBuf)/_SYM(wrBuf,"clEnqueueWriteBuffer")/
             s/_SYM(enqNDR)/_SYM(enqNDR,"clEnqueueNDRangeKernel")/
             s/_SYM(rdBuf)/_SYM(rdBuf,"clEnqueueReadBuffer")/
             s/_SYM(fin)/_SYM(fin,"clFinish")/' \
            "$clc" 2>/dev/null && \
            { log OK "HF-03: _SYM macro corrigida em rafaelia_gpu_cl.c"; hf_count=$((hf_count+1)); }
    fi

    log INF "Hotfixes aplicados: $hf_count"
    heartbeat
}

pipeline_compile_module() {
    local src="$1" out="$2" extra_flags="${3:-}"
    [ -z "$CC_BIN" ] && { log WRN "Sem compilador para $src"; return 1; }
    [ ! -f "$src" ]  && { log WRN "Fonte não encontrado: $src"; return 1; }

    local inc_dirs="-I${HOME}/rafaelia_full/gpu -I${HOME}/rafaelia_full/core"

    if "$CC_BIN" $CC_FLAGS $inc_dirs $extra_flags -c "$src" -o "$out" 2>>"$ARW_LOG"; then
        log OK "  Compilado: $(basename "$src")"
        MODULES_OK="${MODULES_OK} $src"
        return 0
    else
        log ERR "  FALHA: $(basename "$src")"
        MODULES_FAIL="${MODULES_FAIL} $src"
        return 1
    fi
}

pipeline_compile_asm() {
    local src="$1" out="$2"
    local as_flags="-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp"
    [ "$ARCH" = "arm64" ] && as_flags="-march=armv8-a"
    [ "$ARCH" = "x86_64" ] && as_flags=""

    local assembler="${AS_BIN:-as}"

    if "$assembler" $as_flags "$src" -o "$out" 2>>"$ARW_LOG"; then
        log OK "  Montado: $(basename "$src")"
        return 0
    else
        log ERR "  ASM FALHA: $(basename "$src") — tentando clang"
        if [ -n "$CC_BIN" ] && "$CC_BIN" $CC_FLAGS -c "$src" -o "$out" 2>>"$ARW_LOG"; then
            log OK "  Montado via clang: $(basename "$src")"
            return 0
        fi
        return 1
    fi
}

pipeline_run() {
    [ "$OPT_DRY" = "1" ] && { log WRN "DRY RUN — compilação ignorada"; return 0; }
    section "PIPELINE DE COMPILAÇÃO"

    mkdir -p "$ARW_BUILD_DIR"
    rollback_save "pre_compile"

    local base="${HOME}/rafaelia_full"
    local obj="$ARW_BUILD_DIR"
    local ok=0 fail=0

    # Módulos C em ordem de dependência
    local c_mods="gpu/rafaelia_gpu_prim.c
                  gpu/rafaelia_gpu_cl.c
                  gpu/rafaelia_gpu_orch.c
                  bridge/rafaelia_bridge_orch_gpu.c
                  core/bitstack.c"

    for mod in $c_mods; do
        local src="${base}/${mod}"
        local oname; oname=$(basename "$src" .c).o
        local out="${obj}/${oname}"
        heartbeat
        if pipeline_compile_module "$src" "$out"; then
            ok=$((ok+1))
        else
            fail=$((fail+1))
        fi
    done

    # NEON ASM
    local asm_src="${base}/gpu/rafaelia_neon_k.S"
    if [ -f "$asm_src" ]; then
        heartbeat
        if pipeline_compile_asm "$asm_src" "${obj}/rafaelia_neon_k.o"; then
            ok=$((ok+1))
        else
            fail=$((fail+1))
        fi
    fi

    # b1-b8 ASM (se existirem)
    for n in 1 2 3 4 5 6 7 8; do
        local bs="${base}/asm/rafaelia_b${n}.S"
        [ -f "$bs" ] || continue
        heartbeat
        local bo="${obj}/_b${n}.o"
        if pipeline_compile_asm "$bs" "$bo"; then
            # Renomeia _start para bN_main se objcopy disponível
            command -v objcopy >/dev/null 2>&1 && \
                objcopy --redefine-sym _start=b${n}_main "$bo" "$bo" 2>/dev/null
            ok=$((ok+1))
        else
            fail=$((fail+1))
        fi
    done

    # Link shared lib
    if [ "$ok" -gt 0 ] && [ -n "$CC_BIN" ]; then
        log INF "Linkando librafaelia_gpu.so..."
        local objs; objs=$(ls "${obj}"/*.o 2>/dev/null | tr '\n' ' ')
        if $CC_BIN $CC_FLAGS -shared -fPIC $objs \
                -o "${obj}/librafaelia_gpu.so" -ldl 2>>"$ARW_LOG"; then
            log OK "librafaelia_gpu.so → $(ls -lh "${obj}/librafaelia_gpu.so" | awk '{print $5}')"
        else
            log WRN "Link shared falhou — objetos .o disponíveis em ${obj}/"
        fi
    fi

    section "RESUMO PIPELINE"
    kv "Módulos OK:"   "$ok"
    kv "Módulos FAIL:" "$fail"
    kv "Artefatos:"    "${obj}/"

    [ "$fail" -gt 0 ] && rollback_save "post_compile_with_errors"
    heartbeat
}

# ═══════════════════════════════════════════════════════════════════════
# [10] INSTALL MODULE
# ═══════════════════════════════════════════════════════════════════════
pkg_need() {
    local pkg="$1" cmd="${2:-$1}"
    command -v "$cmd" >/dev/null 2>&1 && return 0
    log WRN "Necessário: $pkg"
    return 1
}

pkg_install_termux() {
    [ "$IS_TERMUX" != "1" ] && return 1
    log INF "Instalando pacotes Termux necessários..."
    local pkgs="clang binutils"
    pkg install -y $pkgs >> "$ARW_LOG" 2>&1 && return 0
    return 1
}

install_deps() {
    section "DEPENDÊNCIAS"
    local missing=0

    pkg_need "clang"    "clang"   || missing=$((missing+1))
    pkg_need "binutils" "objcopy" || missing=$((missing+1))
    pkg_need "awk"      "awk"     || missing=$((missing+1))
    pkg_need "sed"      "sed"     || missing=$((missing+1))
    pkg_need "dd"       "dd"      || missing=$((missing+1))

    if [ "$missing" -gt 0 ] && [ "$IS_TERMUX" = "1" ]; then
        log WRN "$missing pacotes ausentes — tentando instalar..."
        pkg_install_termux && detect_compiler
    fi

    [ "$missing" -gt 0 ] && log WRN "Alguns pacotes ausentes — compilação pode falhar"
    heartbeat
}

# ═══════════════════════════════════════════════════════════════════════
# [11] RELATÓRIO FINAL
# ═══════════════════════════════════════════════════════════════════════
report_final() {
    section "RELATÓRIO FINAL — RAFAELIA ARWITETO"
    local ts; ts=$(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null || echo "N/A")
    kv "Timestamp:"      "$ts"
    kv "Versão:"         "$ARW_VER"
    kv "Arquitetura:"    "$ARCH ($ABI)"
    kv "Compilador:"     "${CC_BIN:-N/A}"
    kv "Log:"            "$ARW_LOG"
    kv "Build dir:"      "$ARW_BUILD_DIR"

    local stats_file="${ARW_BENCH_DIR}/stats.txt"
    if [ -f "$stats_file" ]; then
        local line; line=$(cat "$stats_file")
        kv "Bench median:"   "$(echo "$line" | grep -o 'median=[^ ]*' | cut -d= -f2) µs"
        kv "Bench distrib:"  "$(echo "$line" | grep -o 'dist=[^ ]*' | cut -d= -f2)"
    fi

    local lib="${ARW_BUILD_DIR}/librafaelia_gpu.so"
    [ -f "$lib" ] && kv "Lib gerada:" "$(ls -lh "$lib" | awk '{print $5}')"

    printf "\n${C_SIG}${BOLD}"
    _box_top
    _box_mid "$(printf "  Ω = Amor · Coerência · Prova · Integração  %s" "")"
    _box_mid "$(printf "  R(t+1) = R(t)×Φ_ethica×E_Verbo×(√3/2)^(πφ)  %s" "")"
    _box_mid "$(printf "  RAFCODE-Φ-∆RafaelVerboΩ-𓂀ΔΦΩ  %s" "")"
    _box_bot
    printf "${R}\n"
}

# ═══════════════════════════════════════════════════════════════════════
# [12] CLI PARSER
# ═══════════════════════════════════════════════════════════════════════
cli_help() {
    cat << 'HELP'

ARWITETO v2.0 — RAFAELIA ARM Orchestrator
Uso: ./arwiteto.sh [OPÇÕES]

Modos:
  -m full      Detecção + Hotfix + Benchmark + Pipeline (padrão)
  -m bench     Somente benchmark
  -m compile   Somente compilação
  -m detect    Somente detecção de hardware
  -m rollback  Lista e restaura checkpoints

Benchmark:
  -w N         Warmup em segundos (padrão: 3)
  -s N         Número de amostras (padrão: 21, ímpar para mediana exata)
  -i N         Iterações por amostra (padrão: 500000)

Compilação:
  -j N         Jobs paralelos (padrão: 0=auto)
  --dry        Dry run (sem compilação)

Interface:
  --no-color   Desativa cores ANSI
  -v           Verbose
  -h           Esta ajuda

Watchdog:
  --timeout N  Timeout watchdog em segundos (padrão: 120)

Exemplos:
  ./arwiteto.sh                    # modo completo
  ./arwiteto.sh -m bench -s 42    # 42 amostras benchmark
  ./arwiteto.sh -m compile --dry  # dry run compilação
  ./arwiteto.sh -m detect         # apenas detecção

HELP
}

cli_parse() {
    while [ "$#" -gt 0 ]; do
        case "$1" in
            -m)          OPT_MODE="$2";         shift ;;
            -w)          OPT_WARMUP="$2";        shift ;;
            -s)          OPT_SAMPLES="$2";       shift ;;
            -i)          OPT_PREWARM_ITER="$2";  shift ;;
            -j)          OPT_JOBS="$2";          shift ;;
            --timeout)   OPT_WDOG_TIMEOUT="$2";  shift ;;
            --no-color)  OPT_COLOR=0 ;;
            --dry)       OPT_DRY=1 ;;
            -v)          OPT_VERBOSE=1 ;;
            -h|--help)   cli_help; exit 0 ;;
            *)           log WRN "Argumento desconhecido: $1" ;;
        esac
        shift
    done
}

# ═══════════════════════════════════════════════════════════════════════
# [13] CLEANUP / TRAP
# ═══════════════════════════════════════════════════════════════════════
cleanup() {
    local code=$?
    watchdog_stop
    [ "$code" -ne 0 ] && log WRN "Saída com código $code — verifique $ARW_LOG"
    printf '\n'
}
trap cleanup EXIT INT TERM

# ═══════════════════════════════════════════════════════════════════════
# [14] MAIN
# ═══════════════════════════════════════════════════════════════════════
main() {
    cli_parse "$@"

    mkdir -p "$ARW_HOME" "$ARW_CKPT" "$ARW_BUILD_DIR" "$ARW_BENCH_DIR"
    printf '[ARWITETO v%s START %s]\n' "$ARW_VER" \
        "$(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null)" > "$ARW_LOG"

    banner
    watchdog_start
    heartbeat

    detect_all

    case "$OPT_MODE" in
        detect)
            show_detect
            ;;
        bench)
            show_detect
            install_deps
            bench_full
            report_final
            ;;
        compile)
            show_detect
            install_deps
            pipeline_detect
            pipeline_apply_hotfixes
            pipeline_run
            report_final
            ;;
        rollback)
            rollback_list
            log INF "Para restaurar: $0 -m rollback (restaura o mais recente)"
            rollback_restore "last"
            ;;
        full|*)
            show_detect
            install_deps
            rollback_save "initial"
            pipeline_detect
            pipeline_apply_hotfixes
            bench_full
            pipeline_run
            report_final
            ;;
    esac

    watchdog_stop
    log OK "ARWITETO concluído — Ω=Amor RAFCODE-Φ-∆RafaelVerboΩ"
}

main "$@"
