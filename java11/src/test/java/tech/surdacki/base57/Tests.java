package tech.surdacki.base57;

import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestFactory;

import java.io.*;
import java.nio.ByteBuffer;
import java.util.*;

public final class Tests {

    static final int LIMITED_TESTS_NUMBER = 32;
    static final int DEFAULT_TESTS_NUMBER = 1024;
    static final int EXTENSIVE_TESTS_NUMBER = 1024 * 1024;
    static final int BLOB_MAX_SIZE = 32 * 1024 *1024;

    @Test
    void testEncodingInvariance() {
        assertEquals("ZYY22344556", Encoder.encodeToString(0));
        assertEquals("pX8jCDYpvEF", Encoder.encodeToString(0x0123456789ABCDEFL));
        assertEquals("vgUKGEG53ut", Encoder.encodeToString(0xFEDCBA9876543210L));
        assertEquals("Ud36kjFsx98", Encoder.encodeToString(-1-58));
        assertEquals("Vf47mkGtya9", Encoder.encodeToString(-1));
    }

    @Test
    void testDecodingInvariance() {
        assertEquals(0, Decoder.decodeLong("ZYY22344556"));
        assertEquals(0x0123456789ABCDEFL, Decoder.decodeLong("pX8jCDYpvEF"));
        assertEquals(0xFEDCBA9876543210L, Decoder.decodeLong("vgUKGEG53ut"));
        assertEquals(-1-58, Decoder.decodeLong("Ud36kjFsx98"));
        assertEquals(-1, Decoder.decodeLong("Vf47mkGtya9"));
    }

    @Test
    void testSmallIntegers() {
        for (long x = -DEFAULT_TESTS_NUMBER; x <= DEFAULT_TESTS_NUMBER; ++x) {
            String encoded = Encoder.encodeToString(x);
            assertEquals(x, Decoder.decodeLong(encoded));
        }
    }

    @Test
    void testRandomIntegers() {
        Random prng = new Random(0x28654A083F05F36BL);
        for(int t = 0; t < EXTENSIVE_TESTS_NUMBER; ++t) {
            long x = prng.nextLong();
            String encoded = Encoder.encodeToString(x);
            assertEquals(x, Decoder.decodeLong(encoded));
        }
    }

    @Test
    void testRandomUUIDs() {
        Random prng = new Random(0xCCAE957E275DC4F5L);
        for(int t = 0; t < EXTENSIVE_TESTS_NUMBER; ++t) {
            UUID uuid = new UUID(prng.nextLong(), prng.nextLong());
            String encoded = Encoder.encodeToString(uuid);
            assertEquals(uuid, Decoder.decodeUuid(encoded));
        }
    }

    @Test
    void testIntegersInvalidDecoding() {
        assertDoesNotThrow(() -> Decoder.decodeLong("ZZZZZZZZZZZ"));
        assertDoesNotThrow(() -> Decoder.decodeLong("00000000000"));
        assertDoesNotThrow(() -> Decoder.decodeLong("OOOOOOOOOOO"));
        assertDoesNotThrow(() -> Decoder.decodeLong("IIIIIIIIIII"));
        assertDoesNotThrow(() -> Decoder.decodeLong("lllllllllll"));
        assertDoesNotThrow(() -> Decoder.decodeLong("11111111111"));
        assertDoesNotThrow(() -> Decoder.decodeLong("01OlI01OlI0"));
        assertDoesNotThrow(() -> Decoder.decodeLong(ByteBuffer.wrap(
                new byte[] { -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11 }
        )));
        assertDoesNotThrow(() -> Decoder.decodeLong(ByteBuffer.wrap(
                new byte[] { 0x7D, 0x7E, 0x7F, -128, -127, -126, -125, -124, -123, -122, -121 }
        )));
        assertDoesNotThrow(() -> Decoder.decodeLong("\b\t\n\u000B\f\r\u001C\u001D\u001E\u001F "));
        assertDoesNotThrow(() -> Decoder.decodeLong("XWXWXWVVUVU"));
    }

    @TestFactory
    Collection<DynamicTest> testShortRandomBytes() {
        List<DynamicTest> tests = new ArrayList<>(DEFAULT_TESTS_NUMBER);
        Random prng = new Random(0x80858872834AA8D1L);
        for (int plainSize = 0; plainSize < DEFAULT_TESTS_NUMBER; ++plainSize) {
            int finalPlainSize1 = plainSize;
            tests.add(DynamicTest.dynamicTest(
                    "plain size of "  + finalPlainSize1 + " bytes",
                    () -> testRandomBytes(prng, finalPlainSize1))
            );
            int finalPlainSize2 = DEFAULT_TESTS_NUMBER - plainSize - 1;
            tests.add(DynamicTest.dynamicTest(
                    "plain size of "  + finalPlainSize2 + " bytes",
                    () -> testRandomBytes(prng, finalPlainSize2))
            );
        }
        return tests;
    }

    @TestFactory
    Collection<DynamicTest> testLongRandomBytes() {
        List<DynamicTest> tests = new ArrayList<>(LIMITED_TESTS_NUMBER);
        Random prng = new Random(0xB16D6FA325824008L);
        for (int t = 0; t < LIMITED_TESTS_NUMBER; ++t) {
            int plainSize = prng.nextInt(BLOB_MAX_SIZE);
            tests.add(DynamicTest.dynamicTest(
                    "plain size of "  + plainSize + " bytes",
                    () -> testRandomBytes(prng, plainSize))
            );
        }
        return tests;
    }

    private void testRandomBytes(Random prng, int plainSize) {
        byte[] plain = new byte[plainSize];
        prng.nextBytes(plain);
        byte[] encoded = Encoder.encode(ByteBuffer.wrap(plain));
        byte[] decoded = Decoder.decode(ByteBuffer.wrap(encoded));
        assertArrayEquals(plain, decoded);
    }

    @TestFactory
    Collection<DynamicTest> testShortStreams() {
        List<DynamicTest> tests = new ArrayList<>(LIMITED_TESTS_NUMBER);
        Random prng = new Random(0x131450F2D41522C1L);
        for (int t = 0; t < DEFAULT_TESTS_NUMBER; ++t) {
            int plainSize = t;
            tests.add(DynamicTest.dynamicTest(
                "stream of size " + plainSize, () -> testRandomStream(prng, plainSize)
            ));
        }
        return tests;
    }

    @TestFactory
    Collection<DynamicTest> testLongStreams() {
        List<DynamicTest> tests = new ArrayList<>(LIMITED_TESTS_NUMBER);
        Random prng = new Random(0x216EB112F64BCC08L);
        for (int t = 0; t < LIMITED_TESTS_NUMBER; ++t) {
            int plainSize = prng.nextInt(BLOB_MAX_SIZE);
            tests.add(DynamicTest.dynamicTest(
                "stream of size " + plainSize, () -> testRandomStream(prng, plainSize)
            ));
        }
        return tests;
    }

    static final int CHUNK = 1 << 16;
    private void testRandomStream(Random prng, int plainSize) throws IOException {
        byte[] plain = new byte[plainSize];
        prng.nextBytes(plain);
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        OutputStream es = Encoder.wrap(os);
        int i = 0;
        while (i < plainSize) {
            int length = prng.nextInt(Math.min(CHUNK, plain.length - i + 1));
            es.write(plain, i, length);
            i += length;
        }
        es.close();

        byte[] decoded = new byte[plainSize + CHUNK];
        InputStream is = new ByteArrayInputStream(os.toByteArray());
        InputStream ds = Decoder.wrap(is);
        i = 0;
        while (i < plainSize) {
            int length = prng.nextInt(CHUNK + 1);
            int read = ds.read(decoded, i, length);
            assertTrue(read >= 0, "ds.read() is " + read);
            i += read;
        }
        assertEquals(-1, ds.read(decoded, 0, decoded.length));
        ds.close();
        assertEquals(plainSize, i);
        assertTrue(Arrays.equals(plain, 0, plain.length, decoded, 0, i));
    }
}
