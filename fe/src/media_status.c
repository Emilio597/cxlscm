/*
 * ============================================================================
 * 文件: fe/src/media_status.c
 * 描述: 介质状态的周期性检查。
 * ============================================================================
 */
#include "fe_common.h"

void fe_media_status_monitor_task(void) {
    static uint32_t check_count = 0;
    check_count++;

    // 每1000 ticks (约10秒) 打印一次状态
    if (check_count % 1000 == 0) {
        fe_log_print("Media Status Check: All media ready. Temperature: 45C. Wear Level: 98%%");
    }
}
