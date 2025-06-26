/*
 * ============================================================================
 * 新增文件: fe/inc/fw_update.h
 * 描述: 固件更新模块的接口。
 * ============================================================================
 */
#ifndef FW_UPDATE_H
#define FW_UPDATE_H

#include <stdint.h>

#define FW_CHUNK_SIZE 4096

// 固件数据块结构
typedef struct {
    uint32_t sequence_number;
    uint32_t size;
    uint8_t data[FW_CHUNK_SIZE];
    uint32_t checksum;
} fw_chunk_t;


void fw_update_init(void);
// 返回0表示成功
int fw_update_download_chunk(const fw_chunk_t *chunk);
// 返回0表示成功
int fw_update_activate(void);

#endif /* FW_UPDATE_H */
