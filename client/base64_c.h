#ifndef BASE64_C_H
#define BASE64_C_H

#ifdef __cplusplus
extern "C" {
#endif

// base64编码函数
char* base64_encode_c(const unsigned char* input, int length);

// base64解码函数
unsigned char* base64_decode_c(const char* input, int* outlen);

// 释放base64编码/解码分配的内存
void base64_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // BASE64_C_H