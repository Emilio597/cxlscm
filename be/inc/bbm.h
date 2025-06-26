/*
 * ============================================================================
 * 文件: be/inc/bbm.h
 * ============================================================================
 */
#ifndef BBM_H
#define BBM_H
#include <stdint.h>
#define MAX_BAD_BLOCKS 1024
void bbm_init(void);
int bbm_retire_block(uint64_t physical_block_address);
uint64_t bbm_get_spare_block(void);
#endif /* BBM_H */