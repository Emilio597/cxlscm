
/*
 * ============================================================================
 * 文件: be/inc/be_common.h
 * ============================================================================
 */
#ifndef BE_COMMON_H
#define BE_COMMON_H
#include "fe_common.h"
typedef enum { BE_STATE_INIT, BE_STATE_FORMATTING, BE_STATE_RESTORING, BE_STATE_RUNNING_BG_TASKS, BE_STATE_HALTED } be_state_t;
typedef struct { be_state_t state; uint32_t wear_leveling_runs; uint32_t patrol_runs; uint64_t uncorrectable_errors; uint32_t spare_blocks_available; } be_control_t;
extern be_control_t g_be_ctrl;
#endif /* BE_COMMON_H */
