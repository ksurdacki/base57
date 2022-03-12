package tech.surdacki.base57;

import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

import static tech.surdacki.base57.Constants.*;

public class Encoder {

    public static void encode(long n, ByteBuffer dst) {
        int value;
        int shift = 0;

        value = (int)Long.remainderUnsigned(n, 57);
        n = Long.divideUnsigned(n, 57);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        n = Long.divideUnsigned(n, 56);
        dst.put(SYMBOLS[shift + value]);

        value = (int)Long.remainderUnsigned(n, 57);
        n = Long.divideUnsigned(n, 57);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        n = Long.divideUnsigned(n, 56);
        dst.put(SYMBOLS[shift + value]);

        value = (int)Long.remainderUnsigned(n, 57);
        n = Long.divideUnsigned(n, 57);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        n = Long.divideUnsigned(n, 56);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        n = Long.divideUnsigned(n, 56);
        dst.put(SYMBOLS[shift + value]);

        value = (int)Long.remainderUnsigned(n, 57);
        n = Long.divideUnsigned(n, 57);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        n = Long.divideUnsigned(n, 56);
        dst.put(SYMBOLS[shift + value]);

        value = (int)Long.remainderUnsigned(n, 57);
        n = Long.divideUnsigned(n, 57);
        dst.put(SYMBOLS[shift + value]);

        shift += value + 1;
        value = (int)Long.remainderUnsigned(n, 56);
        assert n < 56;
        dst.put(SYMBOLS[shift + value]);
    }

    public static byte[] encode(long n) {
        byte[] result = new byte[ENCODED_LONG_LENGTH];
        encode(n, ByteBuffer.wrap(result));
        return result;
    }

    public static String encodeToString(long n) {
        return new String(encode(n), StandardCharsets.US_ASCII);
    }

    public static void encode(UUID uuid, ByteBuffer dst) {
        encode(uuid.getLeastSignificantBits(), dst);
        encode(uuid.getMostSignificantBits(), dst);
    }

    public static byte[] encode(UUID uuid) {
        byte[] result = new byte[2 * ENCODED_LONG_LENGTH];
        encode(uuid, ByteBuffer.wrap(result));
        return result;
    }

    public static String encodeToString(UUID uuid) {
        return new String(encode(uuid), StandardCharsets.US_ASCII);
    }

    public static long calcEncodedLength(int plainLength) {
        if (plainLength <= 0) {
            if (plainLength == 0) {
                return 0;
            } else {
                throw new IllegalArgumentException("plainLength < 0");
            }
        }
        long encodedLength = (long) ENCODED_LONG_LENGTH * (plainLength / Long.BYTES)
                + PLAIN_TO_ENCODED_LENGTH_MAPPING[plainLength % Long.BYTES];
        long lineSeparators = (encodedLength - 1) / ENCODED_LONG_LENGTH / ENCODED_LONGS_PER_LINE;
        return encodedLength + lineSeparators;
    }

    public static void encode(ByteBuffer src, ByteBuffer dst) {
        src.order(ByteOrder.LITTLE_ENDIAN);
        while (src.remaining() > DECODED_LINE_LENGTH) {
            for (int i = 0; i < ENCODED_LONGS_PER_LINE; i += 1) {
                encode(src.getLong(), dst);
            }
            dst.put((byte)'\n');
        }
        while (src.remaining() >= Long.BYTES) {
            encode(src.getLong(), dst);
        }
        if (src.hasRemaining()) {
            encodeRemaining(src, dst);
        }
    }

    public static byte[] encode(ByteBuffer src) {
        long encodedLength = calcEncodedLength(src.remaining());
        if (encodedLength > Integer.MAX_VALUE) {
            throw new OutOfMemoryError(
                    "Cannot allocate so huge array for an encoded data: encodedLength > Integer.MAX_VALUE"
            );
        }
        byte[] result = new byte[(int)encodedLength];
        encode(src, ByteBuffer.wrap(result));
        return result;
    }

    public static String encodeToString(ByteBuffer src) {
        return new String(encode(src), StandardCharsets.US_ASCII);
    }

    static void encodeRemaining(ByteBuffer src, ByteBuffer dst) {
        assert src.remaining() < Long.BYTES : src.remaining();
        int length = src.remaining();
        long n = 0;
        int shift = 0;
        while (src.hasRemaining()) {
            n |= ((long)src.get() & 0xFF) << shift;
            shift += 8;
        }
        byte[] buffer = encode(n);
        for (int i = 0; i < PLAIN_TO_ENCODED_LENGTH_MAPPING[length]; i += 1) {
            dst.put(buffer[i]);
        }
    }

    public static OutputStream wrap(OutputStream os) {
        return new EncodingOutputStream(os);
    }

    public static final class EncodingOutputStream extends FilterOutputStream {

        final ByteBuffer inputBuffer = ByteBuffer.allocate(DECODED_LINE_LENGTH);
        final ByteBuffer outputBuffer = ByteBuffer.allocate(ENCODED_LINE_LENGTH);

        public EncodingOutputStream(OutputStream os) {
            super(os);
            inputBuffer.order(ByteOrder.LITTLE_ENDIAN);
            outputBuffer.put(ENCODED_LINE_LENGTH - 1, (byte)'\n');
        }

        @Override
        public void write(int b) throws IOException {
            if (!inputBuffer.hasRemaining()) {
                writeLine();
            }
            inputBuffer.put((byte)b);
        }

        @Override
        public void write(byte[] bytes, int off, int len) throws IOException {
            ByteBuffer src = ByteBuffer.wrap(bytes, off, len);
            src.order(ByteOrder.LITTLE_ENDIAN);
            while (true) {
                int chunk = Math.min(inputBuffer.remaining(), len);
                inputBuffer.put(bytes, off, chunk);
                off += chunk;
                len -= chunk;
                if (len == 0) {
                    break;
                }
                writeLine();
            }
        }

        @Override
        public void close() throws IOException {
            try {
                if (inputBuffer.position() == 0) {
                    out.close();
                    return;
                }
                inputBuffer.flip();
                outputBuffer.clear();
                while (inputBuffer.remaining() >= Long.BYTES) {
                    encode(inputBuffer.getLong(), outputBuffer);
                }
                encodeRemaining(inputBuffer, outputBuffer);
                out.write(outputBuffer.array(), 0, outputBuffer.position());
                out.close();
            } finally {
                erase(); // clears potentially sensitive data
            }
        }

        void writeLine() throws IOException {
            assert ! inputBuffer.hasRemaining() : inputBuffer.position();
            inputBuffer.flip();
            outputBuffer.clear();
            for (int n = 0; n < ENCODED_LONGS_PER_LINE; ++n) {
                encode(inputBuffer.getLong(), outputBuffer);
            }
            assert outputBuffer.position() == ENCODED_LINE_LENGTH - 1 : outputBuffer.position();
            out.write(outputBuffer.array());
            inputBuffer.clear();
        }

        void erase() {
            inputBuffer.clear();
            outputBuffer.clear();
            for (int n = 0; n < ENCODED_LONGS_PER_LINE; ++n) {
                inputBuffer.putLong(0);
                outputBuffer.put(
                        "\u0000".repeat(ENCODED_LONG_LENGTH).getBytes(StandardCharsets.US_ASCII)
                );
            }
        }
    }
}
