#include "aes_crypto.h"  
  
// AES S-box  
static const uint8_t sbox[256] = {  
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,  
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,  
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,  
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,  
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,  
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,  
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,  
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,  
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,  
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,  
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,  
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,  
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,  
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,  
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,  
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16  
};  
  
// AES逆S-box  
static const uint8_t inv_sbox[256] = {  
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,  
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,  
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,  
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,  
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,  
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,  
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,  
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,  
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,  
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,  
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,  
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,  
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,  
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,  
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,  
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d  
};  
  
// Rcon常量  
static const uint8_t rcon[11] = {  
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36  
};  
  
// 字节替换  
static void sub_bytes(uint8_t state[16]) {  
    for (int i = 0; i < 16; i++) {  
        state[i] = sbox[state[i]];  
    }  
}  
  
// 逆字节替换  
static void inv_sub_bytes(uint8_t state[16]) {  
    for (int i = 0; i < 16; i++) {  
        state[i] = inv_sbox[state[i]];  
    }  
}  
  
// 行移位  
static void shift_rows(uint8_t state[16]) {  
    uint8_t temp;  
      
    // 第二行左移1位  
    temp = state[1];  
    state[1] = state[5];  
    state[5] = state[9];  
    state[9] = state[13];  
    state[13] = temp;  
      
    // 第三行左移2位  
    temp = state[2];  
    state[2] = state[10];  
    state[10] = temp;  
    temp = state[6];  
    state[6] = state[14];  
    state[14] = temp;  
      
    // 第四行左移3位  
    temp = state[3];  
    state[3] = state[15];  
    state[15] = state[11];  
    state[11] = state[7];  
    state[7] = temp;  
}  
  
// 逆行移位  
static void inv_shift_rows(uint8_t state[16]) {  
    uint8_t temp;  
      
    // 第二行右移1位  
    temp = state[13];  
    state[13] = state[9];  
    state[9] = state[5];  
    state[5] = state[1];  
    state[1] = temp;  
      
    // 第三行右移2位  
    temp = state[2];  
    state[2] = state[10];  
    state[10] = temp;  
    temp = state[6];  
    state[6] = state[14];  
    state[14] = temp;  
      
    // 第四行右移3位  
    temp = state[3];  
    state[3] = state[7];  
    state[7] = state[11];  
    state[11] = state[15];  
    state[15] = temp;  
}  
  
// GF(2^8)乘法
static uint8_t gf_mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    uint8_t hi_bit_set;

    for (int i = 0; i < 8; i++) {
        if (b & 1) {
            result ^= a;
        }
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) {
            a ^= 0x1b; // AES不可约多项式
        }
        b >>= 1;
    }
    return result;
}

// 列混合
static void mix_columns(uint8_t state[16]) {
    uint8_t temp[4];

    for (int col = 0; col < 4; col++) {
        temp[0] = gf_mul(0x02, state[col]) ^ gf_mul(0x03, state[col + 4]) ^
                  state[col + 8] ^ state[col + 12];
        temp[1] = state[col] ^ gf_mul(0x02, state[col + 4]) ^
                  gf_mul(0x03, state[col + 8]) ^ state[col + 12];
        temp[2] = state[col] ^ state[col + 4] ^
                  gf_mul(0x02, state[col + 8]) ^ gf_mul(0x03, state[col + 12]);
        temp[3] = gf_mul(0x03, state[col]) ^ state[col + 4] ^
                  state[col + 8] ^ gf_mul(0x02, state[col + 12]);

        state[col] = temp[0];
        state[col + 4] = temp[1];
        state[col + 8] = temp[2];
        state[col + 12] = temp[3];
    }
}

// 逆列混合
static void inv_mix_columns(uint8_t state[16]) {
    uint8_t temp[4];

    for (int col = 0; col < 4; col++) {
        temp[0] = gf_mul(0x0e, state[col]) ^ gf_mul(0x0b, state[col + 4]) ^
                  gf_mul(0x0d, state[col + 8]) ^ gf_mul(0x09, state[col + 12]);
        temp[1] = gf_mul(0x09, state[col]) ^ gf_mul(0x0e, state[col + 4]) ^
                  gf_mul(0x0b, state[col + 8]) ^ gf_mul(0x0d, state[col + 12]);
        temp[2] = gf_mul(0x0d, state[col]) ^ gf_mul(0x09, state[col + 4]) ^
                  gf_mul(0x0e, state[col + 8]) ^ gf_mul(0x0b, state[col + 12]);
        temp[3] = gf_mul(0x0b, state[col]) ^ gf_mul(0x0d, state[col + 4]) ^
                  gf_mul(0x09, state[col + 8]) ^ gf_mul(0x0e, state[col + 12]);

        state[col] = temp[0];
        state[col + 4] = temp[1];
        state[col + 8] = temp[2];
        state[col + 12] = temp[3];
    }
}

// 轮密钥加
static void add_round_key(uint8_t state[16], const uint8_t round_key[16]) {
    for (int i = 0; i < 16; i++) {
        state[i] ^= round_key[i];
    }
}

// 密钥扩展
static void key_expansion(const uint8_t key[16], uint8_t round_keys[11][16]) {
    uint8_t temp[4];
    int i = 0;

    // 复制原始密钥
    memcpy(round_keys[0], key, 16);

    for (i = 1; i <= 10; i++) {
        // 复制前一轮密钥的最后4字节
        memcpy(temp, &round_keys[i-1][12], 4);

        // RotWord
        uint8_t t = temp[0];
        temp[0] = temp[1];
        temp[1] = temp[2];
        temp[2] = temp[3];
        temp[3] = t;

        // SubWord
        for (int j = 0; j < 4; j++) {
            temp[j] = sbox[temp[j]];
        }

        // 异或Rcon
        temp[0] ^= rcon[i];

        // 生成新轮密钥
        for (int j = 0; j < 4; j++) {
            round_keys[i][j] = round_keys[i-1][j] ^ temp[j];
        }
        for (int j = 4; j < 16; j++) {
            round_keys[i][j] = round_keys[i-1][j] ^ round_keys[i][j-4];
        }
    }
}

// AES初始化
be_status_t aes_init(aes_context_t *ctx, const uint8_t *key) {
    if (!ctx || !key) {
        return BE_STATUS_INVALID_PARAM;
    }

    // 密钥扩展
    key_expansion(key, ctx->round_keys);
    ctx->initialized = true;

    return BE_STATUS_SUCCESS;
}

// AES单块加密
be_status_t aes_encrypt_block(aes_context_t *ctx, const uint8_t *input, uint8_t *output) {
    if (!ctx || !ctx->initialized || !input || !output) {
        return BE_STATUS_INVALID_PARAM;
    }

    uint8_t state[16];
    memcpy(state, input, 16);

    // 初始轮密钥加
    add_round_key(state, ctx->round_keys[0]);

    // 9轮主循环
    for (int round = 1; round <= 9; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, ctx->round_keys[round]);
    }

    // 最后一轮（无列混合）
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, ctx->round_keys[10]);

    memcpy(output, state, 16);
    return BE_STATUS_SUCCESS;
}

// AES单块解密
be_status_t aes_decrypt_block(aes_context_t *ctx, const uint8_t *input, uint8_t *output) {
    if (!ctx || !ctx->initialized || !input || !output) {
        return BE_STATUS_INVALID_PARAM;
    }

    uint8_t state[16];
    memcpy(state, input, 16);

    // 初始轮密钥加
    add_round_key(state, ctx->round_keys[10]);

    // 9轮主循环
    for (int round = 9; round >= 1; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, ctx->round_keys[round]);
        inv_mix_columns(state);
    }

    // 最后一轮（无逆列混合）
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, ctx->round_keys[0]);

    memcpy(output, state, 16);
    return BE_STATUS_SUCCESS;
}

// AES-ECB模式加密
be_status_t aes_ecb_encrypt(aes_context_t *ctx, const uint8_t *input, uint8_t *output, uint32_t length) {
    if (!ctx || !ctx->initialized || !input || !output) {
        return BE_STATUS_INVALID_PARAM;
    }

    if (length % AES_BLOCK_SIZE != 0) {
        return BE_STATUS_INVALID_PARAM; // ECB模式要求数据长度为16字节的倍数
    }

    uint32_t blocks = length / AES_BLOCK_SIZE;
    be_status_t status;

    for (uint32_t i = 0; i < blocks; i++) {
        status = aes_encrypt_block(ctx,
                                 input + i * AES_BLOCK_SIZE,
                                 output + i * AES_BLOCK_SIZE);
        if (status != BE_STATUS_SUCCESS) {
            return status;
        }
    }

    return BE_STATUS_SUCCESS;
}

// AES-ECB模式解密
be_status_t aes_ecb_decrypt(aes_context_t *ctx, const uint8_t *input, uint8_t *output, uint32_t length) {
    if (!ctx || !ctx->initialized || !input || !output) {
        return BE_STATUS_INVALID_PARAM;
    }

    if (length % AES_BLOCK_SIZE != 0) {
        return BE_STATUS_INVALID_PARAM; // ECB模式要求数据长度为16字节的倍数
    }

    uint32_t blocks = length / AES_BLOCK_SIZE;
    be_status_t status;

    for (uint32_t i = 0; i < blocks; i++) {
        status = aes_decrypt_block(ctx,
                                 input + i * AES_BLOCK_SIZE,
                                 output + i * AES_BLOCK_SIZE);
        if (status != BE_STATUS_SUCCESS) {
            return status;
        }
    }

    return BE_STATUS_SUCCESS;
}

// AES清理
void aes_deinit(aes_context_t *ctx) {
    if (ctx) {
        memset(ctx, 0, sizeof(aes_context_t));
    }
}
