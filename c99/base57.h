#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char* const base57_version;


#define base57_ENCODED_UINT64_SIZE 11


/// Encodes 64-bit unsigned integer.
/// \returns NUL terminated output
char* base57_encode_uint64(char output[base57_ENCODED_UINT64_SIZE + 1], uint64_t input);

/// Decodes 64-bit unsigned integer
/// \warning Undefined value is returned for invalid input.
uint64_t base57_decode_uint64(char input[base57_ENCODED_UINT64_SIZE]);


/// Calculates encoded output length for a given plain input length.
/// \warning No overflow check.
/// \post base57_calculate_encoded_length(plain_length) <= 1.5*plain_length FOR plain_length >= 4
size_t base57_calculate_encoded_length(size_t plain_length);


/// Encodes \c input into \c output with a NUL termination.
/// Encoded data have lines with a maximum length of 88 characters.
/// \pre \c Output must have at least 1 + base57_calculate_encoded_length().
/// \remark Stream encoding may be done in blocks which are multiple of 64 bytes in size.
/// \returns \c output
char* base57_encode(char* output, const uint8_t* input, size_t input_length);


/// Calculates a maximum length of decoded data for a given encoded data length.
/// \post base57_calculate_decoded_max_length(encoded_length) <= encoded_length
size_t base57_calculate_decoded_max_length(size_t encoded_length);


typedef struct base57_DecodingBuffer {
    char symbols[base57_ENCODED_UINT64_SIZE];
    uint8_t symbols_number;
} base57_DecodingBuffer;


/// Advance function for stream decoding.
/// \see base57_decode() for simple use.
/// \param[out] output Must have at least base57_calculate_decoded_max_length(input_length).
/// Will be updated to point a byte just after the written data.
/// \param[out] buffer Must be initialized with zeros before the first call.
/// \warning Stream decoding must end with base57_flush_decoding_buffer() call.
/// \param input[in] Will be updated to point to a first unprocessed character.
/// \param input_length[in] Will be updated to indicate remaining number of characters.
void base57_decode_part(
    uint8_t** output, base57_DecodingBuffer* buffer, const char** input, size_t* input_length
);

/// \see base57_decode_part()
/// \param[out] output Must have at least 8 bytes.
void base57_flush_decoding_buffer(uint8_t** output, base57_DecodingBuffer* buffer);


/// Decodes \c input into \c output.
/// \pre \c Output must have at least base57_calculate_decoded_max_length(input_length).
/// \param input Will point just after a last processed character.
/// \param input_length Will be set to 0 on success.
static inline
void base57_decode(uint8_t** output, const char** input, size_t* input_length) {
    base57_DecodingBuffer buffer = { 0 };
    base57_decode_part(output, &buffer, input, input_length);
    if (*input_length == 0) { // no errors
        base57_flush_decoding_buffer(output, &buffer);
    }
}


#ifdef __cplusplus
} // extern "C"
#endif
