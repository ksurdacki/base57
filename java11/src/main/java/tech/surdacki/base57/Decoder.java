package tech.surdacki.base57;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Objects;
import java.util.UUID;

import static tech.surdacki.base57.Constants.*;

public class Decoder {
    public static long decodeLong(ByteBuffer src) {
        long symbolValue;
        long shiftedValue;
        long result = 0;
        int shift = 0;

        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[0];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[1];

        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[2];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[3];

        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[4];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[5];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[6];

        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[7];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[8];

        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[9];

        shift += shiftedValue + 1;
        symbolValue = SYMBOL_VALUES[0xFF & (int)src.get()];
        shiftedValue = (570 + symbolValue - shift) % 57;
        result += shiftedValue * MAGNITUDES[10];

        return result;
    }

    public static long decodeLong(String src) {
        byte[] bytes = src.getBytes(StandardCharsets.US_ASCII);
        return decodeLong(ByteBuffer.wrap(bytes));
    }

    public static UUID decodeUuid(ByteBuffer src) {
        long lo = decodeLong(src);
        long hi = decodeLong(src);
        return new UUID(hi, lo);
    }

    public static UUID decodeUuid(String src) {
        byte[] bytes = src.getBytes(StandardCharsets.US_ASCII);
        return decodeUuid(ByteBuffer.wrap(bytes));
    }

    public static int calcDecodedMaxLength(int encodedLength) {
        int longs = encodedLength / ENCODED_LONG_LENGTH;
        int remain = encodedLength %  ENCODED_LONG_LENGTH;
        return longs * Long.BYTES + ENCODED_TO_PLAIN_LENGTH_MAPPING[remain];
    }

    public static void decode(ByteBuffer src, ByteBuffer dst) {
        dst.order(ByteOrder.LITTLE_ENDIAN);
        ByteBuffer buffer = ByteBuffer.allocate(ENCODED_LONG_LENGTH);
        while (src.hasRemaining()) {
            putSymbolOnly(src.get(), buffer);
            if (!buffer.hasRemaining()) {
                dst.putLong(decodeLong(buffer.flip()));
                buffer.clear();
            }
        }
        if (buffer.position() > 0) {
            flushBuffer(buffer, dst);
        }
    }

    public static byte[] decode(ByteBuffer src) {
        ByteBuffer dst = ByteBuffer.allocate(calcDecodedMaxLength(src.remaining()));
        decode(src, dst);
        return Arrays.copyOfRange(dst.array(), 0, dst.position());
    }

    public static byte[] decode(String src) {
        byte[] bytes = src.getBytes(StandardCharsets.US_ASCII);
        return decode(ByteBuffer.wrap(bytes));
    }

    public static InputStream wrap(InputStream is) {
        return new DecodingInputStream(is);
    }

    static void putSymbolOnly(byte symbol, ByteBuffer buffer) {
        byte symbol_value = SYMBOL_VALUES[0xFF & symbol];
        if (symbol_value < 57) {
            buffer.put(symbol);
            return;
        }
        if (symbol_value != DELIMITER_VALUE) {
            throw new IllegalArgumentException(String.format(
                    "Non Base57 symbol 0x%02X encountered.", symbol
            ));
        }
    }

    static void flushBuffer(ByteBuffer buffer, ByteBuffer dst) {
        int encoded_length = buffer.position();
        padInputBuffer(buffer);
        long decoded = decodeLong(buffer.flip()) % MAGNITUDES[encoded_length];
        for (int dl = ENCODED_TO_PLAIN_LENGTH_MAPPING[encoded_length]; dl > 0; --dl) {
            dst.put((byte)decoded);
            decoded >>= 8;
        }
    }

    static void padInputBuffer(ByteBuffer buffer) {
        byte symbol_value = SYMBOL_VALUES[buffer.get(buffer.position() - 1)];
        while (buffer.hasRemaining()) {
            buffer.put(SYMBOLS[symbol_value += 1]);
        }
    }

    public static final class DecodingInputStream extends InputStream {

        final InputStream is;
        final ByteBuffer readBuffer = ByteBuffer.allocate(0x8000);
        final ByteBuffer decodingBuffer = ByteBuffer.allocate(ENCODED_LONG_LENGTH);
        long outputValue;
        int outputBytes = 0;

        public DecodingInputStream(InputStream is) {
            this.is = is;
            readBuffer.position(readBuffer.limit());
        }

        @Override
        public int read() throws IOException {
            if (outputBytes == 0) {
                decode();
                if (outputBytes == 0) {
                    return -1;
                }
            }
            int result = (int)outputValue & 0xFF;
            outputValue >>= 8;
            outputBytes -= 1;
            return result;
        }

        @Override
        public int read(byte[] b, int off, int len) throws IOException {
            Objects.checkFromIndexSize(off, len, b.length);
            int i = off;
            int end = off + len;
            while (i < end) {
                if (outputBytes == 0) {
                    decode();
                    if (outputBytes == 0) {
                        return i > off ? i - off : -1;
                    }
                }
                b[i++] = (byte)outputValue;
                outputValue >>>= 8;
                outputBytes -= 1;
            }
            return i - off;
        }

        @Override
        public void close() throws IOException {
            is.close();
        }

        void decode() throws IOException {
            fillDecodingBuffer();
            if (decodingBuffer.hasRemaining()) {
                flushDecodingBuffer();
            } else {
                decodingBuffer.flip();
                outputValue = decodeLong(decodingBuffer);
                outputBytes = Long.BYTES;
            }
            decodingBuffer.clear();
        }

        void fillDecodingBuffer() throws IOException {
            while (decodingBuffer.hasRemaining()) {
                if (!readBuffer.hasRemaining()) {
                    int read = is.read(readBuffer.array());
                    if (read < 0) {
                        return;
                    }
                    readBuffer.position(0);
                    readBuffer.limit(read);
                }
                putSymbolOnly(readBuffer.get(), decodingBuffer);
            }
        }

        void flushDecodingBuffer() {
            if (decodingBuffer.position() > 0) {
                int encoded_length = decodingBuffer.position();
                padInputBuffer(decodingBuffer);
                decodingBuffer.flip();
                outputValue = decodeLong(decodingBuffer) % MAGNITUDES[encoded_length];
                outputBytes = ENCODED_TO_PLAIN_LENGTH_MAPPING[encoded_length];
            }
        }
    }
}
