package com.termux.app;

import android.os.Build;

import androidx.annotation.NonNull;

import com.termux.rafacodephi.BuildConfig;
import com.termux.shared.logger.Logger;

import org.bouncycastle.crypto.digests.Blake3Digest;

import java.util.Locale;

final class BootstrapIntegrityVerifier {

    private static final String LOG_TAG = "BootstrapIntegrity";
    private static final String DEBUG_BOOTSTRAP_HASH_BYPASS = "debug-bootstrap-integrity-bypass";

    private BootstrapIntegrityVerifier() {}

    @NonNull
    static String expectedHashForCurrentAbi() {
        String[] supportedAbis = Build.SUPPORTED_ABIS;
        if (supportedAbis.length == 0) return debugBypassOrEmpty("unknown");

        String abi = supportedAbis[0];
        String expectedHash;
        switch (abi) {
            case "arm64-v8a":
                expectedHash = BuildConfig.BOOTSTRAP_BLAKE3_AARCH64;
                break;
            case "armeabi-v7a":
                expectedHash = BuildConfig.BOOTSTRAP_BLAKE3_ARM;
                break;
            case "x86":
                expectedHash = BuildConfig.BOOTSTRAP_BLAKE3_I686;
                break;
            case "x86_64":
                expectedHash = BuildConfig.BOOTSTRAP_BLAKE3_X86_64;
                break;
            default:
                Logger.logError(LOG_TAG, "Unsupported ABI for bootstrap hash verification: " + abi);
                return debugBypassOrEmpty(abi);
        }

        if (expectedHash == null || expectedHash.isEmpty()) {
            return debugBypassOrEmpty(abi);
        }
        return expectedHash;
    }

    @NonNull
    static String blake3Hex(@NonNull byte[] bytes) {
        if (DEBUG_BOOTSTRAP_HASH_BYPASS.equals(expectedHashForCurrentAbi()) &&
            !BuildConfig.BOOTSTRAP_BAREMETAL_STRICT) {
            Logger.logWarn(LOG_TAG,
                "Bootstrap BLAKE3 hash is not configured for this debug/internal build; " +
                "bypassing integrity comparison so bootstrap can be diagnosed at runtime. " +
                "Release/strict builds still require TERMUX_BOOTSTRAP_BLAKE3_* values.");
            return DEBUG_BOOTSTRAP_HASH_BYPASS;
        }

        Blake3Digest digest = new Blake3Digest(256);
        digest.update(bytes, 0, bytes.length);
        byte[] out = new byte[32];
        digest.doFinal(out, 0);
        StringBuilder sb = new StringBuilder(out.length * 2);
        for (byte b : out) {
            sb.append(String.format(Locale.US, "%02x", b));
        }
        return sb.toString();
    }

    @NonNull
    private static String debugBypassOrEmpty(@NonNull String abi) {
        if (!BuildConfig.BOOTSTRAP_BAREMETAL_STRICT) {
            Logger.logWarn(LOG_TAG,
                "Missing bootstrap BLAKE3 hash for ABI " + abi +
                "; debug/internal build will bypass comparison. " +
                "Set TERMUX_BOOTSTRAP_BLAKE3_* for release/strict builds.");
            return DEBUG_BOOTSTRAP_HASH_BYPASS;
        }
        return "";
    }
}
