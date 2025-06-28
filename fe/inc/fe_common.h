/*
 * ============================================================================
 * 文件: fe/inc/fe_common.h
 * 描述: FE固件的通用定义，包括状态机、全局控制块和公共函数声明。
 * ============================================================================
 */
#ifndef FE_COMMON_H
#define FE_COMMON_H

#include "tx_api.h" // ThreadX API
#include "firmware_common.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

/* FE状态机定义 (新增FW_UPDATE状态) */
typedef enum {
    FE_STATE_RESET = 0,             // 复位状态
    FE_STATE_WAITING_FOR_BE,        // 等待后端(BE)上电
    FE_STATE_POWER_UP_INIT,         // 上电初始化
    FE_STATE_MEMORY_TRAINING,       // 内存训练
    FE_STATE_CXL_CONFIG,            // CXL 和 PCIe 核心配置
    FE_STATE_WAITING_FOR_HOST,      // 等待主机使能
    FE_STATE_HOST_HANDSHAKE,        // 与主机握手，处理邮箱命令
    FE_STATE_NORMAL_OPERATION,      // 正常运行
    FE_STATE_FW_UPDATE,             // 固件更新中
    FE_STATE_ERROR                  // 错误状态
} fe_state_t;

/* 关机状态定义 */
typedef enum {
    SHUTDOWN_STATE_CLEAN = 0,       // 清洁关机
    SHUTDOWN_STATE_DIRTY            // 异常掉电
} shutdown_state_t;

/* 时间戳结构 */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} timestamp_t;

/* FE 全局控制块 */
typedef struct {
    fe_state_t state;               // 当前FE状态
    timestamp_t current_time;       // 当前固件时间
    TX_MUTEX   log_mutex;           // 日志打印互斥锁
    UINT be_power_up_done;          // BE上电完成标志
    UINT host_cxl_enabled;          // 主机已使能CXL.mem标志
    shutdown_state_t last_shutdown_state; // 上次关机状态
} fe_control_t;

/* BE -> FE 消息队列的消息结构 */
typedef struct {
    uint32_t event_id;
    uint32_t payload[3];
} be_fe_message_t;

#define BE_EVENT_POWER_UP_DONE 0xBE000001 // BE上电完成事件

/* 全局变量声明 */
extern fe_control_t g_fe_ctrl;
extern TX_QUEUE be_to_fe_queue;

/* 函数声明 */
// 日志系统
void fe_log_init(void);
void fe_log_print(const char* format, ...);
void fe_timestamp_update(void);

// 状态处理函数
void fe_handle_state_reset(void);
void fe_handle_state_waiting_for_be(void);
void fe_handle_state_power_up_init(void);
void fe_handle_state_memory_training(void);
void fe_handle_state_cxl_config(void);
void fe_handle_state_waiting_for_host(void);
void fe_handle_state_host_handshake(void);
void fe_handle_state_normal_operation(void);
void fe_handle_state_fw_update(void);

// 初始化流程函数
void fe_power_up_sequence(void);
void fe_cxl_initialization_sequence(void);
void fe_host_handshake_sequence(void);
void fe_media_status_monitor_task(void);
void fe_process_be_message(fe_be_message_t *msg);  
void fe_periodic_tasks(void);  

#endif /* FE_COMMON_H */

