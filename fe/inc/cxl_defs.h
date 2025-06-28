
/*
 * ============================================================================
 * 文件: fe/inc/cxl_defs.h
 * 描述: CXL协议相关的宏定义，例如CDAT结构、事件定义等。
 * ============================================================================
 */
#ifndef CXL_DEFS_H
#define CXL_DEFS_H

#include <stdint.h>

#define BE_EVENT_POWER_UP_DONE    0xBE000001
#define BE_RESPONSE_SMART_DATA    0xBE000002
#define FE_CMD_GET_SMART_DATA     0xFE000001

/* CDAT (CXL Data Object) 相关定义 */
typedef struct {
    uint32_t read_latency_ns;       // 读延迟 (纳秒)
    uint32_t write_latency_ns;      // 写延迟 (纳秒)
    uint32_t bandwidth_gbps;        // 读写带宽 (GB/s)
    uint64_t dpa_length;            // 设备物理地址长度
    uint8_t  checksum;              // 校验和
} cdat_data_t;

/* 邮箱 Poison List 相关定义 */
#define MAX_POISON_RECORDS 584

/* CXL 事件中断定义 */
typedef struct {
    uint32_t info;
    uint32_t warn;
    uint32_t fail;
    uint32_t fatal;
} event_intr_mask_t;

/* CXL 事件严重性定义 (根据日志) */
typedef enum {
    EVENT_SEVERITY_INFO,
    EVENT_SEVERITY_WARNING,
    EVENT_SEVERITY_FAILURE,
    EVENT_SEVERITY_FATAL
} event_severity_t;

/* CXL 事件类型定义 (根据日志) */
typedef enum {
    EVENT_TYPE_VENDOR_SRAM_ERROR
} event_type_t;


#endif /* CXL_DEFS_H */

