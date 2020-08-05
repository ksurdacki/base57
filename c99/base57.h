#pragma once

/** \author Krzysztof Piotr Surdacki https://surdacki.pl/
  * \date 2020
  * \brief Binary-to-text encoding which uses only selected 57 alphanumeric US-ASCII characters.
  * It does not use any of symbol characters like +, -, / nor =.
  * Five visually similar looking characters are removed additionally: 0, 0, 1, l and I.
  * Base57 has efficiency of 8/11 (~73%) which is comparable to base64 3/4 (75%).
  * Comparison of UUID encoding:
    format | length | example
 ----------|--------|----------------------------------------
       dec |     39 | 277443415005306452147246869113559209911
 canonical |     35 | d0b9a878-239d-42d3-bec9-934386a257b
       hex |     32 | d0b9a878239d42d3bec9934386a257b7
    base57 |     22 | yYbAg7domA657cEd5E5Wmi
    base64 | 22(24) | 0LmoeCOdQtO+yZNDhqJXtw==
   Ascii85 |     20 | d(-*",Fq0M^<)R+L8%bY
    binary |     16 |
  */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Calculates encoded output length for a given plain input length.
  * \warning No overflow check. */
size_t base57_calculate_encoded_length(size_t plain_length);


/** Encodes \c input into \c output with a NUL termination.
  * \pre \c Output must have at least \c base57_calc_encoded_length() + 1 size
  * \returns \c output
  * \pre Stream encoding must be done in packets lengths dividable by 8. */
char* base57_encode(char* output, const uint8_t* input, size_t input_length);


size_t base57_calculate_decoded_max_length(size_t encoded_length);


typedef struct base57_DecodingResult {
    size_t output_length;
    size_t consumed_input_length;
    enum {
        base57_NO_INPUT = 0, ///< input has been processed without any objections,
        base57_CONTROL_SYMBOL, ///< decoding stopped on US-ASCII control character like NUL
        base57_INVALID_SYMBOL, ///< decoding stopped on other, non base57 symbol
        base57_OVERFLOW, ///< alnum characters sequence was invalid for base57 encoding
        base57_TRUNCATED, ///< whole input has been processed but the input data is incomplete
    } termination_reason;
} base57_DecodingResult;

/** Decodes \c input into \c output.
  * \pre \c Output must have at least \c base57_calculate_decoded_max_length() size.
  * \pre Stream decoding must be done in packets containing dividable by 11 number of alnum characters. */
base57_DecodingResult base57_decode(uint8_t* output, const char* input, size_t input_length);


#ifdef __cplusplus
} // extern "C"
#endif
