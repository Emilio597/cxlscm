#include "be_common.h"
#include "bbm.h"
#include "address_translation.h"

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
