#ifndef BCH_H  
#define BCH_H  
  
#include <stdint.h>  
#include <stdbool.h>  
#include "be_common.h"  
  
// BCH(1280,1024,47)参数定义  
#define BCH_CODE_LENGTH     1280    // 码长  
#define BCH_INFO_LENGTH     1024    // 信息位长度  
#define BCH_PARITY_LENGTH   256     // 冗余位长度(1280-1024)  
#define BCH_MIN_DISTANCE    47      // 最小距离  
#define BCH_ERROR_CAPACITY  23      // 纠错能力(d-1)/2  
#define BCH_GF_M            11      // GF(2^11)  
#define BCH_GF_SIZE         2048    // 2^11  
  
// BCH编解码状态  
typedef enum {  
    BCH_STATUS_SUCCESS = 0,  
    BCH_STATUS_UNCORRECTABLE,  
    BCH_STATUS_CORRECTED,  
    BCH_STATUS_INVALID_PARAM,  
    BCH_STATUS_ERROR  
} bch_status_t;  
  
// BCH编解码器结构  
typedef struct {  
    bool initialized;  
    uint16_t generator_poly[BCH_PARITY_LENGTH + 1];  // 生成多项式  
    uint16_t gf_log[BCH_GF_SIZE];                    // GF对数表  
    uint16_t gf_antilog[BCH_GF_SIZE];                // GF反对数表  
    uint32_t error_count;                            // 累计纠错次数  
    uint32_t uncorrectable_count;                    // 不可纠正错误次数  
} bch_codec_t;  
  
// BCH编解码接口  
be_status_t bch_init(void);  
be_status_t bch_encode(const uint8_t *data, uint8_t *codeword);  
bch_status_t bch_decode(uint8_t *codeword, uint8_t *data, uint32_t *error_bits);  
be_status_t bch_get_stats(uint32_t *corrected, uint32_t *uncorrectable);  
  
#endif // BCH_H
