/*
 * ============================================================================
 * 文件: be/inc/address_translation.h
 * ============================================================================
 */
#ifndef ADDRESS_TRANSLATION_H
#define ADDRESS_TRANSLATION_H
#include <stdint.h>
void at_init(void);
int at_update_mapping(uint64_t lba, uint64_t new_pba);
#endif /* ADDRESS_TRANSLATION_H */
