
/*
 * ============================================================================
 * 文件: fe/inc/hw_access.h
 * 描述: 硬件抽象层，提供寄存器读写接口。
 * ============================================================================
 */
#ifndef HW_ACCESS_H
#define HW_ACCESS_H

#include <stdint.h>

// 初始化的硬件寄存器空间
void hw_init_registers(void);

// 寄存器读操作
uint32_t hw_reg_read(uint32_t offset);

// 寄存器写操作
void hw_reg_write(uint32_t offset, uint32_t value);

#endif /* HW_ACCESS_H */

