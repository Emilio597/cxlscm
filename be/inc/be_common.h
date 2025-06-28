
/*
 * ============================================================================
 * 文件: be/inc/be_common.h
 * ============================================================================
 */
#ifndef BE_COMMON_H
#define BE_COMMON_H
#include "fe_common.h"
#include "firmware_common.h"
#include "media_health.h"
#include "tx_api.h"

// BE 控制块
typedef struct {
    status_t state;
    uint32_t wear_leveling_runs;
    uint32_t patrol_runs;
    uint64_t uncorrectable_errors;
    uint32_t spare_blocks_available;
} be_control_t;

extern be_control_t g_be_ctrl;

void be_log_print(const char* format, ...);
void be_main_thread_entry(ULONG thread_input);
void be_process_fe_message(fe_be_message_t *msg);
void be_periodic_tasks(void);
#endif /* BE_COMMON_H */
