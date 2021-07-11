#include "base57.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


/** Donald Knuth's Linear Congruential Generator */
static inline
uint64_t lcg(uint64_t *state) {
    *state = 6364136223846793005ull * *state + 1442695040888963407ull;
    return *state;
}


static unsigned passed_tests = 0;
static unsigned failed_tests = 0;

static void increment_failed_tests() {
    ++failed_tests; // a good place for putting a breakpoint
}


static FILE* output_stream;


#define TEST(EXP) do { \
    if (EXP) { \
        ++passed_tests; \
    } \
    else { \
        fprintf(output_stream, "%s@%u failed! (", __func__, __LINE__); \
        fputs(#EXP ") is false\n", output_stream); \
        increment_failed_tests(); \
    } \
} while (false)


#define TEST_UINT_RELATION(EXPECTED, RELATION, ACTUAL) do { \
    uint64_t expected = (EXPECTED); \
    uint64_t actual = (ACTUAL); \
    if (expected RELATION actual) { \
        ++passed_tests; \
    } \
    else { \
        fprintf(output_stream, "%s@%u failed !(", __func__, __LINE__); \
        fputs(#EXPECTED " " #RELATION " " #ACTUAL ") evaluates as !(", output_stream); \
        fprintf(output_stream, "%" PRIu64 " " #RELATION " %" PRIu64 ")\n", expected, actual); \
        increment_failed_tests(); \
    } \
} while (false)


#define TEST_UINT_EQUALITY(EXPECTED, ACTUAL) TEST_UINT_RELATION(EXPECTED, ==, ACTUAL)


static const uint64_t TEST_INTEGERS[] = {
    1ull, 2ull, 3ull, 4ull, 5ull, 6ull, 7ull, 8ull, 9ull, 10ull, 11ull, 12ull,
    13ull, 14ull, 15ull, 16ull, 17ull, 18ull, 19ull, 20ull, 21ull, 22ull, 23ull,
    24ull, 25ull, 26ull, 27ull, 28ull, 29ull, 30ull, 31ull, 32ull, 33ull, 34ull,
    35ull, 36ull, 37ull, 38ull, 39ull, 40ull, 41ull, 42ull, 43ull, 44ull, 45ull,
    46ull, 47ull, 48ull, 49ull, 50ull, 51ull, 52ull, 53ull, 54ull, 55ull, 56ull,
    57ull, 58ull, 59ull, 60ull, 61ull, 62ull, 63ull, 64ull, 65ull, 66ull, 67ull,
    68ull, 69ull, 70ull, 71ull, 72ull, 73ull, 74ull, 75ull, 76ull, 77ull, 78ull,
    79ull, 80ull, 81ull, 82ull, 83ull, 84ull, 85ull, 86ull, 87ull, 88ull, 89ull,
    90ull, 91ull, 92ull, 93ull, 94ull, 95ull, 96ull, 97ull, 98ull, 99ull,
    100ull, 101ull, 102ull, 103ull, 104ull, 105ull, 106ull, 107ull, 108ull,
    109ull, 110ull, 111ull, 112ull, 113ull, 114ull, 115ull, 116ull, 117ull,
    118ull, 119ull, 120ull, 121ull, 122ull, 123ull, 124ull, 125ull, 126ull,
    127ull, 128ull, 129ull, 168ull, 180ull, 255ull, 256ull, 257ull, 360ull,
    365ull, 999ull, 1000ull, 9999ull, 10000ull, 3600ull, 86400ull, 604800ull,
    511ull, 1023ull, 2047ull, 4095ull, 8191ull, 16383ull, 32767ull, 65535ull,
    131071ull, 262143ull, 524287ull, 1048575ull, 2097151ull, 4194303ull,
    8388607ull, 16777215ull, 33554431ull, 67108863ull, 134217727ull,
    268435455ull, 536870911ull, 1073741823ull, 2147483647ull, 4294967295ull,
    8589934591ull, 17179869183ull, 34359738367ull, 68719476735ull,
    137438953471ull, 274877906943ull, 549755813887ull, 1099511627775ull,
    2199023255551ull, 4398046511103ull, 8796093022207ull, 17592186044415ull,
    35184372088831ull, 70368744177663ull, 140737488355327ull,
    281474976710655ull, 562949953421311ull, 1125899906842623ull,
    2251799813685247ull, 4503599627370495ull, 9007199254740991ull,
    18014398509481983ull, 36028797018963967ull, 72057594037927935ull,
    144115188075855871ull, 288230376151711743ull, 576460752303423487ull,
    1152921504606846975ull, 2305843009213693951ull, 4611686018427387903ull,
    9223372036854775807ull, 18446744073709551615ull, 256ull, 512ull, 1024ull,
    2048ull, 4096ull, 8192ull, 16384ull, 32768ull, 65536ull, 131072ull,
    262144ull, 524288ull, 1048576ull, 2097152ull, 4194304ull, 8388608ull,
    16777216ull, 33554432ull, 67108864ull, 134217728ull, 268435456ull,
    536870912ull, 1073741824ull, 2147483648ull, 4294967296ull, 8589934592ull,
    17179869184ull, 34359738368ull, 68719476736ull, 137438953472ull,
    274877906944ull, 549755813888ull, 1099511627776ull, 2199023255552ull,
    4398046511104ull, 8796093022208ull, 17592186044416ull, 35184372088832ull,
    70368744177664ull, 140737488355328ull, 281474976710656ull,
    562949953421312ull, 1125899906842624ull, 2251799813685248ull,
    4503599627370496ull, 9007199254740992ull, 18014398509481984ull,
    36028797018963968ull, 72057594037927936ull, 144115188075855872ull,
    288230376151711744ull, 576460752303423488ull, 1152921504606846976ull,
    2305843009213693952ull, 4611686018427387904ull, 9223372036854775808ull,
    257ull, 513ull, 1025ull, 2049ull, 4097ull, 8193ull, 16385ull, 32769ull,
    65537ull, 131073ull, 262145ull, 524289ull, 1048577ull, 2097153ull,
    4194305ull, 8388609ull, 16777217ull, 33554433ull, 67108865ull,
    134217729ull, 268435457ull, 536870913ull, 1073741825ull, 2147483649ull,
    4294967297ull, 8589934593ull, 17179869185ull, 34359738369ull,
    68719476737ull, 137438953473ull, 274877906945ull, 549755813889ull,
    1099511627777ull, 2199023255553ull, 4398046511105ull, 8796093022209ull,
    17592186044417ull, 35184372088833ull, 70368744177665ull, 140737488355329ull,
    281474976710657ull, 562949953421313ull, 1125899906842625ull,
    2251799813685249ull, 4503599627370497ull, 9007199254740993ull,
    18014398509481985ull, 36028797018963969ull, 72057594037927937ull,
    144115188075855873ull, 288230376151711745ull, 576460752303423489ull,
    1152921504606846977ull, 2305843009213693953ull, 4611686018427387905ull,
    9223372036854775809ull, 32749ull, 32719ull, 32717ull, 32713ull, 32707ull,
    32693ull, 32687ull, 32653ull, 32647ull, 32633ull, 65521ull, 65519ull,
    65497ull, 65479ull, 65449ull, 65447ull, 65437ull, 65423ull, 65419ull,
    65413ull, 2147483647ull, 2147483629ull, 2147483587ull, 2147483579ull,
    2147483563ull, 2147483549ull, 2147483543ull, 2147483497ull, 2147483489ull,
    2147483477ull, 4294967291ull, 4294967279ull, 4294967231ull, 4294967197ull,
    4294967189ull, 4294967161ull, 4294967143ull, 4294967111ull, 4294967087ull,
    4294967029ull, 281474976710597ull, 281474976710591ull, 281474976710567ull,
    281474976710563ull, 281474976710509ull, 281474976710491ull,
    281474976710467ull, 281474976710423ull, 281474976710413ull,
    281474976710399ull, 1152921504606846883ull, 1152921504606846869ull,
    1152921504606846803ull, 1152921504606846797ull, 1152921504606846719ull,
    1152921504606846697ull, 1152921504606846607ull, 1152921504606846581ull,
    1152921504606846577ull, 1152921504606846523ull, 2305843009213693951ull,
    2305843009213693921ull, 2305843009213693907ull, 2305843009213693723ull,
    2305843009213693693ull, 2305843009213693669ull, 2305843009213693613ull,
    2305843009213693561ull, 2305843009213693549ull, 2305843009213693487ull,
    4611686018427387847ull, 4611686018427387817ull, 4611686018427387787ull,
    4611686018427387761ull, 4611686018427387751ull, 4611686018427387737ull,
    4611686018427387733ull, 4611686018427387709ull, 4611686018427387701ull,
    4611686018427387631ull, 9223372036854775783ull, 9223372036854775643ull,
    9223372036854775549ull, 9223372036854775507ull, 9223372036854775433ull,
    9223372036854775421ull, 9223372036854775417ull, 9223372036854775399ull,
    9223372036854775351ull, 9223372036854775337ull, 18446744073709551557ull,
    18446744073709551533ull, 18446744073709551521ull, 18446744073709551437ull,
    18446744073709551427ull, 18446744073709551359ull, 18446744073709551337ull,
    18446744073709551293ull, 18446744073709551263ull, 18446744073709551253ull,
    0 // sentinel
};



static void test_uint64_encoding_invariance() {
    char encoded[base57_ENCODED_UINT64_SIZE + 1];

    TEST(strcmp(base57_encode_uint64(encoded, 0), "ZYY22344556") == 0);
    TEST_UINT_EQUALITY(0, base57_decode_uint64("ZYY22344556"));

    TEST(strcmp(base57_encode_uint64(encoded, 0x0123456789ABCDEFull), "pX8jCDYpvEF") == 0);
    TEST_UINT_EQUALITY(0x0123456789ABCDEFull, base57_decode_uint64("pX8jCDYpvEF"));

    TEST(strcmp(base57_encode_uint64(encoded, 0xFEDCBA9876543210ull), "vgUKGEG53ut") == 0);
    TEST_UINT_EQUALITY(0xFEDCBA9876543210ull, base57_decode_uint64("vgUKGEG53ut"));

    TEST(strcmp(base57_encode_uint64(encoded, ~0ull-58), "Ud36kjFsx98") == 0);
    TEST_UINT_EQUALITY(~0ull - 58, base57_decode_uint64("Ud36kjFsx98"));

    TEST(strcmp(base57_encode_uint64(encoded, ~0ull), "Vf47mkGtya9") == 0);
    TEST_UINT_EQUALITY(~0ull, base57_decode_uint64("Vf47mkGtya9"));
}


static void test_uint64_encoding() {
    char encoded[base57_ENCODED_UINT64_SIZE + 1];
    for (int i = 0; TEST_INTEGERS[i] != 0; ++i) {
        base57_encode_uint64(encoded, TEST_INTEGERS[i]);
        TEST_UINT_EQUALITY(TEST_INTEGERS[i], base57_decode_uint64(encoded));
    }
}


static void test_invalid_uint64_decoding() {
    base57_decode_uint64("ZZZZZZZZZZZ");
    base57_decode_uint64("00000000000");
    base57_decode_uint64("OOOOOOOOOOO");
    base57_decode_uint64("IIIIIIIIIII");
    base57_decode_uint64("lllllllllll");
    base57_decode_uint64("11111111111");
    base57_decode_uint64("01OlI01OlI0");
    base57_decode_uint64("\xFF\xFE\xFD\xFC\xFB\xFA\xF9\xF8\xF7\xF6\xF5");
    base57_decode_uint64("\x7D\x7E\x7F\x80\x81\x82\x83\x84\x84\x85\x86");
    base57_decode_uint64("\t\n\v\f\r\x1C\x1D\x1E\x1F ,");
    base57_decode_uint64("XWXWXWVVUVU");
}


static void test_invalid_string_decoding(const char* str, size_t error_index) {
    uint8_t buffer[1024];
    uint8_t* decoded = buffer + 1;
    uint8_t* output = decoded;
    char* input = str;
    size_t input_length = strlen(str);
    size_t output_max_length = base57_calculate_decoded_max_length(input_length);
    static const uint8_t sentry = 0x5A;
    buffer[0] = sentry;
    decoded[output_max_length] = sentry;
    base57_decode(&output, &input, &input_length);
    TEST_UINT_EQUALITY(sentry, buffer[0]);
    TEST_UINT_EQUALITY(sentry, decoded[output_max_length]);
    TEST(str <= input);
    TEST(decoded <= output);
    TEST_UINT_EQUALITY(error_index, input - str);
    TEST_UINT_RELATION(output - decoded, <=, base57_calculate_decoded_max_length(input - str));
}


static void test_invalid_strings_decoding() {
    test_invalid_string_decoding("0", 0);
    test_invalid_string_decoding("xO", 1);
    test_invalid_string_decoding("xx1", 2);
    test_invalid_string_decoding("xxxl", 3);
    test_invalid_string_decoding("xxxxI", 4);
    test_invalid_string_decoding("xxxxx\xFF", 5);
    test_invalid_string_decoding("xxxxxx\x7F", 6);
    test_invalid_string_decoding("xxxxxxx\x80", 7);
    test_invalid_string_decoding("xxxxxxxx\x11", 8);
    test_invalid_string_decoding("xxxxxxxxx~", 9);
    test_invalid_string_decoding("xxxxxxxxxx)", 10);
    test_invalid_string_decoding("xxxxxxxxxxx{", 11);
    test_invalid_string_decoding("xOx", 1);
    test_invalid_string_decoding("xx1xx", 2);
    test_invalid_string_decoding("xxxlxxx", 3);
    test_invalid_string_decoding("xxxxIxxxx", 4);
    test_invalid_string_decoding("xxxxx\xFFxxxxx", 5);
    test_invalid_string_decoding("xxxxxx\x7Fxxxxxx", 6);
    test_invalid_string_decoding("xxxxxxx\x80xxxxxxx", 7);
    test_invalid_string_decoding("xxxxxxxx\x11xxxxxxxx", 8);
    test_invalid_string_decoding("xxxxxxxxx~xxxxxxxxx", 9);
    test_invalid_string_decoding("xxxxxxxxxx)xxxxxxxxxx", 10);
    test_invalid_string_decoding("xxxxxxxxxxx{xxxxxxxxxxx", 11);
    test_invalid_string_decoding("XXXXXXXXXXX0", 11);
    test_invalid_string_decoding("ZZZZZZZZZZX0", 11);
}


static void test_bytes_encoding(const uint8_t* const plain, const size_t plain_size) {
    char* encoded = (char*)malloc(2 * plain_size + 8);
    uint8_t* decoded = (uint8_t*)malloc(plain_size + 8);
    base57_encode(encoded, plain, plain_size);
    const size_t encoded_size = base57_calculate_encoded_length(plain_size);
    TEST_UINT_RELATION(2 * plain_size, >= , encoded_size);
    TEST_UINT_EQUALITY(0, encoded[encoded_size]);
    char* input = encoded;
    size_t input_length = encoded_size;
    uint8_t* output = decoded;
    base57_decode(&output, &input, &input_length);
    TEST_UINT_EQUALITY(encoded_size, input - encoded);
    TEST_UINT_EQUALITY(plain_size, output - decoded);
    TEST_UINT_EQUALITY(0, memcmp(plain, decoded, plain_size));
    free(decoded);
    free(encoded);
}


static void test_same_bytes_encoding(const uint8_t byte) {
    uint8_t plain[130];
    memset(plain, byte, sizeof(plain));
    for (size_t i = 0; i <= sizeof(plain); ++i) {
        test_bytes_encoding(plain, i);
    }
}


static void fill_randomly(uint8_t* plain, size_t plain_size, uint64_t* lcg_state) {
    uint64_t i;
    for (i = 0; i + sizeof(uint64_t) <= plain_size; i += sizeof(uint64_t)) {
        *(uint64_t*)(plain + i) = lcg(lcg_state);
    }
    uint64_t value = lcg(lcg_state);
    while (i < plain_size) {
        plain[i++] = value & 0xFFull;
        value >>= 8;
    }
}


static void test_random_bytes_encoding(size_t max_size, size_t tests, uint64_t* lcg_state) {
    uint8_t* plain = (uint8_t*)malloc(max_size);
    for (int test = 1; test <= tests; ++test) {
        size_t plain_size = lcg(lcg_state) % max_size;
        fill_randomly(plain, plain_size, lcg_state);
        test_bytes_encoding(plain, plain_size);
    }
    free(plain);
}


#define PRINT_AND_CALL(STATEMENT) do { \
    fputs("\n" #STATEMENT "\n", output_stream); \
    do { STATEMENT; } while (false); \
    fprintf(output_stream, "number of passed tests: %u\n", passed_tests); \
    fprintf(output_stream, "number of failed tests: %u\n", failed_tests); \
} while (false)


int main() {
    output_stream = stderr;
    PRINT_AND_CALL(test_uint64_encoding_invariance());
    PRINT_AND_CALL(test_uint64_encoding());
    PRINT_AND_CALL(test_invalid_uint64_decoding());
    PRINT_AND_CALL(test_same_bytes_encoding(0x00));
    PRINT_AND_CALL(test_same_bytes_encoding(0xFF));
    PRINT_AND_CALL(test_same_bytes_encoding(0xA5));
    PRINT_AND_CALL(test_invalid_strings_decoding());
    uint64_t lcg_state = 0xC089D80887303354ull;
    PRINT_AND_CALL(test_random_bytes_encoding(8 * 1024, 1024, &lcg_state));
    PRINT_AND_CALL(test_random_bytes_encoding(1024 * 1024, 256, &lcg_state));
    char output[16];
    printf("%s\n", base57_encode_uint64(output, 0xbf13433c9e01b63bull));
    printf("%s\n", base57_encode_uint64(output, 0xd77eddbddabc4762ull));
    return failed_tests > 0;
}
