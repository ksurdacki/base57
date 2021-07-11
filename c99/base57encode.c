#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
    #include <fcntl.h>
#endif

#include "base57.h"


#define BUFFER_SIZE (1 << 15)
static uint8_t plain_buffer[BUFFER_SIZE];
static char encoded_buffer[2 * BUFFER_SIZE];


static inline void set_binary_input() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#else
    freopen(NULL, "rb", stdin);
#endif
}


static inline size_t read_plain() {
    size_t bytes_read = fread(plain_buffer, 1, BUFFER_SIZE, stdin);
    if (bytes_read < BUFFER_SIZE && ferror(stdin)) {
        perror("Standard input reading error");
        exit(1);
    }
    assert(feof(stdin) || bytes_read % 64 == 0);
    return bytes_read;
}


static inline void write_encoded() {
    if (fputs(encoded_buffer, stdout) < 0) {
        perror("Standard output writing error");
        exit(1);
    }
}


int main() {
    set_binary_input();
    while (true) {
        size_t plain_length = read_plain();
        base57_encode(encoded_buffer, plain_buffer, plain_length);
        write_encoded();
        if (feof(stdin)) {
            return 0;
        }
        fputc('\n', stdout);
    }
}
