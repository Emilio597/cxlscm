#ifndef AES_CRYPTO_H  
#define AES_CRYPTO_H  
  
#include "be_common.h"  
  
// AES-128常量定义  
#define AES_BLOCK_SIZE 16  
#define AES_KEY_SIZE 16  
#define AES_ROUNDS 10  
  
// AES状态结构  
typedef struct {  
    uint8_t round_keys[11][16];  // 轮密钥  
    bool initialized;  
} aes_context_t;  
  
// 函数声明  
be_status_t aes_init(aes_context_t *ctx, const uint8_t *key);  
be_status_t aes_encrypt_block(aes_context_t *ctx, const uint8_t *input, uint8_t *output);  
be_status_t aes_decrypt_block(aes_context_t *ctx, const uint8_t *input, uint8_t *output);  
be_status_t aes_ecb_encrypt(aes_context_t *ctx, const uint8_t *input, uint8_t *output, uint32_t length);  
be_status_t aes_ecb_decrypt(aes_context_t *ctx, const uint8_t *input, uint8_t *output, uint32_t length);  
void aes_deinit(aes_context_t *ctx);  
  
#endif // AES_CRYPTO_H
