/*
 * ============================================================================
 * 新增文件: fe/inc/telemetry.h
 * 描述: 健康遥测与拓扑发现模块的接口。
 * ============================================================================
 */
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include "firmware_common.h"
#define NUM_RANKS       4
#define NUM_CHANNELS    4
#define NUM_DIES        5

// 设备物理拓扑结构
typedef struct {
    uint8_t ranks;
    uint8_t channels_per_rank;
    uint8_t dies_per_channel;
    uint32_t die_capacity_mb;
} device_topology_t;

// SMART 健康信息结构
typedef struct {
    uint32_t temperature_celsius;   // 控制器温度
    uint32_t power_on_hours;        // 通电时间
    uint32_t wear_level_percentage; // 平均磨损度
    uint32_t spare_blocks_percentage; // 备用块剩余
    uint64_t uncorrectable_errors;  // 无法纠正的错误数
} smart_health_info_t;

void telemetry_init(void);
void telemetry_get_topology(device_topology_t *topo);
int telemetry_get_smart_health(smart_health_info_t *health);

#endif /* TELEMETRY_H */
