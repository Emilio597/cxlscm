#include "bch.h"  
#include "be_common.h"  
  
static bch_codec_t g_bch_codec = {0};  
  
// GF(2^11)有限域运算  
static uint16_t gf_multiply(uint16_t a, uint16_t b) {  
    if (a == 0 || b == 0) return 0;  
    return g_bch_codec.gf_antilog[(g_bch_codec.gf_log[a] + g_bch_codec.gf_log[b]) % (BCH_GF_SIZE - 1)];  
}  
  
static uint16_t gf_divide(uint16_t a, uint16_t b) {  
    if (a == 0) return 0;  
    if (b == 0) return 0; // 除零错误  
    return g_bch_codec.gf_antilog[(g_bch_codec.gf_log[a] - g_bch_codec.gf_log[b] + BCH_GF_SIZE - 1) % (BCH_GF_SIZE - 1)];  
}  
  
// 初始化GF(2^11)对数表和反对数表  
static void bch_init_gf_tables(void) {  
    uint16_t primitive_poly = 0x805; // x^11 + x^2 + 1 (本原多项式)  
    uint16_t alpha = 1;  
      
    g_bch_codec.gf_log[0] = 0;  
    g_bch_codec.gf_antilog[0] = 1;  
      
    for (int i = 1; i < BCH_GF_SIZE; i++) {  
        g_bch_codec.gf_antilog[i] = alpha;  
        g_bch_codec.gf_log[alpha] = i;  
          
        alpha <<= 1;  
        if (alpha & BCH_GF_SIZE) {  
            alpha ^= primitive_poly;  
        }  
    }  
}  
  
// 初始化BCH生成多项式  
static void bch_init_generator_poly(void) {  
    // 简化实现：使用预计算的生成多项式系数  
    // 实际应用中需要根据本原多项式和最小多项式计算  
    memset(g_bch_codec.generator_poly, 0, sizeof(g_bch_codec.generator_poly));  
    g_bch_codec.generator_poly[0] = 1; // 最低次项  
    g_bch_codec.generator_poly[BCH_PARITY_LENGTH] = 1; // 最高次项  
      
    // 这里应该包含完整的生成多项式计算逻辑  
    // 为简化演示，使用固定模式  
    for (int i = 1; i < BCH_PARITY_LENGTH; i++) {  
        g_bch_codec.generator_poly[i] = (i * 0x123) & 0x7FF; // 示例系数  
    }  
}  
  
// BCH编码器初始化  
be_status_t bch_init(void) {  
    be_log_print("BCH: Initializing BCH(1280,1024,47) codec...");  
      
    memset(&g_bch_codec, 0, sizeof(g_bch_codec));  
      
    // 初始化GF(2^11)有限域表  
    bch_init_gf_tables();  
      
    // 初始化生成多项式  
    bch_init_generator_poly();  
      
    g_bch_codec.initialized = true;  
      
    be_log_print("BCH: Codec initialized. Code rate: %d/%d = %.2f",   
                 BCH_INFO_LENGTH, BCH_CODE_LENGTH,   
                 (float)BCH_INFO_LENGTH / BCH_CODE_LENGTH);  
    be_log_print("BCH: Error correction capability: %d bits", BCH_ERROR_CAPACITY);  
      
    return BE_STATUS_SUCCESS;  
}  
  
// BCH编码  
be_status_t bch_encode(const uint8_t *data, uint8_t *codeword) {  
    if (!g_bch_codec.initialized || !data || !codeword) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    // 复制信息位到码字前部  
    memcpy(codeword, data, BCH_INFO_LENGTH / 8);  
      
    // 计算校验位  
    uint16_t remainder[BCH_PARITY_LENGTH / 16] = {0};  
      
    // 多项式长除法计算校验位  
    for (int i = 0; i < BCH_INFO_LENGTH / 8; i++) {  
        uint8_t byte = data[i];  
        for (int bit = 7; bit >= 0; bit--) {  
            uint16_t feedback = remainder[0] ^ ((byte >> bit) & 1);  
              
            // 移位寄存器操作  
            for (int j = 0; j < (BCH_PARITY_LENGTH / 16) - 1; j++) {  
                remainder[j] = remainder[j + 1] ^ gf_multiply(feedback, g_bch_codec.generator_poly[j + 1]);  
            }  
            remainder[(BCH_PARITY_LENGTH / 16) - 1] = gf_multiply(feedback, g_bch_codec.generator_poly[BCH_PARITY_LENGTH / 16]);  
        }  
    }  
      
    // 将校验位添加到码字后部  
    for (int i = 0; i < BCH_PARITY_LENGTH / 16; i++) {  
        codeword[BCH_INFO_LENGTH / 8 + i * 2] = remainder[i] & 0xFF;  
        codeword[BCH_INFO_LENGTH / 8 + i * 2 + 1] = (remainder[i] >> 8) & 0xFF;  
    }  
      
    be_log_print("BCH: Encoded %d bytes data to %d bytes codeword",   
                 BCH_INFO_LENGTH / 8, BCH_CODE_LENGTH / 8);  
      
    return BE_STATUS_SUCCESS;  
}  
  
// 计算伴随式  
static void bch_calculate_syndromes(const uint8_t *codeword, uint16_t *syndromes) {  
    memset(syndromes, 0, BCH_ERROR_CAPACITY * 2 * sizeof(uint16_t));  
      
    for (int i = 0; i < BCH_ERROR_CAPACITY * 2; i++) {  
        uint16_t alpha_power = i + 1;  
        uint16_t syndrome = 0;  
          
        for (int j = 0; j < BCH_CODE_LENGTH / 8; j++) {  
            uint8_t byte = codeword[j];  
            for (int bit = 0; bit < 8; bit++) {  
                if ((byte >> bit) & 1) {  
                    uint16_t position = j * 8 + bit;  
                    uint16_t alpha_exp = (alpha_power * position) % (BCH_GF_SIZE - 1);  
                    syndrome ^= g_bch_codec.gf_antilog[alpha_exp];  
                }  
            }  
        }  
        syndromes[i] = syndrome;  
    }  
}  
  
// Berlekamp-Massey算法求错误定位多项式  
static int bch_find_error_locator(const uint16_t *syndromes, uint16_t *error_locator) {  
    uint16_t lambda[BCH_ERROR_CAPACITY + 1] = {0};  
    uint16_t prev_lambda[BCH_ERROR_CAPACITY + 1] = {0};  
    uint16_t temp[BCH_ERROR_CAPACITY + 1];  
      
    lambda[0] = 1;  
    prev_lambda[0] = 1;  
      
    int L = 0; // 当前多项式次数  
    int m = 1; // 上次长度改变的位置  
    uint16_t b = 1;  
      
    for (int n = 0; n < BCH_ERROR_CAPACITY * 2; n++) {  
        uint16_t discrepancy = syndromes[n];  
          
        for (int i = 1; i <= L; i++) {  
            discrepancy ^= gf_multiply(lambda[i], syndromes[n - i]);  
        }  
          
        if (discrepancy != 0) {  
            memcpy(temp, lambda, sizeof(lambda));  
              
            for (int i = 0; i <= BCH_ERROR_CAPACITY; i++) {  
                if (i >= m && (i - m) <= BCH_ERROR_CAPACITY) {  
                    lambda[i] ^= gf_multiply(discrepancy, gf_divide(prev_lambda[i - m], b));  
                }  
            }  
              
            if (2 * L <= n) {  
                L = n + 1 - L;  
                memcpy(prev_lambda, temp, sizeof(prev_lambda));  
                b = discrepancy;  
                m = 1;  
            } else {  
                m++;  
            }  
        } else {  
            m++;  
        }  
    }  
      
    memcpy(error_locator, lambda, (L + 1) * sizeof(uint16_t));  
    return L;  
}  
  
// Chien搜索找错误位置  
static int bch_find_error_positions(const uint16_t *error_locator, int degree, uint16_t *positions) {  
    int error_count = 0;  
      
    for (int i = 1; i < BCH_GF_SIZE && error_count < degree; i++) {  
        uint16_t result = 0;  
          
        for (int j = 0; j <= degree; j++) {  
            if (error_locator[j] != 0) {  
                uint16_t alpha_exp = (j * i) % (BCH_GF_SIZE - 1);  
                result ^= gf_multiply(error_locator[j], g_bch_codec.gf_antilog[alpha_exp]);  
            }  
        }  
          
        if (result == 0) {  
            // 找到错误位置  
            uint16_t error_pos = (BCH_GF_SIZE - 1 - i) % (BCH_GF_SIZE - 1);  
            if (error_pos < BCH_CODE_LENGTH) {  
                positions[error_count++] = error_pos;  
            }  
        }  
    }  
      
    return error_count;  
}  
  
// BCH解码  
bch_status_t bch_decode(uint8_t *codeword, uint8_t *data, uint32_t *error_bits) {  
    if (!g_bch_codec.initialized || !codeword || !data) {  
        return BCH_STATUS_INVALID_PARAM;  
    }  
      
    *error_bits = 0;  
      
    // 计算伴随式  
    uint16_t syndromes[BCH_ERROR_CAPACITY * 2];  
    bch_calculate_syndromes(codeword, syndromes);  
      
    // 检查是否有错误  
    bool has_error = false;  
    for (int i = 0; i < BCH_ERROR_CAPACITY * 2; i++) {  
        if (syndromes[i] != 0) {  
            has_error = true;  
            break;  
        }  
    }  
      
    if (!has_error) {  
        // 无错误，直接提取信息位  
        memcpy(data, codeword, BCH_INFO_LENGTH / 8);  
        return BCH_STATUS_SUCCESS;  
    }  
      
    // 使用Berlekamp-Massey算法求错误定位多项式  
    uint16_t error_locator[BCH_ERROR_CAPACITY + 1];  
    int degree = bch_find_error_locator(syndromes, error_locator);  
      
    if (degree > BCH_ERROR_CAPACITY) {  
        // 错误超出纠正能力  
        g_bch_codec.uncorrectable_count++;  
        be_log_print("BCH: Uncorrectable error detected, degree=%d", degree);  
        return BCH_STATUS_UNCORRECTABLE;  
    }  
      
    // Chien搜索找错误位置  
    uint16_t error_positions[BCH_ERROR_CAPACITY];  
    int error_count = bch_find_error_positions(error_locator, degree, error_positions);  
      
    if (error_count != degree) {  
        // 错误位置数量不匹配  
        g_bch_codec.uncorrectable_count++;  
        be_log_print("BCH: Error position mismatch,expected=%d, found=%d", degree, error_count);  
        return BCH_STATUS_UNCORRECTABLE;  
    }  
      
    // 纠正错误位  
    for (int i = 0; i < error_count; i++) {  
        uint16_t pos = error_positions[i];  
        uint32_t byte_index = pos / 8;  
        uint32_t bit_index = pos % 8;  
          
        if (byte_index < BCH_CODE_LENGTH / 8) {  
            codeword[byte_index] ^= (1 << bit_index);  
            (*error_bits)++;  
        }  
    }  
      
    // 提取纠正后的信息位  
    memcpy(data, codeword, BCH_INFO_LENGTH / 8);  
      
    // 更新统计信息  
    g_bch_codec.error_count += error_count;  
      
    be_log_print("BCH: Corrected %d bit errors at positions:", error_count);  
    for (int i = 0; i < error_count; i++) {  
        be_log_print("  Error position %d: bit %d", i, error_positions[i]);  
    }  
      
    return BCH_STATUS_CORRECTED;  
}  
  
// 获取BCH编解码统计信息  
be_status_t bch_get_stats(uint32_t *corrected, uint32_t *uncorrectable) {  
    if (!g_bch_codec.initialized || !corrected || !uncorrectable) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    *corrected = g_bch_codec.error_count;  
    *uncorrectable = g_bch_codec.uncorrectable_count;  
      
    return BE_STATUS_SUCCESS;  
}
