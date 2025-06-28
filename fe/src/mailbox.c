/*
 * ============================================================================
 * 文件: fe/src/mailbox.c
 * 描述: 实现邮箱命令的具体处理逻辑。
 * ============================================================================
 */
#include "fe_common.h"
#include "mailbox.h"
#include "cxl_defs.h"
#include "telemetry.h"
#include "fw_update.h"

void mbox_init(void) {
    fe_log_print("mbox_poison_list_init, max records: %d", MAX_POISON_RECORDS);
    fe_log_print("mbox_poison_list_mem_init");
}

void mbox_command_handler(uint16_t command_code, void* payload, uint32_t payload_size) {
    switch (command_code) {
        case MBOX_CMD_GET_TOPOLOGY: telemetry_get_topology((device_topology_t*)payload); break;
        case MBOX_CMD_GET_SMART_HEALTH:
            if (telemetry_get_smart_health((smart_health_info_t*)payload) != 0) {
                fe_log_print("Mailbox: Failed to get SMART health data from BE.");
            }
            break;
        case MBOX_CMD_FW_DOWNLOAD: fw_update_download_chunk((const fw_chunk_t*)payload); break;
        case MBOX_CMD_FW_ACTIVATE: fw_update_activate(); break;
        default: fe_log_print("Mailbox: Unknown Cmd: 0x%04X", command_code); break;
    }
}

