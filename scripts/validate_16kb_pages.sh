#!/bin/bash
# Validation script for Android 15/16 16KB page size compatibility
# This script checks if the APK is built correctly with proper memory alignment

set -euo pipefail

COLOR_RED='\033[0;31m'
COLOR_GREEN='\033[0;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_BLUE='\033[0;34m'
COLOR_RESET='\033[0m'

echo -e "${COLOR_BLUE}==================================================${COLOR_RESET}"
echo -e "${COLOR_BLUE}  Android 15/16 16KB Page Size Validation${COLOR_RESET}"
echo -e "${COLOR_BLUE}==================================================${COLOR_RESET}"
echo ""

APK_FILE=""
REQUIRE_READELF=0
TARGET_ARCHS="arm64-v8a,x86_64"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --apk)
            APK_FILE="${2:-}"
            if [[ -z "$APK_FILE" || "$APK_FILE" == --* ]]; then
                echo -e "${COLOR_RED}✗ --apk requires a file path${COLOR_RESET}" >&2
                exit 1
            fi
            shift 2
            ;;
        --require-readelf)
            REQUIRE_READELF=1
            shift
            ;;
        --abis)
            TARGET_ARCHS="${2:-}"
            if [[ -z "$TARGET_ARCHS" || "$TARGET_ARCHS" == --* ]]; then
                echo -e "${COLOR_RED}✗ --abis requires a comma-separated ABI list${COLOR_RESET}" >&2
                exit 1
            fi
            shift 2
            ;;
        *)
            echo -e "${COLOR_RED}✗ Unknown argument:${COLOR_RESET} $1" >&2
            echo "Usage: $0 [--apk <path>] [--require-readelf] [--abis arm64-v8a,x86_64,armeabi-v7a,x86]"
            exit 1
            ;;
    esac
done

if [[ -z "$APK_FILE" ]]; then
    APK_PATH="app/build/outputs/apk/debug"
    APK_FILE=$(find "$APK_PATH" -name "*.apk" 2>/dev/null | sort | tail -1)
fi

if [ -z "$APK_FILE" ] || [ ! -f "$APK_FILE" ]; then
    echo -e "${COLOR_RED}✗ APK not found!${COLOR_RESET}"
    echo -e "  Please build the APK first with: ./gradlew assembleDebug"
    exit 1
fi

echo -e "${COLOR_GREEN}✓ APK found:${COLOR_RESET} $APK_FILE"
echo ""

# Create temp directory for extraction
TEMP_DIR=$(mktemp -d)

cleanup() {
    local temp_path="${TEMP_DIR:-}"
    if [[ -z "${temp_path}" || "${temp_path}" == "/" || "${temp_path}" == "." ]]; then
        echo "Unsafe TEMP_DIR; aborting cleanup" >&2
        return 1
    fi
    local normalized
    normalized="$(realpath -m "${temp_path}")"
    if [[ ${#normalized} -lt 5 ]]; then
        echo "TEMP_DIR path too short; aborting cleanup" >&2
        return 1
    fi
    case "${normalized}" in
        /tmp/*)
            rm -rf -- "${normalized}"
        ;;
        *)
            echo "Refusing to remove non-/tmp directory: ${normalized}" >&2
            return 1
            ;;
    esac
}

trap cleanup EXIT

echo -e "${COLOR_BLUE}Extracting APK...${COLOR_RESET}"
unzip -q "$APK_FILE" -d "$TEMP_DIR"

# Check for native libraries
echo ""
echo -e "${COLOR_BLUE}=== Native Libraries ===${COLOR_RESET}"
LIBS_FOUND=0
LIBS_CHECKED=0
LIBS_VALID=0
declare -A ARCH_TOTAL=()
declare -A ARCH_VALID=()
declare -A TARGET_ARCH_SET=()
ALL_ARCHS=("arm64-v8a" "x86_64" "armeabi-v7a" "x86")

for arch in "${ALL_ARCHS[@]}"; do
    ARCH_TOTAL["$arch"]=0
    ARCH_VALID["$arch"]=0
done

IFS=',' read -ra RAW_TARGET_ARCHS <<< "$TARGET_ARCHS"
TARGET_ARCHS_NORMALIZED=()
for raw_arch in "${RAW_TARGET_ARCHS[@]}"; do
    arch="$(printf '%s' "$raw_arch" | xargs)"
    [[ -n "$arch" ]] || continue
    case "$arch" in
        arm64-v8a|x86_64|armeabi-v7a|x86)
            TARGET_ARCHS_NORMALIZED+=("$arch")
            TARGET_ARCH_SET["$arch"]=1
            ;;
        *)
            echo -e "${COLOR_RED}✗ Unsupported ABI:${COLOR_RESET} $arch" >&2
            echo "Supported ABIs: arm64-v8a,x86_64,armeabi-v7a,x86" >&2
            exit 1
            ;;
    esac
done

if [ "${#TARGET_ARCHS_NORMALIZED[@]}" -eq 0 ]; then
    echo -e "${COLOR_RED}✗ No ABI selected${COLOR_RESET}" >&2
    exit 1
fi

is_target_arch() {
    [[ -n "${TARGET_ARCH_SET[$1]:-}" ]]
}

is_64_bit_arch() {
    [[ "$1" = "arm64-v8a" || "$1" = "x86_64" ]]
}

alignment_is_16kb_compatible() {
    local alignment="$1"
    local value
    if [[ "$alignment" =~ ^0x[0-9a-fA-F]+$ ]]; then
        value=$((alignment))
    elif [[ "$alignment" =~ ^[0-9]+$ ]]; then
        value="$alignment"
    else
        return 1
    fi
    [[ "$value" -ge 16384 ]]
}

for arch in "${ALL_ARCHS[@]}"; do
    LIB_DIR="$TEMP_DIR/lib/$arch"
    if [ -d "$LIB_DIR" ]; then
        echo -e "${COLOR_YELLOW}Architecture: $arch${COLOR_RESET}"
        
        for lib in "$LIB_DIR"/*.so; do
            if [ -f "$lib" ]; then
                LIBS_FOUND=$((LIBS_FOUND + 1))
                LIBNAME=$(basename "$lib")
                
                # Check if readelf is available
                if command -v readelf &> /dev/null; then
                    LIBS_CHECKED=$((LIBS_CHECKED + 1))
                    
                    # Check page alignment - extract from LOAD segment's Align field
                    # The alignment is the last field on the second line after "LOAD"
                    ALIGNMENT=$(readelf -l "$lib" 2>/dev/null | awk '/LOAD/{getline; print $NF; exit}')
                    
                    if is_target_arch "$arch"; then
                        ARCH_TOTAL["$arch"]=$((ARCH_TOTAL["$arch"] + 1))
                    fi
                    
                    if is_64_bit_arch "$arch"; then
                        if alignment_is_16kb_compatible "$ALIGNMENT"; then
                            echo -e "  ${COLOR_GREEN}✓${COLOR_RESET} $LIBNAME - Alignment: $ALIGNMENT (≥16KB) ✓"
                            LIBS_VALID=$((LIBS_VALID + 1))
                            if is_target_arch "$arch"; then
                                ARCH_VALID["$arch"]=$((ARCH_VALID["$arch"] + 1))
                            fi
                        else
                            echo -e "  ${COLOR_RED}✗${COLOR_RESET} $LIBNAME - Alignment: $ALIGNMENT (should be ≥0x4000)"
                        fi
                    else
                        echo -e "  ${COLOR_BLUE}•${COLOR_RESET} $LIBNAME - Alignment: $ALIGNMENT (32-bit ABI present)"
                        if is_target_arch "$arch"; then
                            ARCH_VALID["$arch"]=$((ARCH_VALID["$arch"] + 1))
                        fi
                    fi
                else
                    echo -e "  ${COLOR_BLUE}•${COLOR_RESET} $LIBNAME - found"
                fi
            fi
        done
    fi
done

echo ""

# Summary
echo -e "${COLOR_BLUE}=== Summary ===${COLOR_RESET}"
echo -e "Libraries found: $LIBS_FOUND"
if command -v readelf &> /dev/null; then
    echo -e "Libraries checked: $LIBS_CHECKED"
    echo -e "Libraries with 16KB alignment: $LIBS_VALID"
    echo ""
    echo -e "${COLOR_BLUE}=== Target ABIs ===${COLOR_RESET}"
    for arch in "${TARGET_ARCHS_NORMALIZED[@]}"; do
        if is_64_bit_arch "$arch"; then
            printf '%s: %s/%s libraries with >=16KB alignment\n' "$arch" "${ARCH_VALID[$arch]}" "${ARCH_TOTAL[$arch]}"
        else
            printf '%s: %s/%s ARM32/x86 32-bit libraries present\n' "$arch" "${ARCH_VALID[$arch]}" "${ARCH_TOTAL[$arch]}"
        fi
    done
    echo ""
    
    TARGET_PASS=true
    TARGET_TOTAL=0
    TARGET_VALID=0
    for arch in "${TARGET_ARCHS_NORMALIZED[@]}"; do
        TARGET_TOTAL=$((TARGET_TOTAL + ARCH_TOTAL["$arch"]))
        TARGET_VALID=$((TARGET_VALID + ARCH_VALID["$arch"]))
        if [ "${ARCH_TOTAL[$arch]}" -eq 0 ] || [ "${ARCH_VALID[$arch]}" -ne "${ARCH_TOTAL[$arch]}" ]; then
            TARGET_PASS=false
        fi
    done

    if [ "$TARGET_PASS" = true ] && [ "$TARGET_TOTAL" -gt 0 ] && [ "$TARGET_VALID" -eq "$TARGET_TOTAL" ]; then
        echo -e "${COLOR_GREEN}✓✓✓ ABI CHECKS PASSED! ✓✓✓${COLOR_RESET}"
        echo -e "APK is correctly built for Android 15/16 with 16KB page size support"
        echo ""
        echo -e "${COLOR_GREEN}Safe to install on:${COLOR_RESET}"
        echo "  • Android 15 with 4KB pages ✓"
        echo "  • Android 15 with 16KB pages ✓"
        echo "  • Android 16 Beta (arm64-v8a devices) ✓"
        echo "  • RMX3834 and similar ARM64 devices ✓"
        echo ""
        echo -e "${COLOR_BLUE}Note:${COLOR_RESET} 32-bit architectures (armeabi-v7a, x86) use standard alignment."
        echo "This is acceptable as Android 15/16 16KB page requirement is for 64-bit only."
        echo ""
        exit 0
    else
        echo -e "${COLOR_RED}✗✗✗ VALIDATION FAILED! ✗✗✗${COLOR_RESET}"
        echo -e "One or more requested ABIs are missing or fail their alignment contract"
        echo -e "64-bit ABIs require ≥16KB page alignment; 32-bit ABIs must be present and readable"
        echo ""
        exit 1
    fi
else
    echo ""
    echo -e "${COLOR_YELLOW}⚠ Warning: readelf not found${COLOR_RESET}"
    echo "Cannot verify page alignment. Install binutils:"
    echo "  • Ubuntu/Debian: apt-get install binutils"
    echo "  • macOS: brew install binutils"
    echo "  • Or use Android NDK's readelf from: \$NDK_ROOT/toolchains/llvm/prebuilt/*/bin/"
    echo ""
    echo -e "${COLOR_BLUE}Manual verification:${COLOR_RESET}"
    echo "  1. Install on Android 15/16 device: adb install -r \"$APK_FILE\""
    echo "  2. Run the app and check for crashes"
    echo "  3. Check logcat: adb logcat | grep -i 'sigsegv\\|signal 11\\|termux'"
    echo ""
    if [ "$REQUIRE_READELF" -eq 1 ]; then
        exit 1
    fi
    exit 2
fi
