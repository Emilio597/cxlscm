
/*
 * ============================================================================
 * 文件: fe/inc/mailbox.h
 * 描述: 邮箱命令处理模块的接口。
 * ============================================================================
 */
#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>
/* 邮箱命令码定义 */
#define MBOX_CMD_GET_LSA                  0x0001 // 获取LSA
#define MBOX_CMD_SET_TIMESTAMP            0x0002 // 设置时间戳

#define MBOX_CMD_GET_TOPOLOGY             0x1001 // 新增: 获取设备物理拓扑
#define MBOX_CMD_GET_SMART_HEALTH         0x1002 // 新增: 获取SMART健康信息

#define MBOX_CMD_MANAGE_BAD_BLOCK         0x2001 // 新增: 管理坏块

#define MBOX_CMD_FW_DOWNLOAD              0x3001 // 新增: 固件下载
#define MBOX_CMD_FW_ACTIVATE              0x3002 // 新增: 固件激活

#define MBOX_CMD_UNSUPPORTED_TEST         0x4500 // 日志中出现的未知命令

// 邮箱命令处理入口
void mbox_command_handler(uint16_t command_code, void* payload, uint32_t payload_size);
void mbox_init(void);

#endif /* MAILBOX_H */
