#include "be_common.h"
#include "bbm.h"
#include "pcm_controller.h"
#include "address_translation.h"
#include <stdio.h>

#define SPARE_BLOCK_POOL_SIZE 100
static uint64_t g_spare_blocks[SPARE_BLOCK_POOL_SIZE];
static uint32_t g_next_spare_block_idx = 0;

static uint64_t g_bad_block_table[MAX_BAD_BLOCKS];
static uint32_t g_bad_block_count = 0;


void bbm_init(void) {
    be_log_print("Bad Block Management (BBM) module initialized.");
    for (int i = 0; i < SPARE_BLOCK_POOL_SIZE; ++i) { g_spare_blocks[i] = 0xDEADBEEF00000000 + i; }
    g_be_ctrl.spare_blocks_available = SPARE_BLOCK_POOL_SIZE;
    be_log_print("BBM: Initialized with %d spare blocks.", g_be_ctrl.spare_blocks_available);
}

uint64_t bbm_get_spare_block(void) {
    if (g_next_spare_block_idx < SPARE_BLOCK_POOL_SIZE) {
        uint64_t spare = g_spare_blocks[g_next_spare_block_idx++];
        g_be_ctrl.spare_blocks_available--;
        be_log_print("BBM: Allocated spare block 0x%llx. %d remaining.", spare, g_be_ctrl.spare_blocks_available);
        return spare;
    }
    be_log_print("BBM: ERROR! No spare blocks left!");
    return 0;
}

int bbm_retire_block(uint64_t physical_block_address) {
    be_log_print("BBM: Attempting to retire physical block 0x%llx.", physical_block_address);
    if (g_bad_block_count >= MAX_BAD_BLOCKS) { be_log_print("BBM: ERROR! Bad Block Table is full."); return -1; }
    
    uint64_t spare_pba = bbm_get_spare_block();
    if (spare_pba == 0) return -1;

    // 数据迁移
    be_log_print("BBM: Migrating data from bad block 0x%llx to spare block 0x%llx...", physical_block_address, spare_pba);
    tx_thread_sleep(10); // 耗时
    
    uint64_t lba = physical_block_address; // 简化：假设可以从PBA反查LBA
    if (at_update_mapping(lba, spare_pba) != 0) {
        be_log_print("BBM: ERROR! Failed to update mapping table for retired block.");
        return -1;
    }
    
    g_bad_block_table[g_bad_block_count++] = physical_block_address;
    be_log_print("BBM: Successfully retired block. Total bad blocks: %d.", g_bad_block_count);
    return 0;
}

// 坏块表读取  
status_t bbm_read_table(uint32_t block_index, bbm_entry_t *entry) {  
    if (!entry) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 计算坏块表在PCM中的位置  
    uint64_t bbm_address = BBM_TABLE_BASE + (block_index * sizeof(bbm_entry_t));  
      
    // 从PCM读取坏块表项  
    return pcm_read(bbm_address, (uint8_t*)entry, sizeof(bbm_entry_t));  
}  
  
// 坏块标记  
status_t bbm_mark_bad_block(uint32_t block_index) {  
    bbm_entry_t entry;  
      
    // 读取当前表项  
    status_t status = bbm_read_table(block_index, &entry);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 标记为坏块  
    entry.is_bad = true;  
    entry.wear_count = 0xFFFFFFFF;  // 坏块不再计算磨损  
      
    // 写回PCM  
    uint64_t bbm_address = BBM_TABLE_BASE + (block_index * sizeof(bbm_entry_t));  
    return pcm_write(bbm_address, (const uint8_t*)&entry, sizeof(bbm_entry_t));  
}  
  
// 磨损均衡更新  
status_t bbm_update_wear_count(uint32_t block_index) {  
    bbm_entry_t entry;  
      
    // 读取当前表项  
    status_t status = bbm_read_table(block_index, &entry);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 检查是否为坏块  
    if (entry.is_bad) {  
        return STATUS_INVALID_OPERATION;  
    }  
      
    // 增加磨损计数  
    entry.wear_count++;  
      
    // 写回PCM  
    uint64_t bbm_address = BBM_TABLE_BASE + (block_index * sizeof(bbm_entry_t));  
    return pcm_write(bbm_address, (const uint8_t*)&entry, sizeof(bbm_entry_t));  
}