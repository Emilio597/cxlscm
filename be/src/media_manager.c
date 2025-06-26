#include "be_common.h"
#include "media_manager.h"
#include "bbm.h"
#include "pcm_controller.h"
#define WEAR_LEVEL_BLOCK_COUNT 1024
static uint32_t g_erase_counts[WEAR_LEVEL_BLOCK_COUNT];

// 介质管理器初始化  
be_status_t media_manager_init(void) {  
    // 初始化PCM控制器  
    be_status_t status = pcm_controller_init();  
    if (status != BE_STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 其他初始化逻辑...  
    return BE_STATUS_SUCCESS;  
}  

be_status_t media_read(uint64_t lba, uint32_t sector_count, uint8_t *buffer) {  
    if (!buffer || sector_count == 0) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    // 计算PCM物理地址 (假设每个扇区512字节)  
    uint64_t pcm_address = lba * 512;  
    uint32_t read_size = sector_count * 512;  
      
    // 调用PCM控制器读取接口  
    return pcm_read(pcm_address, buffer, read_size);  
}  
  
be_status_t media_write(uint64_t lba, uint32_t sector_count, const uint8_t *buffer) {  
    if (!buffer || sector_count == 0) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    // 计算PCM物理地址  
    uint64_t pcm_address = lba * 512;  
    uint32_t write_size = sector_count * 512;  
      
    // 调用PCM控制器写入接口  
    return pcm_write(pcm_address, buffer, write_size);  
}  
  
// 介质擦除操作  
be_status_t media_erase(uint64_t lba, uint32_t sector_count) {  
    // 计算块地址 (假设块大小4KB)  
    uint64_t block_address = (lba * 512) & ~(4096 - 1);  // 对齐到4KB边界  
      
    // 调用PCM控制器擦除接口  
    return pcm_erase_block(block_address);  
}

void media_patrol_task(ULONG initial_input) {
    be_log_print("Media Patrol task started.");
    while(1) {
        tx_thread_sleep(30000);
        be_log_print("Media Patrol: Starting a scan cycle...");
        g_be_ctrl.patrol_runs++;
        
        // 模拟扫描并发现ECC错误
        int ecc_check = rand() % 10;
        if (ecc_check < 2) { // 20%的几率发现问题
            uint64_t bad_block_addr = 0x0000123400005678 + g_be_ctrl.patrol_runs;
            be_log_print("Media Patrol: Found uncorrectable ECC error at PBA 0x%llx!", bad_block_addr);
            g_be_ctrl.uncorrectable_errors++;
            bbm_retire_block(bad_block_addr);
        }
        be_log_print("Media Patrol: Scan cycle complete.");
    }
}

void wear_leveling_task(ULONG initial_input) {
    be_log_print("Wear Leveling (WL) task started.");
    while(1) {
        tx_thread_sleep(60000);
        be_log_print("WL: Starting wear leveling cycle...");
        g_be_ctrl.wear_leveling_runs++;
        
        // 模拟I/O，增加一些块的磨损
        for(int i=0; i<50; ++i) g_erase_counts[rand() % WEAR_LEVEL_BLOCK_COUNT]++;
        
        // 查找最热和最冷的块
        uint32_t min_wear = 0xFFFFFFFF, max_wear = 0;
        int hot_idx = -1, cold_idx = -1;
        for(int i=0; i<WEAR_LEVEL_BLOCK_COUNT; ++i) {
            if (g_erase_counts[i] > max_wear) { max_wear = g_erase_counts[i]; hot_idx = i; }
            if (g_erase_counts[i] < min_wear) { min_wear = g_erase_counts[i]; cold_idx = i; }
        }

        // 如果磨损差异大于阈值，则执行交换
        if (hot_idx != -1 && cold_idx != -1 && (max_wear > min_wear + 100)) {
            be_log_print("WL: Found hot block %d (wear %d) and cold block %d (wear %d).", hot_idx, max_wear, cold_idx, min_wear);
            be_log_print("WL: Swapping data to balance wear...");
            tx_thread_sleep(50); // 模拟数据搬移
            // 交换磨损计数
            uint32_t temp = g_erase_counts[hot_idx];
            g_erase_counts[hot_idx] = g_erase_counts[cold_idx];
            g_erase_counts[cold_idx] = temp;
            be_log_print("WL: Swap complete.");
        } else {
            be_log_print("WL: Wear levels are balanced. No action needed.");
        }
    }
}
