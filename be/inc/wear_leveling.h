#ifndef WEAR_LEVELING_H  
#define WEAR_LEVELING_H  
  
#include <stdint.h>  
#include <stdbool.h>  
#include "be_common.h"  
  
// 磨损均衡配置  
#define WL_BLOCK_SIZE               (512 * 1024)    // 512KB块大小  
#define WL_TOTAL_BLOCKS             8192            // 总块数 (4GB / 512KB)  
#define WL_WEAR_THRESHOLD           100             // 磨损差异阈值  
#define WL_MAX_ENDURANCE            100000          // 最大擦写次数  
#define WL_SCAN_INTERVAL_MS         30000           // 扫描间隔30秒  
#define WL_BALANCE_INTERVAL_MS      60000           // 均衡间隔60秒  
  
// 块状态定义  
typedef enum {  
    WL_BLOCK_STATUS_GOOD = 0,       // 正常块  
    WL_BLOCK_STATUS_WORN = 1,       // 磨损块  
    WL_BLOCK_STATUS_BAD = 2,        // 坏块  
    WL_BLOCK_STATUS_RESERVED = 3    // 保留块  
} wl_block_status_t;  
  
// 磨损信息结构  
typedef struct {  
    uint32_t wear_count;            // 擦写次数  
    wl_block_status_t status;       // 块状态  
    uint32_t last_access_time;      // 最后访问时间  
    bool is_hot;                    // 是否为热块  
} wl_block_info_t;  
  
// 磨损均衡统计  
typedef struct {  
    uint32_t total_blocks;          // 总块数  
    uint32_t good_blocks;           // 正常块数  
    uint32_t worn_blocks;           // 磨损块数  
    uint32_t bad_blocks;            // 坏块数  
    uint32_t min_wear_count;        // 最小磨损次数  
    uint32_t max_wear_count;        // 最大磨损次数  
    uint32_t avg_wear_count;        // 平均磨损次数  
    uint32_t balance_operations;    // 均衡操作次数  
    uint32_t data_migrations;       // 数据迁移次数  
} wl_statistics_t;  
  
// 磨损均衡接口函数  
be_status_t wl_init(void);  
be_status_t wl_get_block_wear_info(uint32_t block_index, wl_block_info_t *info);  
be_status_t wl_update_block_wear(uint32_t block_index, uint32_t new_wear_count);  
be_status_t wl_find_cold_block(uint32_t *cold_block_index);  
be_status_t wl_find_hot_block(uint32_t *hot_block_index);  
be_status_t wl_perform_balance(uint32_t hot_block, uint32_t cold_block);  
be_status_t wl_get_statistics(wl_statistics_t *stats);  
void wl_scan_task(ULONG initial_input);  
void wl_balance_task(ULONG initial_input);  
  
#endif // WEAR_LEVELING_H