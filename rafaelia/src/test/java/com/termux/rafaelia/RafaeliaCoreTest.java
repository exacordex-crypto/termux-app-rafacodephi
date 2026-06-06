package com.termux.rafaelia;

import org.junit.Test;
import java.nio.ByteBuffer;

import static org.junit.Assert.*;

public class RafaeliaCoreTest {

    @Test
    public void fallbackStateIsSafeWhenNativeUnavailable() {
        if (!RafaeliaCore.isNativeAvailable()) {
            assertEquals("{}", RafaeliaCore.getHwProfile());
            assertEquals(0, RafaeliaCore.getNativeArenaUsed());
            assertEquals(0, RafaeliaCore.process(new byte[]{1,2,3}, 3));
            assertEquals(0, RafaeliaCore.step());
        }
    }

    @Test
    public void commitGateNeverCrashesOnInput() {
        RafaeliaCore.CommitGateResult r = RafaeliaCore.processWithCommitGate(new byte[]{10,20,30,40}, 4);
        assertNotNull(r);
        if (!RafaeliaCore.isNativeAvailable()) {
            assertFalse(r.committed);
        }
    }

    @Test
    public void cycleAlwaysWithinRange() {
        int c = RafaeliaCore.getCurrentCycle();
        assertTrue(c >= 0 && c < 42);
        ByteBuffer dbg = ByteBuffer.allocateDirect(128);
        RafaeliaCore.debugSingleStep(dbg, 128);
        int c2 = RafaeliaCore.getCurrentCycle();
        assertTrue(c2 >= 0 && c2 < 42);
    }
    @Test
    public void publicApiClampsInvalidLengthsAndBuffers() {
        byte[] data = new byte[]{1, 2, 3};
        assertNotNull(RafaeliaCore.processWithCommitGate(data, data.length + 1024));
        assertNotNull(RafaeliaCore.processWithCommitGate(data, Integer.MAX_VALUE));
        assertNotNull(RafaeliaCore.processWithCommitGate(data, -7));
        assertEquals(-1, RafaeliaCore.readOscillatorState(null, 1));
        assertEquals(-1, RafaeliaCore.readOscillatorState(ByteBuffer.allocate(64), 1));
        assertEquals(-1, RafaeliaCore.debugSingleStep(null, 64));
        assertEquals(-1, RafaeliaCore.debugSingleStep(ByteBuffer.allocate(64), 64));
    }

}
