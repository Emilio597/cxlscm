#include "hw_registers.h"  
#include "pcm_registers.h"  
#include "be_common.h"  
  
typedef struct {  
    bool initialized;  
    uint32_t cell_size;          // PCM单元大小 (字节)  
    uint32_t block_size;         // 块大小  
    uint64_t total_capacity;     // 总容量  
    uint32_t read_latency_ns;    // 读延迟 (纳秒)  
    uint32_t write_latency_ns;   // 写延迟 (纳秒)  
    uint32_t set_latency_ns;     // SET操作延迟  
    uint32_t reset_latency_ns;   // RESET操作延迟  
    uint32_t thermal_threshold;  // 热阈值  
    uint32_t wear_threshold;     // 磨损阈值  
} pcm_controller_t;  
  
static pcm_controller_t pcm_ctrl = {0};  
  
// PCM控制器初始化  
status_t pcm_controller_init(void) {  
    // 软复位PCM控制器  
    REG_WRITE(PCM_CTRL, PCM_CTRL_RESET);  
      
    // 等待复位完成  
    while (REG_READ(PCM_CTRL) & PCM_CTRL_RESET) {  
        // 等待复位位自动清零  
    }  
      
    // 配置读时序参数 (PCM读取相对较快，约100ns)  
    REG_WRITE(PCM_TIMING_READ, 0x00000064);  // 100ns读延迟  
      
    // 配置写时序参数 (PCM写入较慢，约1us)  
    REG_WRITE(PCM_TIMING_WRITE, 0x000003E8); // 1000ns写延迟  
      
    // 配置功耗控制  
    REG_WRITE(PCM_POWER_CTRL, 0x00000001);   // 启用动态功耗管理  
      
    // 配置热管理阈值 (85°C)  
    REG_WRITE(PCM_THERMAL_CTRL, 85);  
      
    // 启用PCM控制器和各种管理功能  
    REG_WRITE(PCM_CTRL, PCM_CTRL_ENABLE | PCM_CTRL_THERMAL_EN | PCM_CTRL_WEAR_LEVEL_EN);  
      
    // 初始化PCM参数  
    pcm_ctrl.cell_size = 1;              // 1字节单元  
    pcm_ctrl.block_size = 4096;          // 4KB块  
    pcm_ctrl.total_capacity = 16ULL * 1024 * 1024 * 1024; // 16GB  
    pcm_ctrl.read_latency_ns = 100;      // 100ns读延迟  
    pcm_ctrl.write_latency_ns = 1000;    // 1us写延迟  
    pcm_ctrl.set_latency_ns = 500;       // 500ns SET延迟  
    pcm_ctrl.reset_latency_ns = 50;      // 50ns RESET延迟  
    pcm_ctrl.thermal_threshold = 85;     // 85°C热阈值  
    pcm_ctrl.wear_threshold = 1000000;   // 100万次写入阈值  
    pcm_ctrl.initialized = true;  
      
    return STATUS_SUCCESS;  
}  
  
// 等待PCM就绪  
static status_t pcm_wait_ready(uint32_t timeout_us) {  
    uint32_t timeout = timeout_us;  
      
    while (timeout--) {  
        uint32_t status = REG_READ(PCM_STATUS);  
        if ((status & PCM_STATUS_READY) && !(status & PCM_STATUS_BUSY)) {  
            return STATUS_SUCCESS;  
        }  
          
        // 检查错误状态  
        if (status & PCM_STATUS_ERROR) {  
            return STATUS_ERROR;  
        }  
          
        // 1微秒延时  
        for (volatile int i = 0; i < 24; i++);  // 假设24MHz时钟  
    }  
      
    return STATUS_TIMEOUT;  
}  
  
// PCM读取操作  
status_t pcm_read(uint64_t address, uint8_t *buffer, uint32_t size) {  
    if (!pcm_ctrl.initialized || !buffer || size == 0) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 检查地址范围  
    if (address + size > pcm_ctrl.total_capacity) {  
        return STATUS_OUT_OF_RANGE;  
    }  
      
    status_t status;  
    uint32_t remaining = size;  
    uint64_t current_addr = address;  
    uint8_t *current_buf = buffer;  
      
    while (remaining > 0) {  
        // 等待PCM就绪  
        status = pcm_wait_ready(1000);  // 1ms超时  
        if (status != STATUS_SUCCESS) {  
            return status;  
        }  
          
        // 设置地址  
        REG_WRITE(PCM_ADDR_LOW, (uint32_t)(current_addr & 0xFFFFFFFF));  
        REG_WRITE(PCM_ADDR_HIGH, (uint32_t)(current_addr >> 32));  
          
        // 发送读命令  
        REG_WRITE(PCM_CMD, PCM_CMD_READ);  
          
        // 等待读取完成  
        status = pcm_wait_ready(pcm_ctrl.read_latency_ns / 1000 + 1);  
        if (status != STATUS_SUCCESS) {  
            return status;  
        }  
          
        // 读取数据  
        uint32_t data = REG_READ(PCM_DATA);  
        uint32_t bytes_to_copy = (remaining >= 4) ? 4 : remaining;  
          
        for (uint32_t i = 0; i < bytes_to_copy; i++) {  
            current_buf[i] = (data >> (i * 8)) & 0xFF;  
        }  
          
        current_addr += bytes_to_copy;  
        current_buf += bytes_to_copy;  
        remaining -= bytes_to_copy;  
    }  
      
    return STATUS_SUCCESS;  
}  
  
// PCM写入操作  
status_t pcm_write(uint64_t address, const uint8_t *buffer, uint32_t size) {  
    if (!pcm_ctrl.initialized || !buffer || size == 0) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 检查地址范围  
    if (address + size > pcm_ctrl.total_capacity) {  
        return STATUS_OUT_OF_RANGE;  
    }  
      
    status_t status;  
    uint32_t remaining = size;  
    uint64_t current_addr = address;  
    const uint8_t *current_buf = buffer;  
      
    while (remaining > 0) {  
        // 等待PCM就绪  
        status = pcm_wait_ready(1000);  // 1ms超时  
        if (status != STATUS_SUCCESS) {  
            return status;  
        }  
          
        // 检查热状态  
        uint32_t pcm_status = REG_READ(PCM_STATUS);  
        if (pcm_status & PCM_STATUS_THERMAL_ALERT) {  
            return STATUS_THERMAL_ERROR;  
        }  
          
        // 设置地址  
        REG_WRITE(PCM_ADDR_LOW, (uint32_t)(current_addr & 0xFFFFFFFF));  
        REG_WRITE(PCM_ADDR_HIGH, (uint32_t)(current_addr >> 32));  
          
        // 准备数据  
        uint32_t data = 0;  
        uint32_t bytes_to_write = (remaining >= 4) ? 4 : remaining;  
          
        for (uint32_t i = 0; i < bytes_to_write; i++) {  
            data |= ((uint32_t)current_buf[i]) << (i * 8);  
        }  
          
        REG_WRITE(PCM_DATA, data);  
          
        // 发送写命令  
        REG_WRITE(PCM_CMD, PCM_CMD_WRITE);  
          
        // 等待写入完成 (PCM写入需要更长时间)  
        status = pcm_wait_ready(pcm_ctrl.write_latency_ns / 1000 + 10);  
        if (status != STATUS_SUCCESS) {  
            return status;  
        }  
          
        current_addr += bytes_to_write;  
        current_buf += bytes_to_write;  
        remaining -= bytes_to_write;  
    }  
      
    return STATUS_SUCCESS;  
}  
  
// PCM SET/RESET操作 (相变存储器特有)  
status_t pcm_set_reset_operation(uint64_t address, bool is_set) {  
    if (!pcm_ctrl.initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    // 等待PCM就绪  
    status_t status = pcm_wait_ready(1000);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 设置地址  
    REG_WRITE(PCM_ADDR_LOW, (uint32_t)(address & 0xFFFFFFFF));  
    REG_WRITE(PCM_ADDR_HIGH, (uint32_t)(address >> 32));  
      
    // 设置操作类型数据  
    REG_WRITE(PCM_DATA, is_set ? 1 : 0);  
      
    // 发送SET/RESET命令  
    REG_WRITE(PCM_CMD, PCM_CMD_SET_RESET);  
      
    // 等待操作完成  
    uint32_t latency = is_set ? pcm_ctrl.set_latency_ns : pcm_ctrl.reset_latency_ns;  
    status = pcm_wait_ready(latency / 1000 + 1);  
      
    return status;  
} 


// 获取PCM热状态  
status_t pcm_get_thermal_status(uint32_t *temperature) {  
    if (!pcm_ctrl.initialized || !temperature) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 等待PCM就绪  
    status_t status = pcm_wait_ready(1000);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 发送热读取命令  
    REG_WRITE(PCM_CMD, PCM_CMD_THERMAL_READ);  
      
    // 等待读取完成  
    status = pcm_wait_ready(100);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 读取温度数据  
    *temperature = REG_READ(PCM_DATA) & 0xFF;  // 温度值在低8位  
      
    return STATUS_SUCCESS;  
}  
  
// 获取PCM磨损均衡状态  
status_t pcm_get_wear_level_status(uint32_t *wear_count) {  
    if (!pcm_ctrl.initialized || !wear_count) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 等待PCM就绪  
    status_t status = pcm_wait_ready(1000);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 发送磨损均衡读取命令  
    REG_WRITE(PCM_CMD, PCM_CMD_WEAR_LEVEL_READ);  
      
    // 等待读取完成  
    status = pcm_wait_ready(100);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 读取磨损计数  
    *wear_count = REG_READ(PCM_DATA);  
      
    return STATUS_SUCCESS;  
}  
  
// PCM块擦除操作  
status_t pcm_erase_block(uint64_t block_address) {  
    if (!pcm_ctrl.initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    // 检查地址对齐  
    if (block_address % pcm_ctrl.block_size != 0) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 等待PCM就绪  
    status_t status = pcm_wait_ready(1000);  
    if (status != STATUS_SUCCESS) {  
        return status;  
    }  
      
    // 设置块地址  
    REG_WRITE(PCM_ADDR_LOW, (uint32_t)(block_address & 0xFFFFFFFF));  
    REG_WRITE(PCM_ADDR_HIGH, (uint32_t)(block_address >> 32));  
      
    // 发送擦除命令  
    REG_WRITE(PCM_CMD, PCM_CMD_ERASE);  
      
    // 等待擦除完成 (擦除操作通常需要更长时间)  
    status = pcm_wait_ready(10000);  // 10ms超时  
      
    return status;  
}  
  
// PCM控制器去初始化  
status_t pcm_controller_deinit(void) {  
    if (!pcm_ctrl.initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    // 禁用PCM控制器  
    REG_WRITE(PCM_CTRL, 0);  
      
    // 清零控制结构  
    memset(&pcm_ctrl, 0, sizeof(pcm_ctrl));  
      
    return STATUS_SUCCESS;  
}