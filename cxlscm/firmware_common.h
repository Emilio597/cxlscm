#ifndef FIRMWARE_COMMON_H
#define FIRMWARE_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "tx_api.h" // ThreadX API

// 统一的状态返回码
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR,
    STATUS_INVALID_PARAM,
    STATUS_NOT_INITIALIZED,
    STATUS_TIMEOUT,
    STATUS_OUT_OF_RANGE,
    STATUS_THERMAL_ERROR,
    STATUS_INVALID_OPERATION,
    STATUS_NOT_FOUND,
    STATE_FORMATTING,
    STATUS_OUT_OF_MEMORY,
    STATE_INIT,
    STATE_RESTORING,
    STATE_RUNNING_BG_TASKS
} status_t;

// FE <-> BE 消息定义
typedef struct {
    uint32_t event_id;
    uint32_t payload[16];
} fe_be_message_t;

// 消息事件ID
#define BE_EVENT_POWER_UP_DONE    0xBE000001
#define BE_RESPONSE_SMART_DATA    0xBE000002
#define BE_EVENT_MEDIA_ERROR      0xBE000003
#define FE_CMD_GET_SMART_DATA     0xFE000001
#define FE_CMD_WRITE_DATA         0xFE000002
#define FE_CMD_READ_DATA          0xFE000003
// 共享的全局变量 (由start_threadx.c定义)
extern TX_QUEUE fe_be_queue;

// 共享的日志函数
void fe_log_print(const char* format, ...);
void be_log_print(const char* format, ...);

// 线程入口函数声明
void fe_main_thread_entry(ULONG thread_input);
void be_main_thread_entry(ULONG thread_input);


#endif // FIRMWARE_COMMON_H