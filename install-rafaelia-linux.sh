#!/usr/bin/env bash
set -euo pipefail

PREFIX_DEFAULT="${PREFIX:-$HOME/.rafaelia-linux}"
DISTRO="${DISTRO:-debian}"
SUITE="${SUITE:-bookworm}"
ARCH="${ARCH:-arm64}"
ROOTFS_DIR="${ROOTFS_DIR:-$PREFIX_DEFAULT/rootfs/$DISTRO-$SUITE-$ARCH}"
CACHE_DIR="${CACHE_DIR:-$PREFIX_DEFAULT/cache}"
LAUNCHER_PATH="${LAUNCHER_PATH:-$PREFIX_DEFAULT/start-rafaelia-linux.sh}"
RESOLV_CONF_CONTENT="${RESOLV_CONF_CONTENT:-nameserver 1.1.1.1\nnameserver 8.8.8.8\noptions edns0 trust-ad}\n"

need() { command -v "$1" >/dev/null 2>&1 || { echo "[ERRO] comando ausente: $1"; exit 1; }; }
need proot
need tar
need curl
mkdir -p "$CACHE_DIR" "$ROOTFS_DIR"

if [[ -n "$(find "$ROOTFS_DIR" -mindepth 1 -maxdepth 1 2>/dev/null || true)" ]]; then
  echo "[INFO] rootfs já existe em: $ROOTFS_DIR"
else
  case "$DISTRO" in
    debian)
      BASE_URL="https://github.com/termux/proot-distro/releases/download/v4.16.0"
      TAR_NAME="debian-${ARCH}-pd-v4.16.0.tar.xz"
      ;;
    *) echo "[ERRO] distro não suportada: $DISTRO"; exit 1 ;;
  esac

  TAR_PATH="$CACHE_DIR/$TAR_NAME"
  if [[ ! -f "$TAR_PATH" ]]; then
    echo "[INFO] baixando rootfs minimal: $TAR_NAME"
    curl -fL "$BASE_URL/$TAR_NAME" -o "$TAR_PATH"
  fi

  echo "[INFO] extraindo rootfs em $ROOTFS_DIR"
  tar -xJf "$TAR_PATH" -C "$ROOTFS_DIR"
fi

mkdir -p "$ROOTFS_DIR/etc" "$PREFIX_DEFAULT/binds/shared" "$PREFIX_DEFAULT/home"
printf "%b" "$RESOLV_CONF_CONTENT" > "$ROOTFS_DIR/etc/resolv.conf"

cat > "$LAUNCHER_PATH" <<LAUNCH
#!/usr/bin/env bash
set -euo pipefail
ROOTFS_DIR="${ROOTFS_DIR}"
BIND_SHARED="${PREFIX_DEFAULT}/binds/shared"
HOME_INNER="${PREFIX_DEFAULT}/home"
export PROOT_NO_SECCOMP=1
exec proot \
  --kill-on-exit \
  --link2symlink \
  -0 \
  -r "\$ROOTFS_DIR" \
  -b /dev \
  -b /proc \
  -b /sys \
  -b /sdcard \
  -b "\$BIND_SHARED:/mnt/shared" \
  -b "\$HOME_INNER:/root" \
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
echo "rootfs: $ROOTFS_DIR"
echo "launcher: $LAUNCHER_PATH"
echo "teste: $LAUNCHER_PATH -lc 'cat /etc/os-release'"
