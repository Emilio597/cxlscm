#ifndef PCM_REGISTERS_H  
#define PCM_REGISTERS_H  
  
#include <stdint.h>  
  
// PCM控制器寄存器定义  
#define PCM_CTRL_BASE               (CXL_SCM_PERIPH_BASE + 0x100000)  
#define PCM_CMD                     (PCM_CTRL_BASE + 0x000)  // 命令寄存器  
#define PCM_ADDR_LOW                (PCM_CTRL_BASE + 0x004)  // 地址寄存器低32位  
#define PCM_ADDR_HIGH               (PCM_CTRL_BASE + 0x008)  // 地址寄存器高32位  
#define PCM_DATA                    (PCM_CTRL_BASE + 0x00C)  // 数据寄存器  
#define PCM_STATUS                  (PCM_CTRL_BASE + 0x010)  // 状态寄存器  
#define PCM_CTRL                    (PCM_CTRL_BASE + 0x014)  // 控制寄存器  
#define PCM_TIMING_READ             (PCM_CTRL_BASE + 0x018)  // 读时序寄存器  
#define PCM_TIMING_WRITE            (PCM_CTRL_BASE + 0x01C)  // 写时序寄存器  
#define PCM_POWER_CTRL              (PCM_CTRL_BASE + 0x020)  // 功耗控制寄存器  
#define PCM_THERMAL_CTRL            (PCM_CTRL_BASE + 0x024)  // 热管理寄存器  
#define PCM_WEAR_LEVEL              (PCM_CTRL_BASE + 0x028)  // 磨损均衡寄存器  
#define PCM_ERROR_STATUS            (PCM_CTRL_BASE + 0x02C)  // 错误状态寄存器  
#define PCM_ENDURANCE_CNT           (PCM_CTRL_BASE + 0x030)  // 耐久性计数器  
  
// PCM命令定义  
#define PCM_CMD_READ                0x01  
#define PCM_CMD_WRITE               0x02  
#define PCM_CMD_ERASE               0x03  
#define PCM_CMD_SET_RESET           0x04  // SET/RESET操作  
#define PCM_CMD_THERMAL_READ        0x05  
#define PCM_CMD_WEAR_LEVEL_READ     0x06  
  
// PCM状态位定义  
#define PCM_STATUS_READY            (1 << 0)  // 就绪状态  
#define PCM_STATUS_BUSY             (1 << 1)  // 忙状态  
#define PCM_STATUS_ERROR            (1 << 2)  // 错误状态  
#define PCM_STATUS_THERMAL_ALERT    (1 << 3)  // 热警告  
#define PCM_STATUS_WEAR_ALERT       (1 << 4)  // 磨损警告  
#define PCM_STATUS_POWER_GOOD       (1 << 5)  // 电源正常  
  
// PCM控制位定义  
#define PCM_CTRL_ENABLE             (1 << 0)  // 使能PCM控制器  
#define PCM_CTRL_RESET              (1 << 1)  // 软复位  
#define PCM_CTRL_POWER_SAVE         (1 << 2)  // 省电模式  
#define PCM_CTRL_THERMAL_EN         (1 << 3)  // 热管理使能  
#define PCM_CTRL_WEAR_LEVEL_EN      (1 << 4)  // 磨损均衡使能  
  
#endif // PCM_REGISTERS_H