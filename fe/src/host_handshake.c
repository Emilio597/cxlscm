
/*
 * ============================================================================
 * 文件: fe/src/host_handshake.c
 * 描述: 实现与主机握手、事件处理和邮箱命令响应的逻辑。
 * ============================================================================
 */
#include "fe_common.h"
#include "cxl_defs.h"
#include "mailbox.h"
#include "telemetry.h"
#include "bbm.h"
#include "fw_update.h"

static void init_event_handling(void) {
    fe_log_print("None Valid Event, Start Init");
    mbox_init();
    uint32_t hw_event_bmask = 0x627;
    uint32_t fw_event_bmask = 0xffffffc0;
    fe_log_print("core_id:0 enter hw_event_bmask:0x%x fw_event_bmask:0x%x", hw_event_bmask, fw_event_bmask);
    uint32_t be_hw_bmask = 0xfffff9d8;
    uint32_t be_fw_bmask = 0xfffffc07;
    fe_log_print("BE core_id:1 enter hw_event_bmask:0x%x fw_event_bmask:0x%x", be_hw_bmask, be_fw_bmask);
}

void fe_host_handshake_sequence(void) {
    init_event_handling();
    
    fe_log_print("Enter Normal Flow config: meta override");
    
    event_intr_mask_t intr_mask = {0x0, 0x0, 0x0, 0x30};
    fe_log_print("Set Event Intr: Info:0x%x, Warn:0x%x, Fail:0x%x, Fatal:0x%x", intr_mask.info, intr_mask.warn, intr_mask.fail, intr_mask.fatal);
    fe_log_print("Get Event Intr: Info: 0x%x, Warn: 0x%x, Fail: 0x%x, Fatal: 0x%x", intr_mask.info, intr_mask.warn, intr_mask.fail, intr_mask.fatal);

    tx_thread_sleep(1500);
    fe_log_print("RMW_SRAM ecc fail");
    fe_log_print("CXL Event:Vendor SRAM Error, Severity:Fatal, TimeStamp:0x3a_36067500");

    tx_thread_sleep(1000);
    g_fe_ctrl.current_time.year = 2025; g_fe_ctrl.current_time.month = 6; g_fe_ctrl.current_time.day = 26; g_fe_ctrl.current_time.hour = 18; g_fe_ctrl.current_time.minute = 51; g_fe_ctrl.current_time.second = 27;
    fe_log_print("set timestamp 2025-6-26 18:51:27");
    
    tx_thread_sleep(10); mbox_command_handler(MBOX_CMD_UNSUPPORTED_TEST, NULL, 0);
    tx_thread_sleep(10); mbox_command_handler(MBOX_CMD_UNSUPPORTED_TEST, NULL, 0);

    // --- 新增功能：主机在握手后开始查询设备信息和执行管理任务 ---
    fe_log_print("--- Host starts advanced management tasks ---");

    // 1. 主机查询设备拓扑
    tx_thread_sleep(500);
    fe_log_print("Host is querying device topology...");
    device_topology_t topo;
    mbox_command_handler(MBOX_CMD_GET_TOPOLOGY, &topo, sizeof(topo));

    // 2. 主机查询SMART健康信息
    tx_thread_sleep(500);
    fe_log_print("Host is querying SMART health data...");
    smart_health_info_t health;
    mbox_command_handler(MBOX_CMD_GET_SMART_HEALTH, &health, sizeof(health));

    // 3. 主机发起固件更新流程
    tx_thread_sleep(1000);
    fe_log_print("Host is initiating firmware update...");
    fw_chunk_t chunk = { .sequence_number = 1, .size = 1024, .checksum = 0xABCD };
    snprintf((char*)chunk.data, 32, "This is the first FW chunk.");
    mbox_command_handler(MBOX_CMD_FW_DOWNLOAD, &chunk, sizeof(chunk));
    
    chunk.sequence_number = 2;
    chunk.checksum = 0xBCDE;
    snprintf((char*)chunk.data, 32, "This is the second FW chunk.");
    mbox_command_handler(MBOX_CMD_FW_DOWNLOAD, &chunk, sizeof(chunk));

    tx_thread_sleep(500);
    fe_log_print("Host requests to activate the new firmware.");
    mbox_command_handler(MBOX_CMD_FW_ACTIVATE, NULL, 0);
}

