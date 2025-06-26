/*
 * ============================================================================
 * 文件: fe/inc/hw_registers.h
 * 描述: 的硬件寄存器地址偏移量和关键值定义。
 * ============================================================================
 */
#ifndef HW_REGISTERS_H
#define HW_REGISTERS_H

#include <stdint.h>

/* CXL设备寄存器基地址 */
#define CXL_REG_BASE                    0x80000000

/* PCIe & CXL.io 寄存器偏移量 */
#define REG_CLASS_CODE_REVISION_ID      0x0008
#define REG_LINK_CONTROL_LINK_STATUS    0x0080
#define REG_DESIGN_VENDOR_SPEC_HEADER2  0x0100
#define REG_CXL_DVSEC_RANGE1_SIZE_HIGH  0x018C
#define REG_CXL_DVSEC_RANGE1_SIZE_LOW   0x0190
#define REG_CXL_DVSEC_RANGE2_SIZE_LOW   0x0194
#define REG_FLEXBUS_CNTRL_STATUS_OFF    0x0200
#define REG_CXL_M2S_CTL_EN              0x0204 // 假设地址

/* CXL.mem & CXL.cache 寄存器偏移量 (假设) */
#define REG_PM_PROCESS_CTL              0x0720
#define REG_SYMBOL_TIMER_FILTER_1_OFF   0x071C
#define REG_GEN3_RELATED_OFF            0x0890
#define REG_GEN3_EQ_LOCAL_FS_LF_OFF     0x0894
#define REG_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF 0x0898
#define REG_GEN3_EQ_CONTROL_OFF         0x089C
#define REG_PL32G_CONTROL               0x08A8

/*
 * 寄存器预设值 (根据日志)
 * 这些值用于在 cxl_reg_read/write 中硬件行为
 */

// 写值 (W) 和 期望读回值 (RD)
#define VAL_W_CLASS_CODE_REVISION_ID      0x05021001
#define VAL_RD_CLASS_CODE_REVISION_ID     0x05021001

#define VAL_W_DESIGN_VENDOR_SPEC_HEADER2  0xc39e0000
#define VAL_RD_DESIGN_VENDOR_SPEC_HEADER2 0xc39e0000

#define VAL_W_CXL_DVSEC_RANGE2_SIZE_LOW   0
#define VAL_RD_CXL_DVSEC_RANGE2_SIZE_LOW  0

#define VAL_W_CXL_DVSEC_RANGE1_SIZE_HIGH  0x1 // 写入1，但日志显示读回'a'，这可能是用户能力(Usr Cap)的一部分
#define VAL_RD_CXL_DVSEC_RANGE1_SIZE_HIGH 0xa

#define VAL_W_CXL_DVSEC_RANGE1_SIZE_LOW   0x804b // 日志中为 ____804b, 假设写入低16位
#define VAL_RD_CXL_DVSEC_RANGE1_SIZE_LOW  0xd000804b

#define VAL_RD_FLEXBUS_CNTRL_STATUS_OFF 0x26

#define VAL_W_CXL_M2S_CTL_EN              0x70007
#define VAL_RD_CXL_M2S_CTL_EN             0x70007

#define VAL_W_SYMBOL_TIMER_FILTER_1_OFF   0x400140
#define VAL_RD_SYMBOL_TIMER_FILTER_1_OFF  0x400140

#define VAL_W_PM_PROCESS_CTL              0x3
#define VAL_RD_PM_PROCESS_CTL             0x3

#define VAL_W_LINK_CONTROL_LINK_STATUS    0x10000040
#define VAL_RD_LINK_CONTROL_LINK_STATUS   0x10110040 // 读回时状态位已更新

#define VAL_W_PL32G_CONTROL               0x3
#define VAL_RD_PL32G_CONTROL              0x3

// GEN3
#define VAL_W_GEN3_RELATED_OFF            0x2800
#define VAL_W_GEN3_EQ_LOCAL_FS_LF_OFF     0xc10
#define VAL_W_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF 0x0
#define VAL_W_GEN3_EQ_CONTROL_OFF         0xc07ff71

// GEN4->GEN3
#define VAL_RD_GEN4_GEN3_RELATED_OFF      0x1002800
#define VAL_RD_GEN4_GEN3_EQ_CONTROL_OFF   0x507ff31

// GEN5->GEN3
#define VAL_RD_GEN5_GEN3_RELATED_OFF      0x2002800
#define VAL_RD_GEN5_GEN3_EQ_CONTROL_OFF   0x407ff31

#endif /* HW_REGISTERS_H */

