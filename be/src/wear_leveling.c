#include "wear_leveling.h"  
#include "pcm_controller.h"  
#include "media_health.h"  
#include "tx_api.h"  
  
// 全局磨损信息表  
static wl_block_info_t g_wear_table[WL_TOTAL_BLOCKS];  
static wl_statistics_t g_wl_stats = {0};  
static bool wl_initialized = false;  
  
  
// 从硬件获取块磨损信息  
status_t wl_get_block_wear_info(uint32_t block_index, wl_block_info_t *info) {  
    if (!wl_initialized || block_index >= WL_TOTAL_BLOCKS || !info) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 从硬件更新磨损计数  
    uint64_t block_addr = block_index * WL_BLOCK_SIZE;  
    uint32_t hw_wear_count;  
      
    status_t status = pcm_get_block_wear_count(block_addr, &hw_wear_count);  
    if (status == STATUS_SUCCESS) {  
        g_wear_table[block_index].wear_count = hw_wear_count;  
    }  
      
    // 复制信息  
    memcpy(info, &g_wear_table[block_index], sizeof(wl_block_info_t));  
      
    return STATUS_SUCCESS;  
}  
  
// 更新块磨损信息  
status_t wl_update_block_wear(uint32_t block_index, uint32_t new_wear_count) {  
    if (!wl_initialized || block_index >= WL_TOTAL_BLOCKS) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 更新本地记录  
    g_wear_table[block_index].wear_count = new_wear_count;  
    g_wear_table[block_index].last_access_time = tx_time_get();  
      
    // 更新块状态  
    if (new_wear_count > WL_MAX_ENDURANCE * 0.9) {  
        g_wear_table[block_index].status = WL_BLOCK_STATUS_WORN;  
    } else if (new_wear_count > WL_MAX_ENDURANCE) {  
        g_wear_table[block_index].status = WL_BLOCK_STATUS_BAD;  
    }  
      
    // 同步到硬件  
    uint64_t block_addr = block_index * WL_BLOCK_SIZE;  
    return pcm_set_block_wear_count(block_addr, new_wear_count);  
}  
  
// 查找冷块（磨损次数最少的块）  
status_t wl_find_cold_block(uint32_t *cold_block_index) {  
    if (!wl_initialized || !cold_block_index) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    uint32_t min_wear = 0xFFFFFFFF;  
    uint32_t cold_index = 0xFFFFFFFF;  
      
    for (uint32_t i = 0; i < WL_TOTAL_BLOCKS; i++) {  
        if (g_wear_table[i].status == WL_BLOCK_STATUS_GOOD &&   
            g_wear_table[i].wear_count < min_wear) {  
            min_wear = g_wear_table[i].wear_count;  
            cold_index = i;  
        }  
    }  
      
    if (cold_index == 0xFFFFFFFF) {  
        return STATUS_NOT_FOUND;  
    }  
      
    *cold_block_index = cold_index;  
    return STATUS_SUCCESS;  
}  
  
// 查找热块（磨损次数最多的块）  
status_t wl_find_hot_block(uint32_t *hot_block_index) {  
    if (!wl_initialized || !hot_block_index) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    uint32_t max_wear = 0;  
    uint32_t hot_index = 0xFFFFFFFF;  
      
    for (uint32_t i = 0; i < WL_TOTAL_BLOCKS; i++) {  
        if (g_wear_table[i].status == WL_BLOCK_STATUS_GOOD &&   
            g_wear_table[i].wear_count > max_wear) {  
            max_wear = g_wear_table[i].wear_count;  
            hot_index = i;  
        }  
    }  
      
    if (hot_index == 0xFFFFFFFF) {  
        return STATUS_NOT_FOUND;  
    }  
      
    *hot_block_index = hot_index;  
    return STATUS_SUCCESS;  
}  
  
// 执行磨损均衡操作  
status_t wl_perform_balance(uint32_t hot_block, uint32_t cold_block) {  
    if (!wl_initialized || hot_block >= WL_TOTAL_BLOCKS || cold_block >= WL_TOTAL_BLOCKS) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    be_log_print("WL: Balancing hot block %d (wear=%d) with cold block %d (wear=%d)",  
                 hot_block, g_wear_table[hot_block].wear_count,  
                 cold_block, g_wear_table[cold_block].wear_count);  
      
    // 数据迁移：从热块迁移到冷块  
    uint64_t hot_addr = hot_block * WL_BLOCK_SIZE;  
    uint64_t cold_addr = cold_block * WL_BLOCK_SIZE;  
      
    // 分配临时缓冲区进行数据迁移  
    uint8_t *temp_buffer = malloc(WL_BLOCK_SIZE);  
    if (!temp_buffer) {  
        return STATUS_OUT_OF_MEMORY;  
    }  
      
    status_t status;  
      
    // 1. 读取热块数据  
    status = pcm_read(hot_addr, temp_buffer, WL_BLOCK_SIZE);  
    if (status != STATUS_SUCCESS) {  
        free(temp_buffer);  
        return status;  
    }  
      
    // 2. 擦除冷块  
    status = pcm_erase_block(cold_addr);  
    if (status != STATUS_SUCCESS) {  
        free(temp_buffer);  
        return status;  
    }  
      
    // 3. 将数据写入冷块  
    status = pcm_write(cold_addr, temp_buffer, WL_BLOCK_SIZE);  
    if (status != STATUS_SUCCESS) {  
        free(temp_buffer);  
        return status;  
    }  
      
    // 4. 擦除热块  
    status = pcm_erase_block(hot_addr);  
    if (status != STATUS_SUCCESS) {  
        free(temp_buffer);  
        return status;  
    }  
      
    free(temp_buffer);  
      
    // 更新磨损信息：交换磨损计数  
    uint32_t temp_wear = g_wear_table[hot_block].wear_count;  
    g_wear_table[hot_block].wear_count = g_wear_table[cold_block].wear_count;  
    g_wear_table[cold_block].wear_count = temp_wear + 1; // 冷块增加一次擦写  
      
    // 更新统计  
    g_wl_stats.balance_operations++;  
    g_wl_stats.data_migrations++;  
      
    be_log_print("WL: Balance operation completed successfully");  
      
    return STATUS_SUCCESS;  
}  

// 更新统计信息  
static void wl_update_statistics(void) {  
    g_wl_stats.total_blocks = WL_TOTAL_BLOCKS;  
    g_wl_stats.good_blocks = 0;  
    g_wl_stats.worn_blocks = 0;  
    g_wl_stats.bad_blocks = 0;  
    g_wl_stats.min_wear_count = 0xFFFFFFFF;  
    g_wl_stats.max_wear_count = 0;  
      
    uint64_t total_wear = 0;  
    uint32_t valid_blocks = 0;  
      
    for (uint32_t i = 0; i < WL_TOTAL_BLOCKS; i++) {  
        switch (g_wear_table[i].status) {  
            case WL_BLOCK_STATUS_GOOD:  
                g_wl_stats.good_blocks++;  
                break;  
            case WL_BLOCK_STATUS_WORN:  
                g_wl_stats.worn_blocks++;  
                break;  
            case WL_BLOCK_STATUS_BAD:  
                g_wl_stats.bad_blocks++;  
                continue; // 跳过坏块的磨损统计  
            default:  
                break;  
        }  
          
        // 更新最小和最大磨损次数  
        if (g_wear_table[i].wear_count < g_wl_stats.min_wear_count) {  
            g_wl_stats.min_wear_count = g_wear_table[i].wear_count;  
        }  
          
        if (g_wear_table[i].wear_count > g_wl_stats.max_wear_count) {  
            g_wl_stats.max_wear_count = g_wear_table[i].wear_count;  
        }  
          
        // 累计磨损次数用于计算平均值  
        total_wear += g_wear_table[i].wear_count;  
        valid_blocks++;  
    }  
      
    // 计算平均磨损次数  
    if (valid_blocks > 0) {  
        g_wl_stats.avg_wear_count = (uint32_t)(total_wear / valid_blocks);  
    } else {  
        g_wl_stats.avg_wear_count = 0;  
    }  
      
    // 如果没有有效块，重置最小磨损次数  
    if (valid_blocks == 0) {  
        g_wl_stats.min_wear_count = 0;  
    }  
}  
  
// 磨损均衡初始化  
status_t wl_init(void) {  
    be_log_print("Wear Leveling: Initializing...");  
      
    // 初始化磨损信息表  
    memset(g_wear_table, 0, sizeof(g_wear_table));  
    memset(&g_wl_stats, 0, sizeof(g_wl_stats));  
      
    // 从硬件读取初始磨损信息  
    for (uint32_t i = 0; i < WL_TOTAL_BLOCKS; i++) {  
        uint32_t hw_wear_count;  
        uint64_t block_addr = i * WL_BLOCK_SIZE;  
          
        // 从PCM控制器读取硬件磨损计数  
        status_t status = pcm_get_block_wear_count(block_addr, &hw_wear_count);  
        if (status == STATUS_SUCCESS) {  
            g_wear_table[i].wear_count = hw_wear_count;  
            g_wear_table[i].status = (hw_wear_count > WL_MAX_ENDURANCE * 0.9) ?   
                                   WL_BLOCK_STATUS_WORN : WL_BLOCK_STATUS_GOOD;  
            g_wear_table[i].last_access_time = 0;  
            g_wear_table[i].is_hot = false;  
        } else {  
            // 硬件读取失败，标记为坏块  
            g_wear_table[i].status = WL_BLOCK_STATUS_BAD;  
            g_wear_table[i].wear_count = 0xFFFFFFFF;  
        }  
    }  
      
    // 计算初始统计信息  
    wl_update_statistics();  
      
    wl_initialized = true;  
    be_log_print("Wear Leveling: Initialized with %d total blocks", WL_TOTAL_BLOCKS);  
    be_log_print("Wear Leveling: Good=%d, Worn=%d, Bad=%d",   
                 g_wl_stats.good_blocks, g_wl_stats.worn_blocks, g_wl_stats.bad_blocks);  
      
    return STATUS_SUCCESS;  
}  

// 获取磨损均衡统计信息  
status_t wl_get_statistics(wl_statistics_t *stats) {  
    if (!wl_initialized || !stats) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 更新最新统计信息  
    wl_update_statistics();  
      
    // 复制统计数据  
    memcpy(stats, &g_wl_stats, sizeof(wl_statistics_t));  
      
    return STATUS_SUCCESS;  
}  
  
// 磨损均衡扫描任务  
void wl_scan_task(ULONG initial_input) {  
    be_log_print("WL: Scan task started");  
      
    while (1) {  
        // 等待扫描间隔  
        tx_thread_sleep(WL_SCAN_INTERVAL_MS);  
          
        if (!wl_initialized) {  
            continue;  
        }  
          
        be_log_print("WL: Starting wear level scan...");  
          
        // 扫描所有块的磨损状态  
        for (uint32_t i = 0; i < WL_TOTAL_BLOCKS; i++) {  
            wl_block_info_t info;  
            if (wl_get_block_wear_info(i, &info) == STATUS_SUCCESS) {  
                // 检查是否需要标记为热块  
                if (info.wear_count > g_wl_stats.avg_wear_count + WL_WEAR_THRESHOLD) {  
                    g_wear_table[i].is_hot = true;  
                } else {  
                    g_wear_table[i].is_hot = false;  
                }  
            }  
        }  
          
        // 更新统计信息  
        wl_update_statistics();  
          
        be_log_print("WL: Scan completed. Good=%d, Worn=%d, Bad=%d, Avg_wear=%d",  
                     g_wl_stats.good_blocks, g_wl_stats.worn_blocks,   
                     g_wl_stats.bad_blocks, g_wl_stats.avg_wear_count);  
    }  
}  
  
// 磨损均衡任务  
void wl_balance_task(ULONG initial_input) {  
    be_log_print("WL: Balance task started");  
      
    while (1) {  
        // 等待均衡间隔  
        tx_thread_sleep(WL_BALANCE_INTERVAL_MS);  
          
        if (!wl_initialized) {  
            continue;  
        }  
          
        be_log_print("WL: Starting wear leveling balance...");  
          
        // 查找热块和冷块  
        uint32_t hot_block, cold_block;  
        status_t hot_status = wl_find_hot_block(&hot_block);  
        status_t cold_status = wl_find_cold_block(&cold_block);  
          
        if (hot_status == STATUS_SUCCESS && cold_status == STATUS_SUCCESS) {  
            // 检查磨损差异是否超过阈值  
            uint32_t wear_diff = g_wear_table[hot_block].wear_count -   
                                g_wear_table[cold_block].wear_count;  
              
            if (wear_diff > WL_WEAR_THRESHOLD) {  
                // 执行磨损均衡  
                status_t balance_status = wl_perform_balance(hot_block, cold_block);  
                if (balance_status == STATUS_SUCCESS) {  
                    be_log_print("WL: Balance operation successful");  
                } else {  
                    be_log_print("WL: Balance operation failed with status %d", balance_status);  
                }  
            } else {  
                be_log_print("WL: Wear levels are balanced, no action needed");  
            }  
        } else {  
            be_log_print("WL: Could not find suitable blocks for balancing");  
        }  
    }  
}