#ifndef CXL_REGISTERS_H  
#define CXL_REGISTERS_H  
  
#include <stdint.h>  
  
/* CXL寄存器基地址 */  
#define CXL_REG_BASE                    0x80000000  
  
/* PCIe配置寄存器 */  
#define CLASS_CODE_REVISION_ID_REG      0x08  
#define DESIGN_VENDOR_SPEC_HEADER2_REG  0x100  
  
/* CXL DVSEC寄存器 */  
#define CXL_DVSEC_RANGE1_SIZE_HIGH_REG  0x18C  
#define CXL_DVSEC_RANGE1_SIZE_LOW_REG   0x190  
#define CXL_DVSEC_RANGE2_SIZE_LOW_REG   0x194  
  
/* CXL控制寄存器 */  
#define FLEXBUS_CNTRL_STATUS_OFF_REG    0x200  
#define CXL_M2S_CTL_EN_REG              0x204  
  
/* PCIe链路控制寄存器 */  
#define LINK_CONTROL_LINK_STATUS_REG    0x80  
#define PL32G_CONTROL_REG               0x8A8  
#define SYMBOL_TIMER_FILTER_1_OFF_REG   0x71C  
#define PM_PROCESS_CTL_REG              0x720  
  
/* PCIe均衡寄存器 */  
#define GEN3_RELATED_OFF_REG            0x890  
#define GEN3_EQ_LOCAL_FS_LF_OFF_REG     0x894  
#define GEN3_EQ_FB_MODE_DIR_CHANGE_OFF_REG 0x898  
#define GEN3_EQ_CONTROL_OFF_REG         0x89C  
  
/* 寄存器默认值 */  
#define CLASS_CODE_REVISION_ID_VAL      0x05021001  
#define DESIGN_VENDOR_SPEC_HEADER2_VAL  0xc39e0000  
#define FLEXBUS_CNTRL_STATUS_VAL        0x26  
#define CXL_M2S_CTL_EN_VAL              0x70007  
#define SYMBOL_TIMER_FILTER_1_VAL       0x400140  
#define PM_PROCESS_CTL_VAL              0x3  
#define LINK_CONTROL_LINK_STATUS_VAL    0x10000040  
#define PL32G_CONTROL_VAL               0x3  
#define GEN3_RELATED_OFF_VAL            0x2800  
#define GEN3_EQ_LOCAL_FS_LF_OFF_VAL     0xc10  
#define GEN3_EQ_FB_MODE_DIR_CHANGE_OFF_VAL 0x0  
#define GEN3_EQ_CONTROL_OFF_VAL         0xc07ff71  
  
/* GEN4配置值 */  
#define GEN4_GEN3_RELATED_OFF_VAL       0x1002800  
#define GEN4_GEN3_EQ_CONTROL_OFF_VAL    0x507ff31  
  
/* GEN5配置值 */  
#define GEN5_GEN3_RELATED_OFF_VAL       0x2002800  
#define GEN5_GEN3_EQ_CONTROL_OFF_VAL    0x407ff31  
  
/* 寄存器操作函数 */  
uint32_t cxl_reg_read(uint32_t offset);  
void cxl_reg_write(uint32_t offset, uint32_t value);  
  
#endif
