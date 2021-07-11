#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef _WIN32
#include <fcntl.h>
#endif

#include "base57.h"


#define BUFFER_SIZE (1 << 15)
static char encoded_buffer[BUFFER_SIZE];
static uint8_t decoded_buffer[BUFFER_SIZE];


static void set_binary_output() {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#else
    freopen(NULL, "wb", stdout);
#endif
}


static inline size_t read_encoded() {
    size_t bytes_read = fread(encoded_buffer, 1, BUFFER_SIZE, stdin);
    if (bytes_read < BUFFER_SIZE && ferror(stdin)) {
        perror("Standard input reading error");
        exit(1);
    }
    return bytes_read;
}


static inline void write_decoded(size_t size) {
    if (fwrite(decoded_buffer, 1, size, stdout) < size) {
        perror("Standard output writing error");
        exit(1);
    }
}


int main() {
    set_binary_output();
    base57_DecodingBuffer buffer = { 0 };
    while (true) {
        size_t bytes_read = read_encoded();
        uint8_t* output = decoded_buffer;
        char* input = encoded_buffer;
        base57_decode_part(&output, &buffer, &input, &bytes_read);
        if (bytes_read > 0) {
            fputs("Invalid Base57 symbol.\n", stderr);
            exit(1);
        }
        write_decoded(output - decoded_buffer);
        if (feof(stdin)) {
            output = decoded_buffer;
            base57_flush_decoding_buffer(&output, &buffer);
            write_decoded(output - decoded_buffer);
            return 0;
        }
    }
}
