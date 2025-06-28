#ifndef MEDIA_HEALTH_H  
#define MEDIA_HEALTH_H  
  
#include <stdint.h>  
#include <stdbool.h>  
#include "be_common.h"  
// 介质健康等级  
typedef enum {  
    MEDIA_HEALTH_EXCELLENT = 0,  // 优秀  
    MEDIA_HEALTH_GOOD = 1,       // 良好  
    MEDIA_HEALTH_FAIR = 2,       // 一般  
    MEDIA_HEALTH_POOR = 3,       // 差  
    MEDIA_HEALTH_CRITICAL = 4    // 危险  
} media_health_level_t;  
  
// 介质统计信息  
typedef struct {  
    uint64_t total_read_count;       // 总读取次数  
    uint64_t total_write_count;      // 总写入次数  
    uint64_t read_retry_count;       // 读重试次数  
    uint64_t write_retry_count;      // 写重试次数  
    uint64_t ecc_correctable_errors; // ECC可纠正错误  
    uint64_t ecc_uncorrectable_errors; // ECC不可纠正错误  
    uint32_t bad_block_count;        // 坏块数量  
    uint32_t spare_block_count;      // 剩余备用块  
    uint32_t wear_level_max;         // 最大磨损次数  
    uint32_t wear_level_avg;         // 平均磨损次数  
    media_health_level_t health_level; // 健康等级  
    uint32_t remaining_life_percent; // 剩余寿命百分比  
} media_health_stats_t;  
  
// 介质健康管理接口  
status_t media_health_init(void);  
status_t media_health_update_read_stats(bool success, uint32_t retry_count);  
status_t media_health_update_write_stats(bool success, uint32_t retry_count);  
status_t media_health_update_ecc_stats(uint32_t correctable, uint32_t uncorrectable);  
status_t media_health_update_wear_stats(uint32_t block_index, uint32_t wear_count);  
status_t media_health_get_stats(media_health_stats_t *stats);  
media_health_level_t media_health_calculate_level(void);  
  
#endif // MEDIA_HEALTH_H