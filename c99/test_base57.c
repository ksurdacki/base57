#include "base57.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>


/** Donald Knuth's Linear Congruential Generator */
static inline
uint64_t lcg(uint64_t *state) {
    *state = 6364136223846793005ull * *state + 1442695040888963407ull;
    return *state;
}


#define LENGTH_OF(A) (sizeof(A) / sizeof(A[0]))


static unsigned passed_tests = 0;
static unsigned failed_tests = 0;

static void increment_failed_tests() {
    ++failed_tests; // a good place for putting a breakpoint
}


static FILE* output;


#define TEST(EXP) do { \
    if (EXP) { \
        ++passed_tests; \
    } \
    else { \
        fprintf(output, "%s@%u failed! (", __func__, __LINE__); \
        fputs(#EXP ") is false!\n", output); \
        increment_failed_tests(); \
    } \
} while (false)


#define TEST_UINT_EQUALITY(EXPECTED, ACTUAL) do { \
    uint64_t expected = (EXPECTED); \
    uint64_t actual = (ACTUAL); \
    if (expected == actual) { \
        ++passed_tests; \
    } \
    else { \
        char relation = expected < actual ? '<' : '>'; \
        fprintf(output, "%s@%u failed! (", __func__, __LINE__); \
        fprintf(output, #EXPECTED " %c " #ACTUAL "): ", relation); \
        fprintf(output, "%" PRIu64 " %c %" PRIu64 "; ", expected, relation, actual); \
        uint64_t difference = expected > actual ? expected - actual : actual - expected; \
        fprintf(output, "the difference is %" PRIu64 "\n", difference); \
        increment_failed_tests(); \
    } \
} while (false)


static void test_uint64_encoding() {
    char encoded[11 + 1];
    uint64_t uint64 = 0;
    base57_encode(encoded, &uint64, sizeof(uint64));
    TEST(strcmp(encoded, "AAAAAAAAAAA") == 0);
    uint64 = ~0ull;
    base57_encode(encoded, &uint64, sizeof(uint64));
    TEST(strcmp(encoded, "7MWNUrdyU73") == 0);
}


#define TEST_DECODING(ENCODED, OUTPUT_LENGTH, CONSUMED_INPUT_LENGTH, TERMINATION_REASON) \
do { \
    uint8_t decoded[32]; \
    base57_DecodingResult result = base57_decode(decoded, (ENCODED), strlen(ENCODED)); \
    TEST_UINT_EQUALITY((OUTPUT_LENGTH), result.output_length); \
    TEST_UINT_EQUALITY((CONSUMED_INPUT_LENGTH), result.consumed_input_length); \
    TEST(result.termination_reason == (TERMINATION_REASON)); \
} while (false)


static void test_decoding() {
    TEST_DECODING("", 0, 0, base57_NO_INPUT);
    TEST_DECODING("A", 0, 1, base57_TRUNCATED);
    TEST_DECODING("AA", 1, 2, base57_NO_INPUT);
    TEST_DECODING("AAA", 2, 3, base57_NO_INPUT);
    TEST_DECODING("AAAA", 0, 4, base57_TRUNCATED);
    TEST_DECODING("AAAAA", 3, 5, base57_NO_INPUT);
    TEST_DECODING("AAAAAA", 4, 6, base57_NO_INPUT);
    TEST_DECODING("AAAAAAA", 5, 7, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAA", 0, 8, base57_TRUNCATED);
    TEST_DECODING("AAAAAAAAA", 6, 9, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAA", 7, 10, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAA", 8, 11, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAA", 8 + 0, 11 + 1, base57_TRUNCATED);
    TEST_DECODING("AAAAAAAAAAAAA", 8 + 1, 11 + 2, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAA", 8 + 2, 11 + 3, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAA", 8 + 0, 11 + 4, base57_TRUNCATED);
    TEST_DECODING("AAAAAAAAAAAAAAAA", 8 + 3, 11 + 5, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAAAA", 8 + 4, 11 + 6, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAAAAA", 8 + 5, 11 + 7, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAAAAAA", 8 + 0, 11 + 8, base57_TRUNCATED);
    TEST_DECODING("AAAAAAAAAAAAAAAAAAAA", 8 + 6, 11 + 9, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAAAAAAAA", 8 + 7, 11 + 10, base57_NO_INPUT);
    TEST_DECODING("AAAAAAAAAAAAAAAAAAAAAA", 8 + 8, 11 + 11, base57_NO_INPUT);
    TEST_DECODING("7MWNUsdyU73", 0, 11, base57_OVERFLOW);
    TEST_DECODING("7MWNUrdyU74", 0, 11, base57_OVERFLOW);
    TEST_DECODING("7MWNUrdyU79", 0, 11, base57_OVERFLOW);
    TEST_DECODING("7MWNUrdyO73", 0, 8, base57_INVALID_SYMBOL);
    TEST_DECODING("7MWNlrdyU73", 0, 4, base57_INVALID_SYMBOL);
    TEST_DECODING("1MWNUrdyU73", 0, 0, base57_INVALID_SYMBOL);
    TEST_DECODING("7MWNUr0yU73", 0, 6, base57_INVALID_SYMBOL);
    TEST_DECODING("7MWNUrdyU7I", 0, 10, base57_INVALID_SYMBOL);
    TEST_DECODING("7MWNU+dyU73", 0, 5, base57_INVALID_SYMBOL);
    TEST_DECODING("9", 0, 1, base57_TRUNCATED);
    TEST_DECODING("99", 0, 2, base57_OVERFLOW);
    TEST_DECODING("999", 0, 3, base57_OVERFLOW);
    TEST_DECODING("9999", 0, 4, base57_TRUNCATED);
    TEST_DECODING("99999", 0, 5, base57_OVERFLOW);
    TEST_DECODING("999999", 0, 6, base57_OVERFLOW);
    TEST_DECODING("9999999", 0, 7, base57_OVERFLOW);
    TEST_DECODING("99999999", 0, 8, base57_TRUNCATED);
    TEST_DECODING("999999999", 0, 9, base57_OVERFLOW);
    TEST_DECODING("9999999999", 0, 10, base57_OVERFLOW);
    TEST_DECODING("99999999999", 0, 11, base57_OVERFLOW);
    TEST_DECODING("dE", 1, 2, base57_NO_INPUT);
    TEST_DECODING("eE", 0, 2, base57_OVERFLOW);
    TEST_DECODING("tKW", 2, 3, base57_NO_INPUT);
    TEST_DECODING("uKW", 0, 3, base57_OVERFLOW);
    TEST_DECODING("GxjjB", 3, 5, base57_NO_INPUT);
    TEST_DECODING("HxjjB", 0, 5, base57_OVERFLOW);
    TEST_DECODING("aJz2HH", 4, 6, base57_NO_INPUT);
    TEST_DECODING("bJz2HH", 0, 6, base57_OVERFLOW);
    TEST_DECODING("R4P2WDi", 5, 7, base57_NO_INPUT);
    TEST_DECODING("S4P2WDi", 0, 7, base57_OVERFLOW);
    TEST_DECODING("zTaKrG9fC", 6, 9, base57_NO_INPUT);
    TEST_DECODING("2TaKrG9fC", 0, 9, base57_OVERFLOW);
    TEST_DECODING("DNMVYGCpVM", 7, 10, base57_NO_INPUT);
    TEST_DECODING("ENMVYGCpVM", 0, 10, base57_OVERFLOW);
    TEST_DECODING("7MWNUrdyU73", 8, 11, base57_NO_INPUT);
    TEST_DECODING("8MWNUrdyU73", 0, 11, base57_OVERFLOW);
}


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


static void test_integers() {
    size_t i = 0;
    uint64_t plain;
    char encoded[2*11 + 1];
    uint64_t test_pair[2];
    uint64_t decoded_pair[2];
    base57_DecodingResult result;
    uint64_t lcg_state = 0x723D77B137A17BB1ull;
    while (TEST_INTEGERS[i] != 0) {
        base57_encode(encoded, &TEST_INTEGERS[i], sizeof(TEST_INTEGERS[i]));
        result = base57_decode(&plain, encoded, LENGTH_OF(encoded));
        TEST_UINT_EQUALITY(TEST_INTEGERS[i], plain);
        TEST_UINT_EQUALITY(base57_CONTROL_SYMBOL, result.termination_reason);
        TEST_UINT_EQUALITY(11, result.consumed_input_length);
        TEST_UINT_EQUALITY(sizeof(plain), result.output_length);

        test_pair[0] = TEST_INTEGERS[lcg(&lcg_state) % LENGTH_OF(TEST_INTEGERS)];
        test_pair[1] = TEST_INTEGERS[lcg(&lcg_state) % LENGTH_OF(TEST_INTEGERS)];
        base57_encode(encoded, test_pair, sizeof(test_pair));
        result = base57_decode(decoded_pair, encoded, LENGTH_OF(encoded));
        TEST_UINT_EQUALITY(test_pair[0], decoded_pair[0]);
        TEST_UINT_EQUALITY(test_pair[1], decoded_pair[1]);
        TEST_UINT_EQUALITY(base57_CONTROL_SYMBOL, result.termination_reason);
        TEST_UINT_EQUALITY(22, result.consumed_input_length);
        TEST_UINT_EQUALITY(sizeof(decoded_pair), result.output_length);

        ++i;
    }
}


int main() {
    output = stderr;
    test_uint64_encoding();
    test_decoding();
    test_integers();
    fprintf(output, "number of passed tests: %u\n", passed_tests);
    fprintf(output, "number of failed tests: %u\n", failed_tests);
    return failed_tests > 0;
}
