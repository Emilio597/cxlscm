
/*
 * ============================================================================
 * 新增文件: fe/src/telemetry.c
 * 描述: 实现健康遥测与拓扑发现功能。
 * ============================================================================
 */
#include "fe_common.h"
#include "telemetry.h"

static device_topology_t g_topo;
static smart_health_info_t g_health;

void telemetry_init(void) {
    fe_log_print("Telemetry module initialized.");
    
    // 初始化拓扑信息
    g_topo.ranks = NUM_RANKS;
    g_topo.channels_per_rank = NUM_CHANNELS;
    g_topo.dies_per_channel = NUM_DIES;
    g_topo.die_capacity_mb = 4096; // 假设每个die是4GB

    // 初始化健康信息
    g_health.power_on_hours = 1250; // 假设已经运行了一段时间
    g_health.uncorrectable_errors = 2;
}

void telemetry_get_topology(device_topology_t *topo) {
    if (!topo) return;
    memcpy(topo, &g_topo, sizeof(device_topology_t));
    fe_log_print("Responded to Get_Topology: %d ranks, %d chan/rank, %d die/chan.", 
                 g_topo.ranks, g_topo.channels_per_rank, g_topo.dies_per_channel);
}

int telemetry_get_smart_health(smart_health_info_t *health) {
    if (!health) return -1;
    fe_be_message_t req_msg, resp_msg;
    req_msg.event_id = FE_CMD_GET_SMART_DATA;
    
    fe_log_print("Telemetry: Sending request for SMART data to BE.");
    // 将请求发送给BE
    UINT status = tx_queue_send(&fe_be_queue, &req_msg, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS) {
        fe_log_print("Telemetry: Failed to send msg to BE queue.");
        return -1;
    }

    // 等待BE的响应 (在真实系统中，这可能通过另一个队列完成，此处简化)
    status = tx_queue_receive(&fe_be_queue, &resp_msg, 500); // 等待5秒
    if (status != TX_SUCCESS || resp_msg.event_id != BE_RESPONSE_SMART_DATA) {
        fe_log_print("Telemetry: Did not receive valid SMART response from BE.");
        return -1;
    }

    memcpy(health, resp_msg.payload, sizeof(smart_health_info_t));
    fe_log_print("Telemetry: Received SMART data from BE. Temp=%dC", health->temperature_celsius);
    return 0;
}