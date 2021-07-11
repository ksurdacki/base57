#include "base57.h"

#include <assert.h>
#include <string.h>


const char* const base57_version = "0.1.0";


#define BASE 57
#define LENGTH_OF(a) (sizeof(a) / sizeof((a)[0]))


const char SYMBOLS[8*BASE] =
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
        "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
    ;


static inline
uint64_t get_little_endian_uint(const uint8_t* bytes, size_t length) {
    assert(length <= sizeof(uint64_t));
    uint64_t result = 0;
    while (length > 0) {
        result <<= 8;
        result |= (uint64_t)(bytes[--length]);
    }
    return result;
}


static const uint8_t BYTE_ORDER_TEST[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
static const uint64_t LITTLE_ENDIAN = 0x7766554433221100ull;


static inline
uint64_t get_little_endian_uint64(const uint8_t bytes[sizeof(uint64_t)]) {
    if (*(uint64_t*)BYTE_ORDER_TEST == LITTLE_ENDIAN) {
        return *(uint64_t*)bytes;
    }
    return ((uint64_t)bytes[0] <<  0)
         | ((uint64_t)bytes[1] <<  8)
         | ((uint64_t)bytes[2] << 16)
         | ((uint64_t)bytes[3] << 24)
         | ((uint64_t)bytes[4] << 32)
         | ((uint64_t)bytes[5] << 40)
         | ((uint64_t)bytes[6] << 48)
         | ((uint64_t)bytes[7] << 56)
         ; 
}


static inline
void put_little_endian_uint(uint8_t* bytes, size_t length, uint64_t value) {
    assert(length <= sizeof(uint64_t));
    for (size_t i = 0; i < length; ++i) {
        bytes[i] = value & 0xFF;
        value >>= 8;
    }
    assert(value == 0);
}


static inline
void put_little_endian_uint64(uint8_t bytes[sizeof(uint64_t)], uint64_t value) {
    if (*(uint64_t*)BYTE_ORDER_TEST == LITTLE_ENDIAN) {
        *(uint64_t*)bytes = value;
        return;
    }
    bytes[0] = (uint8_t)(value >> 0);
    bytes[1] = (uint8_t)(value >>  8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
    bytes[4] = (uint8_t)(value >> 32);
    bytes[5] = (uint8_t)(value >> 40);
    bytes[6] = (uint8_t)(value >> 48);
    bytes[7] = (uint8_t)(value >> 56);
}


static const uint8_t PLAIN_TO_ENCODED_LENGTH_MAPPING[sizeof(uint64_t) + 1] = {
    //  0, 1, 2, 3, 4, 5, 6,  7,  8
        0, 2, 3, 5, 6, 7, 9, 10, 11
};


static const uint8_t ENCODED_TO_PLAIN_LENGTH_MAPPING[base57_ENCODED_UINT64_SIZE + 1] = {
    //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
        0, 1, 1, 2, 3, 3, 4, 5, 6, 6,  7,  8
};


#define ENCODED_UINT64S_PER_LINE 8


size_t base57_calculate_encoded_length(size_t plain_length) {
    if (plain_length == 0) {
        return 0;
    }
    size_t encoded_size
        = base57_ENCODED_UINT64_SIZE * (plain_length / sizeof(uint64_t))
        + PLAIN_TO_ENCODED_LENGTH_MAPPING[plain_length % sizeof(uint64_t)];
    size_t line_separators = (encoded_size - 1) / base57_ENCODED_UINT64_SIZE / ENCODED_UINT64S_PER_LINE;
    return encoded_size + line_separators;
}


char* base57_encode_uint64(char output[base57_ENCODED_UINT64_SIZE + 1], uint64_t input) {
    uint64_t value;
    uint64_t shift = 0;

    #define STEP57(I) do { \
        value = input % 57; \
        input /= 57; \
        assert(shift + value < LENGTH_OF(SYMBOLS)); \
        output[I] = SYMBOLS[shift + value]; \
    } while (false)

    #define STEP56(I) do { \
        shift += value + 1; \
        value = input % 56; \
        input /= 56; \
        assert(shift + value < LENGTH_OF(SYMBOLS)); \
        output[I] = SYMBOLS[shift + value]; \
    } while (false)

    STEP57(0);
    STEP56(1);
    STEP57(2);
    STEP56(3);
    STEP57(4);
    STEP56(5);
    STEP56(6);
    STEP57(7);
    STEP56(8);
    STEP57(9);
    STEP56(10);
    output[11] = 0;
    return output;

    #undef STEP56
    #undef STEP57
}


char* base57_encode(char* output, const uint8_t* input, size_t input_length) {
    char * const initial_output = output;
    while (input_length > ENCODED_UINT64S_PER_LINE * sizeof(uint64_t)) {
        for (int i = 0; i < ENCODED_UINT64S_PER_LINE; ++i) {
            base57_encode_uint64(output, get_little_endian_uint64(input));
            input += sizeof(uint64_t);
            output += base57_ENCODED_UINT64_SIZE;
        }
        input_length -= ENCODED_UINT64S_PER_LINE * sizeof(uint64_t);
        *(output++) = '\n';
    }
    while (input_length >= sizeof(uint64_t)) {
        base57_encode_uint64(output, get_little_endian_uint64(input));
        input += sizeof(uint64_t);
        input_length -= sizeof(uint64_t);
        output += base57_ENCODED_UINT64_SIZE;
    }
    if (input_length > 0) {
        char buffer[base57_ENCODED_UINT64_SIZE + 1];
        base57_encode_uint64(buffer, get_little_endian_uint(input, input_length));
        memcpy(output, buffer, PLAIN_TO_ENCODED_LENGTH_MAPPING[input_length]);
        output += PLAIN_TO_ENCODED_LENGTH_MAPPING[input_length];
    }
    *output = 0;
    return initial_output;
}


static const uint64_t MAGNITUDES[base57_ENCODED_UINT64_SIZE] = {
    1ull,
    1ull * 57,
    1ull * 57 * 56,
    1ull * 57 * 56 * 57,
    1ull * 57 * 56 * 57 * 56,
    1ull * 57 * 56 * 57 * 56 * 57,
    1ull * 57 * 56 * 57 * 56 * 57 * 56,
    1ull * 57 * 56 * 57 * 56 * 57 * 56 * 56,
    1ull * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57,
    1ull * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57 * 56,
    1ull * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57 * 56 * 57,
//     A    r    A    r    A    r    r    A    r    A    r
};


#define DELIMITER 57

/** 0, O, 1, l and I are invalid */
static const uint8_t SYMBOL_VALUES[256] = {
    //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
    //         \0,                         \a, \b, \t, \n, \v, \f, \r,
    /* 0x0x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 57, 57, 57, 57, 57, 99, 99, /* 0x0x */
    /* 0x1x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 57, 57, 57, 57, /* 0x1x */
    //           ,  !,  ",  #,  $,  %   &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,
    /* 0x2x */ 57, 99, 99, 99, 99, 99, 99, 57, 99, 99, 99, 57, 57, 57, 57, 57, /* 0x2x */
    //          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,
    /* 0x3x */ 99, 99,  2,  3,  4,  5,  6,  7,  8,  9, 57, 57, 99, 99, 99, 99, /* 0x3x */
    //          @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
    /* 0x4x */ 99, 35, 36, 37, 38, 39, 40, 41, 42, 99, 43, 44, 45, 46, 47, 99, /* 0x4x */
    //          P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _,
    /* 0x5x */ 48, 49, 50, 51, 52, 53, 54, 55, 56,  1,  0, 99, 57, 99, 99, 57, /* 0x5x */
    //          `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
    /* 0x6x */ 57, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 99, 21, 22, 23, /* 0x6x */
    //          p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },  ~,
    /* 0x7x */ 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 99, 99, 99, 99, 99, /* 0x7x */
    //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
    /* 0x8x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0x8x */
    /* 0x9x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0x9x */
    /* 0xAx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xAx */
    /* 0xBx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xBx */
    /* 0xCx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xCx */
    /* 0xDx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xDx */
    /* 0xEx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xEx */
    /* 0xFx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99  /* 0xFx */
};


static const uint8_t REMAINDERS_OF_57[10 * 57] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,
};


uint64_t base57_decode_uint64(char input[base57_ENCODED_UINT64_SIZE]) {
    static const uint64_t L = LENGTH_OF(REMAINDERS_OF_57) - 2 * 57;
    uint64_t svalue;
    uint64_t value;
    uint64_t result = 0;
    uint64_t shift = 0;

    #define STEP57(I) do { \
        svalue = SYMBOL_VALUES[(uint8_t)input[I]]; \
        value = REMAINDERS_OF_57[L + svalue - shift]; \
        result += value * MAGNITUDES[I]; \
    } while (false)

    #define STEP56(I) do { \
        shift += value + 1; \
        svalue = SYMBOL_VALUES[(uint8_t)input[I]]; \
        assert(L + svalue - shift < LENGTH_OF(REMAINDERS_OF_57)); \
        value = REMAINDERS_OF_57[L + svalue - shift]; \
        result += value * MAGNITUDES[I]; \
    } while (false)

    STEP57(0);
    STEP56(1);
    STEP57(2);
    STEP56(3);
    STEP57(4);
    STEP56(5);
    STEP56(6);
    STEP57(7);
    STEP56(8);
    STEP57(9);
    STEP56(10);
    return result;

    #undef STEP56
    #undef STEP57
}


size_t base57_calculate_decoded_max_length(size_t encoded_length) {
    size_t uint64s = encoded_length / base57_ENCODED_UINT64_SIZE;
    size_t remains = encoded_length % base57_ENCODED_UINT64_SIZE;
    return uint64s * sizeof(uint64_t) + ENCODED_TO_PLAIN_LENGTH_MAPPING[remains];
}


void base57_decode_part(
    uint8_t** output, base57_DecodingBuffer* buffer, const char** input, size_t* input_length
) {
    while (*input_length > 0) {
        uint8_t v = SYMBOL_VALUES[(uint8_t)**input];
        if (v >= BASE) {
            if (v != DELIMITER) {
                return;
            }
            *input += 1;
            *input_length -= 1;
            continue;
        }
        assert(buffer->symbols_number < base57_ENCODED_UINT64_SIZE);
        buffer->symbols[buffer->symbols_number++] = **input;
        *input += 1;
        *input_length -= 1;
        if (buffer->symbols_number >= base57_ENCODED_UINT64_SIZE) {
            put_little_endian_uint64(*output, base57_decode_uint64(buffer->symbols));
            buffer->symbols_number = 0;
            *output += sizeof(uint64_t);
        }
    }
}


void base57_flush_decoding_buffer(uint8_t** output, base57_DecodingBuffer* buffer) {
    assert(buffer->symbols_number < base57_ENCODED_UINT64_SIZE);
    if (buffer->symbols_number > 0) {
        uint64_t svalue = SYMBOL_VALUES[buffer->symbols[buffer->symbols_number - 1]];
        assert(svalue < BASE);
        for (uint8_t i = buffer->symbols_number; i < base57_ENCODED_UINT64_SIZE; ++i) {
            buffer->symbols[i] = SYMBOLS[++svalue];
        }
        uint64_t value = base57_decode_uint64(buffer->symbols) % MAGNITUDES[buffer->symbols_number];
        put_little_endian_uint(*output, ENCODED_TO_PLAIN_LENGTH_MAPPING[buffer->symbols_number], value);
        *output += ENCODED_TO_PLAIN_LENGTH_MAPPING[buffer->symbols_number];
    }
    memset(buffer, 0, sizeof(*buffer));
}
