#include "base64_c.h"
#include <stdlib.h>
#include <string.h>

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode_c(const unsigned char* input, int length) {
    int output_length = 4 * ((length + 2) / 3);
    char* output = (char*)malloc(output_length + 1);
    if (output == NULL) return NULL;

    int i = 0, j = 0;
    int remaining = length;

    while (remaining > 2) {
        output[j++] = base64_chars[input[i] >> 2];
        output[j++] = base64_chars[((input[i] & 0x03) << 4) | ((input[i + 1] & 0xf0) >> 4)];
        output[j++] = base64_chars[((input[i + 1] & 0x0f) << 2) | ((input[i + 2] & 0xc0) >> 6)];
        output[j++] = base64_chars[input[i + 2] & 0x3f];
        remaining -= 3;
        i += 3;
    }

    if (remaining > 0) {
        output[j++] = base64_chars[input[i] >> 2];
        if (remaining == 1) {
            output[j++] = base64_chars[(input[i] & 0x03) << 4];
            output[j++] = '=';
        } else {
            output[j++] = base64_chars[((input[i] & 0x03) << 4) | ((input[i + 1] & 0xf0) >> 4)];
            output[j++] = base64_chars[(input[i + 1] & 0x0f) << 2];
        }
        output[j++] = '=';
    }

    output[j] = '\0';
    return output;
}

void base64_free(void* ptr) {
    free(ptr);
}