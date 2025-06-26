

/*
 * ============================================================================
 * 文件: fe/src/hw_access.c
 * 描述: 硬件寄存器的读写，提供硬件抽象。
 * ============================================================================
 */
#include "fe_common.h"
#include "hw_registers.h"

// 使用一个足够大的数组来CXL设备的寄存器空间
static uint32_t simulated_regs[1024]; // 4KB的寄存器空间

void hw_init_registers(void) {
    memset(simulated_regs, 0, sizeof(simulated_regs));
    
    // 根据日志中的'rd'值预设一些寄存器的值，以硬件上电后的默认状态
    simulated_regs[REG_CLASS_CODE_REVISION_ID / 4] = VAL_RD_CLASS_CODE_REVISION_ID;
    simulated_regs[REG_LINK_CONTROL_LINK_STATUS / 4] = VAL_RD_LINK_CONTROL_LINK_STATUS;
    simulated_regs[REG_FLEXBUS_CNTRL_STATUS_OFF / 4] = VAL_RD_FLEXBUS_CNTRL_STATUS_OFF;
    // ... 可以预设更多寄存器的值
}

uint32_t hw_reg_read(uint32_t offset) {
    if (offset / 4 >= sizeof(simulated_regs) / sizeof(uint32_t)) {
        return 0xFFFFFFFF; // 地址越界
    }
    
    // 特殊处理，读操作会返回与写入值不同的情况（例如状态位更新）
    switch(offset) {
        case REG_LINK_CONTROL_LINK_STATUS:
            // 链路状态从训练中变为激活
            simulated_regs[offset/4] |= 0x00110000; // 假设这些是状态位
            break;
        case REG_CXL_DVSEC_RANGE1_SIZE_HIGH:
             return VAL_RD_CXL_DVSEC_RANGE1_SIZE_HIGH;
        case REG_CXL_DVSEC_RANGE1_SIZE_LOW:
             return VAL_RD_CXL_DVSEC_RANGE1_SIZE_LOW;
    }
    
    return simulated_regs[offset / 4];
}

void hw_reg_write(uint32_t offset, uint32_t value) {
    if (offset / 4 >= sizeof(simulated_regs) / sizeof(uint32_t)) {
        return; // 地址越界
    }
    simulated_regs[offset / 4] = value;
}


