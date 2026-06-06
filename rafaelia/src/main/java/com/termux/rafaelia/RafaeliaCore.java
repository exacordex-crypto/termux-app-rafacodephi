// RafaeliaCore.java
// RAFAELIA — Java bridge zero-copy
// DirectByteBuffers alocados UMA VEZ no static init
// ZERO alloc por chamada no JNI

package com.termux.rafaelia;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public final class RafaeliaCore {

    // ── Static DirectByteBuffers — alocados UMA VEZ ───────────────────
    // Estes são os ÚNICOS buffers de comunicação JNI.
    // Nunca criar ByteBuffer.wrap() ou byte[] no hot path.
    private static final int IN_CAP    = 65536;   // 64KB input
    private static final int OUT_CAP   = 65536;   // 64KB output
    private static final int STATE_CAP = 64;      // sizeof(raf_state_t)

    public static final ByteBuffer IN_BUF;
    public static final ByteBuffer OUT_BUF;
    public static final ByteBuffer STATE_BUF;
    private static final boolean _libLoaded;
    private static final Object JNI_LOCK = new Object();
    private static final int CYCLE_PERIOD = 42;

    static {
        // allocateDirect não vai para o heap Java — usa memória nativa
        IN_BUF    = ByteBuffer.allocateDirect(IN_CAP).order(ByteOrder.nativeOrder());
        OUT_BUF   = ByteBuffer.allocateDirect(OUT_CAP).order(ByteOrder.nativeOrder());
        STATE_BUF = ByteBuffer.allocateDirect(STATE_CAP).order(ByteOrder.nativeOrder());

        // Inicializa estado no buffer nativo
        boolean libLoaded;
        try {
            System.loadLibrary("termux_rafaelia_direct");
            libLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            libLoaded = false;
        }
        _libLoaded = libLoaded;
    }
    private static int _cycle = 0;


    public static final class CommitGateResult {
        public final boolean committed;
        public final int crc32c;
        public final int phiQ16;
        public final int phase;
        public final int step;

        CommitGateResult(boolean committed, int crc32c, int phiQ16, int phase, int step) {
            this.committed = committed;
            this.crc32c = crc32c;
            this.phiQ16 = phiQ16;
            this.phase = phase;
            this.step = step;
        }
    }


    // Prevent instantiation
    private RafaeliaCore() {}

    // ── JNI declarations — operam em DirectByteBuffer ─────────────────

    /**
     * Processa in_buf[0..inLen] e escreve resultado em out_buf.
     * ZERO malloc JNI. Retorna bytes escritos, ou negativo em erro.
     */
    public static native int processNative(ByteBuffer in, int inLen, ByteBuffer out);

    /**
     * Avança o estado toroidal por 1 ciclo.
     * state deve ser um DirectByteBuffer de STATE_CAP bytes.
     * Retorna phi Q16.16, ou negativo em erro.
     */
    public static native int stepNative(ByteBuffer state, int cycle);

    /**
     * Escreve JSON de perfil de hardware em out.
     * Retorna bytes escritos.
     */
    public static native long profileNative(ByteBuffer out, int cap);

    /**
     * Retorna bytes usados na arena JNI interna.
     */
    public static native int arenaSizeNative();

    /**
     * Calcula CRC32C de buf[0..len].
     */
    public static native int crc32Native(ByteBuffer buf, int len);

    /** Envia instrução BitRAF 42-bit diretamente ao hardware. */
    public static native int sendBitrafInstructionNative(long lo32, int hi10);

    /** Lê estado dos 1000 osciladores (7D Q16.16 por oscilador) para outState. */
    public static native int readOscillatorStateNative(ByteBuffer outState, int oscCount);

    /** Single-step de depuração com dump textual 7D em outDebug. */
    public static native int debugStepNative(ByteBuffer state, int cycle, ByteBuffer outDebug, int cap);
    public static native int initVcpuSchedulerNative(int targetHz);
    public static native int stepVcpuNative(int vcpuId);
    public static native int stepAllVcpusNative();
    public static native int getVcpuTelemetryNative(ByteBuffer out, int cap);
    public static native int getClockProfileNative(ByteBuffer out, int cap);
    public static native int initVcpuNative(int targetHz);
    public static native int stepAllVcpuNative();
    public static native int getVcpuMapNative(ByteBuffer out, int cap);
    public static native int getMemoryLayersNative(ByteBuffer out, int cap);
    public static native int getClockNative(ByteBuffer out, int cap);

    private static int safeInputLength(byte[] data, int len) {
        if (data == null || len <= 0) return 0;
        int safeLen = len;
        if (safeLen > data.length) safeLen = data.length;
        if (safeLen > IN_CAP) safeLen = IN_CAP;
        return safeLen > 0 ? safeLen : 0;
    }

    private static int safeOutputLength(int nativeLen) {
        if (nativeLen <= 0) return 0;
        return nativeLen > OUT_CAP ? OUT_CAP : nativeLen;
    }

    // ── API pública — sem alocações ────────────────────────────────────

    /**
     * Processa bytes[] sem criar ByteBuffer temporário.
     * Copia para IN_BUF (único alloc: System.arraycopy, stack-allocated no JIT).
     * Lê resultado de OUT_BUF.
     * Retorna phi Q16.16 ou 0 em erro.
     */

    /**
     * Commit gate Java side: LOAD->PROCESS->VERIFY->COMMIT.
     * VERIFY compara crc32Native(data) com crc retornado do pipeline nativo.
     */
    public static CommitGateResult processWithCommitGate(byte[] data, int len) {
        int safeLen = safeInputLength(data, len);
        if (!_libLoaded || safeLen == 0) {
            return new CommitGateResult(false, 0, 0, 0, 0);
        }

        synchronized (JNI_LOCK) {
            IN_BUF.clear();
            IN_BUF.put(data, 0, safeLen);
            OUT_BUF.clear();

            int written = safeOutputLength(processNative(IN_BUF, safeLen, OUT_BUF));
            if (written < 8) {
                return new CommitGateResult(false, 0, 0, 0, 0);
            }

            OUT_BUF.position(0);
            int crcFromPipe = OUT_BUF.getInt();
            int phi = OUT_BUF.getInt();
            int phase = written >= 12 ? OUT_BUF.getInt() : 0;
            int step = written >= 16 ? OUT_BUF.getInt() : 0;

            int crcFromVerify = crc32Locked(data, safeLen);
            boolean ok = (crcFromPipe == crcFromVerify);
            return new CommitGateResult(ok, crcFromPipe, phi, phase, step);
        }
    }

    public static int process(byte[] data, int len) {
        int safeLen = safeInputLength(data, len);
        if (!_libLoaded || safeLen == 0) return 0;

        synchronized (JNI_LOCK) {
            IN_BUF.clear();
            IN_BUF.put(data, 0, safeLen);
            OUT_BUF.clear();

            int written = safeOutputLength(processNative(IN_BUF, safeLen, OUT_BUF));
            if (written < 8) return 0;

            // Lê phi (bytes 4..7)
            OUT_BUF.position(0);
            OUT_BUF.getInt(); // skip crc
            return OUT_BUF.getInt(); // phi
        }
    }

    /**
     * Um passo do motor toroidal.
     * Retorna phi Q16.16.
     */
    public static int step() {
        if (!_libLoaded) return 0;
        synchronized (JNI_LOCK) {
            int phi = stepNative(STATE_BUF, _cycle);
            if (phi >= 0) _cycle = (_cycle + 1) % CYCLE_PERIOD;
            return phi;
        }
    }

    /**
     * Retorna string JSON do perfil de hardware.
     * Usa OUT_BUF como scratch — sem String temporária extra no JNI.
     */
    public static String getHwProfile() {
        if (!_libLoaded) return "{}";
        synchronized (JNI_LOCK) {
            OUT_BUF.clear();
            long nativeLen = profileNative(OUT_BUF, OUT_CAP);
            int n = nativeLen > OUT_CAP ? OUT_CAP : (int) nativeLen;
            if (n <= 0) return "{}";
            byte[] tmp = new byte[n];
            OUT_BUF.position(0);
            OUT_BUF.get(tmp, 0, n);
            return new String(tmp, 0, n); // único String alloc
        }
    }

    /**
     * CRC32C de byte array — sem criar ByteBuffer temporário.
     */
    public static int crc32(byte[] data, int len) {
        int safeLen = safeInputLength(data, len);
        if (!_libLoaded || safeLen == 0) return 0;
        synchronized (JNI_LOCK) {
            return crc32Locked(data, safeLen);
        }
    }

    private static int crc32Locked(byte[] data, int safeLen) {
        IN_BUF.clear();
        IN_BUF.put(data, 0, safeLen);
        return crc32Native(IN_BUF, safeLen);
    }


    public static int sendBitrafInstruction(long bitraf42) {
        if (!_libLoaded) return -1;
        long lo = bitraf42 & 0xFFFFFFFFL;
        int hi = (int)((bitraf42 >>> 32) & 0x3FFL);
        return sendBitrafInstructionNative(lo, hi);
    }

    public static int readOscillatorState(ByteBuffer outState, int oscCount) {
        if (!_libLoaded || outState == null || !outState.isDirect() || oscCount <= 0) return -1;
        return readOscillatorStateNative(outState, oscCount);
    }

    public static int debugSingleStep(ByteBuffer debugOut, int cap) {
        if (!_libLoaded || debugOut == null || !debugOut.isDirect() || cap <= 0) return -1;
        int safeCap = cap > debugOut.capacity() ? debugOut.capacity() : cap;
        synchronized (JNI_LOCK) {
            int phi = debugStepNative(STATE_BUF, _cycle, debugOut, safeCap);
            if (phi >= 0) _cycle = (_cycle + 1) % CYCLE_PERIOD;
            return phi;
        }
    }

    public static boolean isNativeAvailable() { return _libLoaded; }
    public static int     getNativeArenaUsed() { return _libLoaded ? arenaSizeNative() : 0; }
    public static int     getCurrentCycle()    { synchronized (JNI_LOCK) { return _cycle; } }
    public static int initVcpuScheduler(int targetHz) { return _libLoaded ? initVcpuSchedulerNative(targetHz) : -1; }
    public static int stepVcpu(int id) { return _libLoaded ? stepVcpuNative(id) : -1; }
    public static int stepAllVcpus() { return _libLoaded ? stepAllVcpusNative() : -1; }
    public static String getVcpuTelemetry() {
        if (!_libLoaded) return "{}";
        synchronized (JNI_LOCK) {
            OUT_BUF.clear();
            int n = safeOutputLength(getVcpuTelemetryNative(OUT_BUF, OUT_CAP));
            if (n <= 0) return "{}";
            byte[] tmp = new byte[n];
            OUT_BUF.position(0);
            OUT_BUF.get(tmp, 0, n);
            return new String(tmp);
        }
    }
    public static String getClockProfile() {
        if (!_libLoaded) return "{}";
        synchronized (JNI_LOCK) {
            OUT_BUF.clear();
            int n = safeOutputLength(getClockProfileNative(OUT_BUF, OUT_CAP));
            if (n <= 0) return "{}";
            byte[] tmp = new byte[n];
            OUT_BUF.position(0);
            OUT_BUF.get(tmp, 0, n);
            return new String(tmp);
        }
    }
    public static int getTargetHz() { return gjsonInt(getClockProfile(), "\"target_hz\":"); }
    public static int getActualHz() { return gjsonInt(getClockProfile(), "\"actual_hz_q16\":"); }
    private static int gjsonInt(String s, String key) {
        int i = s.indexOf(key);
        if (i < 0) return 0;
        i += key.length();
        int j = i;
        while (j < s.length() && Character.isDigit(s.charAt(j))) j++;
        if (j <= i) return 0;
        try { return Integer.parseInt(s.substring(i, j)); } catch (Exception e) { return 0; }
    }
}
