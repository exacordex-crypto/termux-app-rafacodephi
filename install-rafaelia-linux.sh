#!/usr/bin/env bash
set -euo pipefail

PREFIX_DEFAULT="${PREFIX:-$HOME/.termux}"
DISTRO="${DISTRO:-debian}"
SUITE="${SUITE:-bookworm}"
ARCH="${ARCH:-aarch64}"
LINUX_BASE_DIR="${LINUX_BASE_DIR:-$PREFIX_DEFAULT/var/lib/rafaelia-linux}"
ROOTFS_DIR="${ROOTFS_DIR:-$LINUX_BASE_DIR/$DISTRO/rootfs}"
CACHE_DIR="${CACHE_DIR:-$LINUX_BASE_DIR/cache}"
LAUNCHER_PATH="${LAUNCHER_PATH:-$PREFIX_DEFAULT/bin/start-rafaelia-linux.sh}"
RESOLV_CONF_CONTENT="${RESOLV_CONF_CONTENT:-nameserver 1.1.1.1\nnameserver 8.8.8.8\noptions edns0 trust-ad}\n"

need() { command -v "$1" >/dev/null 2>&1 || { echo "[ERRO] comando ausente: $1"; exit 1; }; }
need proot
need tar
need curl
need sha256sum
mkdir -p "$CACHE_DIR" "$ROOTFS_DIR"

ROOTFS_URL=""
ROOTFS_TAR=""
ROOTFS_SHA256=""

case "$DISTRO" in
  debian)
    case "$ARCH" in
      aarch64)
        ROOTFS_TAR="debian-aarch64-pd-v4.16.0.tar.xz"
        ROOTFS_SHA256="72ae4d2faf7f9b31bdf50f4f97504f7fd8ca3cb8d8b7f2634d8eb1208ff31f4f"
        ;;
      arm)
        ROOTFS_TAR="debian-arm-pd-v4.16.0.tar.xz"
        ROOTFS_SHA256="5776e5237510f8b1300c2bd4a19c46bd4af1d6dbce9f7a4ef31a75f3276dc2f8"
        ;;
      i686)
        ROOTFS_TAR="debian-i686-pd-v4.16.0.tar.xz"
        ROOTFS_SHA256="66fa8485d31e0e5bcf2f821f4f1f25d4f6b869c5d9ce66f95f3467a6c2244ef4"
        ;;
      x86_64)
        ROOTFS_TAR="debian-x86_64-pd-v4.16.0.tar.xz"
        ROOTFS_SHA256="e2250fc08a5eb9704d83a28b3df23639f14d89c9f3cf89c6f6cdf9475f3ec3bd"
        ;;
      *)
        echo "[ERRO] ARCH não suportada para Debian: $ARCH"
        exit 1
        ;;
    esac
    ROOTFS_URL="https://github.com/termux/proot-distro/releases/download/v4.16.0/$ROOTFS_TAR"
    ;;
  *)
    echo "[ERRO] distro não suportada: $DISTRO"
    exit 1
    ;;
esac

TAR_PATH="$CACHE_DIR/$ROOTFS_TAR"
if [[ ! -f "$TAR_PATH" ]]; then
  echo "[INFO] baixando rootfs minimal: $ROOTFS_TAR"
  curl -fL "$ROOTFS_URL" -o "$TAR_PATH"
fi

echo "${ROOTFS_SHA256}  ${TAR_PATH}" | sha256sum -c -

if [[ -z "$(find "$ROOTFS_DIR" -mindepth 1 -maxdepth 1 2>/dev/null || true)" ]]; then
  echo "[INFO] extraindo rootfs em $ROOTFS_DIR"
  tar -xJf "$TAR_PATH" -C "$ROOTFS_DIR"
else
  echo "[INFO] rootfs já existe em: $ROOTFS_DIR"
fi

mkdir -p "$ROOTFS_DIR/etc" "$LINUX_BASE_DIR/shared" "$LINUX_BASE_DIR/home" "$(dirname "$LAUNCHER_PATH")"
printf "%b" "$RESOLV_CONF_CONTENT" > "$ROOTFS_DIR/etc/resolv.conf"

cat > "$LAUNCHER_PATH" <<LAUNCH
#!/usr/bin/env bash
set -euo pipefail
ROOTFS_DIR="${ROOTFS_DIR}"
LINUX_BASE_DIR="${LINUX_BASE_DIR}"
export PROOT_NO_SECCOMP=1
exec proot \
  --kill-on-exit \
  --link2symlink \
  -0 \
  -r "\$ROOTFS_DIR" \
  -b /dev \
  -b /proc \
  -b /sys \
  -b /tmp \
  -b /sdcard \
  -b "\$LINUX_BASE_DIR/shared:/mnt/shared" \
  -b "\$LINUX_BASE_DIR/home:/root" \
  -w /root \
  /usr/bin/env -i \
    HOME=/root \
    PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    TERM="\${TERM:-xterm-256color}" \
    LANG=C.UTF-8 \
    /bin/bash --login
LAUNCH
chmod +x "$LAUNCHER_PATH"

echo "[OK] instalação concluída"
echo "distro: $DISTRO"
echo "suite: $SUITE"
echo "arch: $ARCH"
echo "rootfs: $ROOTFS_DIR"
echo "launcher: $LAUNCHER_PATH"
echo "teste: $LAUNCHER_PATH -lc 'cat /etc/os-release && apt update && python3 --version && gcc --version'"
