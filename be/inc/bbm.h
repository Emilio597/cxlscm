/*
 * ============================================================================
 * 文件: be/inc/bbm.h
 * ============================================================================
 */
#ifndef BBM_H
#define BBM_H
#include <stdint.h>
#include "be_common.h"
#define MAX_BAD_BLOCKS 1024
#define BBM_TABLE_BASE 0x80000000 // 假设的BBM表物理基地址

typedef enum {
    BBM_OP_RETIRE_BLOCK
} bbm_opcode_t;


typedef struct {
    bbm_opcode_t opcode;
    uint64_t logical_block_address;
} bbm_request_t;

// BBM表中的条目结构
typedef struct {
    bool is_bad;
    uint32_t wear_count;
} bbm_entry_t;

void bbm_init(void);
int bbm_retire_block(uint64_t physical_block_address);
status_t bbm_read_table(uint32_t block_index, bbm_entry_t *entry);
status_t bbm_mark_bad_block(uint32_t block_index);
status_t bbm_update_wear_count(uint32_t block_index);
#endif /* BBM_H */