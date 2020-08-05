#include "base57.h"

#include <assert.h>


#define BASE 57
#define ENCODED_UINT64_SIZE 11
#define ENCODED_UINT64_PER_LINE 8

static const char ALPHABET[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";


static inline
uint64_t get_little_endian_uint64(const uint8_t* buffer, size_t buffer_length) {
    assert(buffer_length <= 8);
    uint64_t result = 0;
    while (buffer_length > 0) {
        result <<= 8;
        result |= (uint64_t)(buffer[--buffer_length]);
    }
    return result;
}


static inline
void put_little_endian_uint64(uint8_t* buffer, size_t buffer_length, uint64_t value) {
    assert(buffer_length <= sizeof(uint64_t));
    for (size_t i = 0; i < buffer_length; ++i) {
        buffer[i] = value & 0xFF;
        value >>= 8;
    }
    assert(value == 0);
}


#define LENGTH_OF(a) (sizeof(a) / sizeof((a)[0]))


#define INVALID_SYMBOL 99
#define CONTROL_SYMBOL 88
#define AUX_SYMBOL 77
/** 0, O, 1, l and I are excluded */
static const uint8_t SYMBOL_VALUES[128] = {
    //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
    //         \0,                         \a, \b, \t, \n, \v, \f, \r,
    /* 0x0x */ 88, 88, 88, 88, 88, 88, 88, 88, 99, 77, 77, 77, 77, 77, 88, 88, /* 0x0x */
    /* 0x1x */ 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, /* 0x1x */
    //           ,  !,  ",  #,  $,  %   &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,
    /* 0x2x */ 77, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 77, 77, 77, /* 0x2x */
    //          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,
    /* 0x3x */ 99, 99, 49, 50, 51, 52, 53, 54, 55, 56, 77, 99, 99, 99, 99, 99, /* 0x3x */
    //          @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
    /* 0x4x */ 99,  0,  1,  2,  3,  4,  5,  6,  7, 99,  8,  9, 10, 11, 12, 99, /* 0x4x */
    //          P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _,
    /* 0x5x */ 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 99, 77, 99, 99, 77, /* 0x5x */
    //          `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
    /* 0x6x */ 99, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 99, 35, 36, 37, /* 0x6x */
    //          p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },  ~,
    /* 0x7x */ 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 99, 99, 99, 99, 99  /* 0x7x */
    //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
};

static inline
uint8_t get_symbol_value(char symbol) {
    if ((uint8_t)symbol >= LENGTH_OF(SYMBOL_VALUES)) {
        return INVALID_SYMBOL;
    }
    return SYMBOL_VALUES[symbol];
}

static const uint8_t PLAIN_TO_ENCODED_LENGTH_MAPPING[sizeof(uint64_t) + 1] = {
    //  ceil(log(256^i, BASE))
    //  0, 1, 2, 3, 4, 5, 6,  7,  8
        0, 2, 3, 5, 6, 7, 9, 10, 11
};

static const uint8_t INVALID_LENGTH = 99;
static const uint8_t ENCODED_TO_PLAIN_LENGTH_MAPPING[11 + 1] = {
    //  0,  1, 2, 3,  4, 5, 6, 7,  8, 9, 10, 11
        0, 99, 1, 2, 99, 3, 4, 5, 99, 6,  7,  8
};


size_t base57_calculate_encoded_length(size_t plain_length) {
    size_t encoded_size
        = ENCODED_UINT64_SIZE * (plain_length / sizeof(uint64_t))
        + PLAIN_TO_ENCODED_LENGTH_MAPPING[plain_length % sizeof(uint64_t)];
    size_t line_separators = encoded_size / ENCODED_UINT64_PER_LINE / ENCODED_UINT64_SIZE;
    return encoded_size + line_separators + 1; // +1 for a terminating NUL
}


static inline
void encode_uint(char* output, size_t encoded_length, uint64_t input) {
    for (size_t i = 0; i < encoded_length; ++i) {
        output[i] = ALPHABET[input % BASE];
        input /= BASE;
    }
    assert(input == 0);
}


char* base57_encode(char* output, const uint8_t* input, size_t input_length) {
    size_t i = 0;
    size_t o = 0;
    size_t encoded_uint64s = 0;
    while (input_length - i >= sizeof(uint64_t)) {
        uint64_t uint64 = get_little_endian_uint64(input + i, sizeof(uint64_t));
        encode_uint(output + o, ENCODED_UINT64_SIZE, uint64);
        i += sizeof(uint64_t);
        o += ENCODED_UINT64_SIZE;
        if (++encoded_uint64s == ENCODED_UINT64_PER_LINE) {
            output[o++] = '\n';
            encoded_uint64s = 0;
        }
    }
    uint64_t uint64 = get_little_endian_uint64(input + i, input_length - i);
    encode_uint(output + o, PLAIN_TO_ENCODED_LENGTH_MAPPING[input_length - i], uint64);
    o += PLAIN_TO_ENCODED_LENGTH_MAPPING[input_length - i];
    output[o] = 0;
    return output;
}


size_t base57_calculate_decoded_max_length(size_t encoded_length) {
    size_t uint64s = encoded_length / ENCODED_UINT64_SIZE;
    size_t remains = encoded_length % ENCODED_UINT64_SIZE;
    if (ENCODED_TO_PLAIN_LENGTH_MAPPING[remains] == INVALID_LENGTH) {
        assert(remains > 0);
        remains -= 1;
    }
    assert(ENCODED_TO_PLAIN_LENGTH_MAPPING[remains] != INVALID_LENGTH);
    return uint64s * sizeof(uint64_t) + ENCODED_TO_PLAIN_LENGTH_MAPPING[remains];
}


static const uint64_t MAGNITUDES[ENCODED_UINT64_SIZE] = {
    1ull,
    1ull * BASE,
    1ull * BASE * BASE,
    1ull * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE,
    1ull * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE * BASE,
};


base57_DecodingResult base57_decode(uint8_t* output, const char* input, size_t input_length) {
    base57_DecodingResult result = { 0, 0, 0 };
    uint64_t value = 0;
    int m = 0;
    while (input_length - result.consumed_input_length > 0) {
        uint8_t digit = get_symbol_value(input[result.consumed_input_length]);
        switch (digit) {
        case INVALID_SYMBOL:
            result.termination_reason = base57_INVALID_SYMBOL;
            return result;
        case CONTROL_SYMBOL:
            result.termination_reason = base57_CONTROL_SYMBOL;
            return result;
        case AUX_SYMBOL:
            ++result.consumed_input_length;
            continue;
        default:
            assert(digit < BASE);
            ++result.consumed_input_length;
        }
        if (m < ENCODED_UINT64_SIZE - 1) {
            value += digit * MAGNITUDES[m++];
            continue;
        }
        // overflow check on highest digit:
        // 1) 50 * 57^10 < UINT64_MAX < 51 * 57^10
        // 2) value + digit * MAGNITUDES[m] <= UINT64_MAX
        //    value <= UINT64_MAX - digit * MAGNITUDES[m]
        if (digit >= 50) { // quick path
            if (digit > 50 || value > UINT64_MAX - digit * MAGNITUDES[m]) {
                result.termination_reason = base57_OVERFLOW;
                return result;
            }
        }
        value += digit * MAGNITUDES[m];
        put_little_endian_uint64(output + result.output_length, sizeof(uint64_t), value);
        result.output_length += sizeof(uint64_t);
        value = 0;
        m = 0;
    }
    uint8_t chunk_length = ENCODED_TO_PLAIN_LENGTH_MAPPING[m];
    if (chunk_length == INVALID_LENGTH) {
        result.termination_reason = base57_TRUNCATED;
        return result;
    }
    // overflow check
    if (value >> (8 * chunk_length) > 0) {
        result.termination_reason = base57_OVERFLOW;
        return result;
    }
    put_little_endian_uint64(output + result.output_length, chunk_length, value);
    result.output_length += chunk_length;
    result.termination_reason = base57_NO_INPUT;
    return result;
}
