#ifndef PCM_CONTROLLER_H  
#define PCM_CONTROLLER_H  
  
#include <stdint.h>  
#include <stdbool.h>  
#include "be_common.h"  

// 模拟PCM控制器寄存器地址
#define PCM_REG_CONTROL 0x00
#define PCM_REG_STATUS  0x04
#define PCM_REG_ADDRESS 0x08
#define PCM_REG_DATA    0x0C

// PCM控制器接口函数声明  
status_t pcm_controller_init(void);  
status_t pcm_controller_deinit(void);  
  
// PCM基本读写操作  
status_t pcm_read(uint64_t address, uint8_t *buffer, uint32_t size);  
status_t pcm_write(uint64_t address, const uint8_t *buffer, uint32_t size);  
status_t pcm_erase_block(uint64_t block_address);  
  
// PCM特有操作  
status_t pcm_set_reset_operation(uint64_t address, bool is_set);  
  
// PCM状态和监控接口  
status_t pcm_get_thermal_status(uint32_t *temperature);  
status_t pcm_get_wear_level_status(uint32_t *wear_count);  
  
// PCM磨损管理接口  
status_t pcm_get_block_wear_count(uint64_t block_address, uint32_t *wear_count);  
status_t pcm_set_block_wear_count(uint64_t block_address, uint32_t wear_count);  
  
// PCM容量和配置信息  
status_t pcm_get_capacity_info(uint64_t *total_capacity, uint32_t *block_size);  
status_t pcm_get_performance_info(uint32_t *read_latency_ns, uint32_t *write_latency_ns);  
  
#endif // PCM_CONTROLLER_H