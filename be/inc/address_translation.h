/*
 * ============================================================================
 * 文件: be/inc/address_translation.h
 * ============================================================================
 */
#ifndef ADDRESS_TRANSLATION_H
#define ADDRESS_TRANSLATION_H
#include <stdint.h>
#include "be_common.h"

#define ADDRESS_TRANSLATION_TABLE_BASE 0x90000a0000
typedef struct {
    uint64_t physical_block_address;
    bool is_valid;
} address_map_entry_t;

void at_init(void);
int at_update_mapping(uint64_t lba, uint64_t new_pba);
#endif /* ADDRESS_TRANSLATION_H */
