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

// CXL SCM 外设基地址定义  
#define CXL_SCM_PERIPH_BASE         0x40000000  
  
// UART 寄存器定义 (PL011 兼容)  
#define UART_BASE                   (CXL_SCM_PERIPH_BASE + 0x10000)  
#define UART_DR                     (UART_BASE + 0x000)  // 数据寄存器  
#define UART_RSR                    (UART_BASE + 0x004)  // 接收状态寄存器  
#define UART_FR                     (UART_BASE + 0x018)  // 标志寄存器  
#define UART_IBRD                   (UART_BASE + 0x024)  // 整数波特率分频器  
#define UART_FBRD                   (UART_BASE + 0x028)  // 小数波特率分频器  
#define UART_LCR_H                  (UART_BASE + 0x02C)  // 线控制寄存器  
#define UART_CR                     (UART_BASE + 0x030)  // 控制寄存器  
#define UART_IMSC                   (UART_BASE + 0x038)  // 中断屏蔽寄存器  
#define UART_MIS                    (UART_BASE + 0x040)  // 屏蔽中断状态寄存器  
#define UART_ICR                    (UART_BASE + 0x044)  // 中断清除寄存器  
  
// UART 标志位定义  
#define UART_FR_TXFF                (1 << 5)  // 发送FIFO满  
#define UART_FR_RXFE                (1 << 4)  // 接收FIFO空  
#define UART_FR_BUSY                (1 << 3)  // UART忙  
  
// GPIO 寄存器定义  
#define GPIO_BASE                   (CXL_SCM_PERIPH_BASE + 0x20000)  
#define GPIO_DATA                   (GPIO_BASE + 0x000)  // 数据寄存器  
#define GPIO_DIR                    (GPIO_BASE + 0x400)  // 方向寄存器  
#define GPIO_IE                     (GPIO_BASE + 0x404)  // 中断使能寄存器  
#define GPIO_RIS                    (GPIO_BASE + 0x414)  // 原始中断状态  
#define GPIO_MIS                    (GPIO_BASE + 0x418)  // 屏蔽中断状态  
#define GPIO_IC                     (GPIO_BASE + 0x41C)  // 中断清除  
  
// 寄存器访问宏  
#define REG_READ(addr)              (*(volatile uint32_t*)(addr))  
#define REG_WRITE(addr, val)        (*(volatile uint32_t*)(addr) = (val))  

#endif /* HW_REGISTERS_H */

