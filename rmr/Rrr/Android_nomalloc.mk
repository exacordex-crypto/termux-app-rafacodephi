# Android_nomalloc.mk
# RAFAELIA — build sem malloc, page-size 16KB, zero overhead
# Coloca em: app/src/main/cpp/lowlevel/Android_nomalloc.mk
# Usar: ndk-build NDK_APPLICATION_MK=Application.mk

LOCAL_PATH := $(call my-dir)

# ── librafaelia_core ───────────────────────────────────────────────────────
include $(CLEAR_VARS)

LOCAL_MODULE    := rafaelia_core
LOCAL_SRC_FILES := \
    baremetal_nomalloc.c \
    rafaelia_jni_direct.c \
    rafaelia_orchestrator.c \
    cti_raw_reader.c \
    zipraf_index.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)

# Zero malloc: -fno-exceptions remove overhead de C++ EH
# -fno-rtti remove RTTI tables (puro C aqui)
# -ffast-math permite reordenar ops FP (seguro para Q16.16)
LOCAL_CFLAGS := \
    -O2 \
    -ffast-math \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -DNDEBUG \
    -D_GNU_SOURCE

# ARM32 específico
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS += \
    -march=armv7-a \
    -mfpu=neon-vfpv4 \
    -mfloat-abi=softfp \
    -DHAS_NEON \
    -DHAS_BM_NEON_ASM
LOCAL_SRC_FILES += rafaelia_b1.S rafaelia_b2.S rafaelia_b3.S \
                   rafaelia_b4.S rafaelia_b5.S
endif

# ARM64 específico
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_CFLAGS += \
    -march=armv8-a+crc \
    -DHAS_NEON \
    -DHAS_BM_NEON_ASM \
    -DRAF_ARCH64
LOCAL_SRC_FILES += rafaelia_b1.S rafaelia_b2.S
endif

# CRÍTICO: page-size 16KB para Android 16 (Pixel 9 / future devices)
# Motorola E7 Power: 4KB OK, mas padronizar para compatibilidade
LOCAL_LDFLAGS := \
    -Wl,-z,max-page-size=16384 \
    -Wl,-z,common-page-size=16384 \
    -Wl,--gc-sections \
    -Wl,-z,relro \
    -Wl,-z,now

LOCAL_LDLIBS := -llog -ldl -lm

# NÃO linka libstdc++ (puro C)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES :=

include $(BUILD_SHARED_LIBRARY)

# ── rafaelia_bitraf (standalone tool, debug only) ──────────────────────────
include $(CLEAR_VARS)
LOCAL_MODULE    := rafaelia_bitraf
LOCAL_SRC_FILES := rafaelia_bitraf.c
LOCAL_CFLAGS    := -O2 -ffast-math -DNDEBUG -D_GNU_SOURCE
include $(BUILD_EXECUTABLE)

# ── cti_scan_tool (standalone: scan any file with CTI BITSTACK) ────────────
include $(CLEAR_VARS)
LOCAL_MODULE    := cti_scan_tool
LOCAL_SRC_FILES := cti_raw_reader.c
LOCAL_CFLAGS    := -O2 -DNDEBUG -D_GNU_SOURCE -DCTI_BUILD_MAIN
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_CFLAGS += -march=armv8-a+crc -DRAF_ARCH64
endif
include $(BUILD_EXECUTABLE)

# ── zipraf_tool (standalone: print manifesto for a ZIP) ───────────────────
include $(CLEAR_VARS)
LOCAL_MODULE    := zipraf_tool
LOCAL_SRC_FILES := zipraf_index.c
LOCAL_CFLAGS    := -O2 -DNDEBUG -D_GNU_SOURCE -DZIPRAF_BUILD_MAIN
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_CFLAGS += -march=armv8-a+crc -DRAF_ARCH64
endif
include $(BUILD_EXECUTABLE)
