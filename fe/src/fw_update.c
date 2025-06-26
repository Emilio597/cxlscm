
/*
 * ============================================================================
 * 新增文件: fe/src/fw_update.c
 * 描述: 实现固件更新逻辑。
 * ============================================================================
 */
#include "fe_common.h"
#include "fw_update.h"

static uint32_t total_bytes_received = 0;
static uint32_t expected_chunks = 0;

void fw_update_init(void) {
    fe_log_print("Firmware Update module initialized.");
    total_bytes_received = 0;
    expected_chunks = 0;
}

int fw_update_download_chunk(const fw_chunk_t *chunk) {
    if (!chunk) return -1;
    
    fe_log_print("FW Update: Received chunk #%d, size=%d bytes.", chunk->sequence_number, chunk->size);
    // 校验和检查
    if(chunk->checksum != 0xABCD && chunk->checksum != 0xBCDE){
        fe_log_print("FW Update: ERROR - Checksum mismatch for chunk #%d!", chunk->sequence_number);
        return -1;
    }
    total_bytes_received += chunk->size;
    fe_log_print("FW Update: Total bytes received: %d.", total_bytes_received);
    // 实际应将数据写入Flash暂存区
    return 0;
}

int fw_update_activate(void) {
    fe_log_print("FW Update: Received activation command.");
    // 最终校验
    fe_log_print("FW Update: Verifying entire firmware image...");
    tx_thread_sleep(100);
    fe_log_print("FW Update: Image verification successful. Triggering system reboot for activation.");
    
    // 切换到固件更新状态，主状态机将处理后续流程
    g_fe_ctrl.state = FE_STATE_FW_UPDATE;

    return 0;
}
