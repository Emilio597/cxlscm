/*
 * ============================================================================
 * 文件: fe/src/fe_main.c
 * 描述: FE固件主入口，实现ThreadX任务和状态机调度。
 * ============================================================================
 */
#include "fe_common.h"
#include "hw_access.h"
#include "mailbox.h"

/* 全局变量定义 */
fe_control_t g_fe_ctrl;
TX_THREAD fe_main_thread;
TX_QUEUE be_to_fe_queue;
TX_BYTE_POOL fe_byte_pool;

/* 线程栈和内存池定义 */
#define FE_THREAD_STACK_SIZE    4096
#define BYTE_POOL_SIZE          16384
#define QUEUE_MESSAGE_SIZE      (sizeof(be_fe_message_t) / sizeof(ULONG))
#define QUEUE_LENGTH            16

static UCHAR fe_thread_stack[FE_THREAD_STACK_SIZE];
static UCHAR fe_byte_pool_memory[BYTE_POOL_SIZE];
static ULONG be_to_fe_queue_storage[QUEUE_LENGTH * QUEUE_MESSAGE_SIZE];

/* FE主线程入口函数 - 实现核心状态机 */
void fe_main_thread_entry(ULONG thread_input)
{
    fe_log_print("FE Main Thread Started on ARC HS Core.");

    while (1) {
        // 根据当前状态调用对应的处理函数
        switch (g_fe_ctrl.state) {
            case FE_STATE_RESET:
                fe_handle_state_reset();
                break;
            case FE_STATE_WAITING_FOR_BE:
                fe_handle_state_waiting_for_be();
                break;
            case FE_STATE_POWER_UP_INIT:
                fe_handle_state_power_up_init();
                break;
            case FE_STATE_MEMORY_TRAINING:
                fe_handle_state_memory_training();
                break;
            case FE_STATE_CXL_CONFIG:
                fe_handle_state_cxl_config();
                break;
            case FE_STATE_WAITING_FOR_HOST:
                fe_handle_state_waiting_for_host();
                break;
            case FE_STATE_HOST_HANDSHAKE:
                fe_handle_state_host_handshake();
                break;
            case FE_STATE_NORMAL_OPERATION:
                fe_handle_state_normal_operation();
                break;
            case FE_STATE_ERROR:
                // 进入错误状态，可以执行循环等待或重启策略
                fe_log_print("--- SYSTEM HALTED IN ERROR STATE ---");
                tx_thread_sleep(1000);
                break;
            default:
                g_fe_ctrl.state = FE_STATE_ERROR;
                break;
        }

        fe_timestamp_update(); 
        tx_thread_sleep(10); // 短暂休眠，防止CPU占用过高
    }
}

/* 状态处理函数实现 */
void fe_handle_state_reset(void) {
    // 固件初始化
    memset(&g_fe_ctrl, 0, sizeof(fe_control_t));
    g_fe_ctrl.state = FE_STATE_RESET;
    g_fe_ctrl.current_time.year = 1970;
    g_fe_ctrl.current_time.month = 1;
    g_fe_ctrl.current_time.day = 1;
    g_fe_ctrl.current_time.hour = 8;

    fe_log_init(); // 初始化日志互斥锁
    hw_init_registers(); // 初始化寄存器
    mbox_init(); // 初始化邮箱

    // 转换到下一状态
    g_fe_ctrl.state = FE_STATE_WAITING_FOR_BE;
}

void fe_handle_state_waiting_for_be(void) {
    be_fe_message_t msg;
    UINT status = tx_queue_receive(&be_to_fe_queue, &msg, 100); // 等待100 ticks
    if (status == TX_SUCCESS && msg.event_id == BE_EVENT_POWER_UP_DONE) {
        fe_log_print("Recvd the Power up done from BE");
        g_fe_ctrl.be_power_up_done = 1;
        g_fe_ctrl.state = FE_STATE_POWER_UP_INIT; // 收到消息，进入下一状态
    }
}

void fe_handle_state_power_up_init(void) {
    fe_power_up_sequence();
    g_fe_ctrl.state = FE_STATE_MEMORY_TRAINING;
}

void fe_handle_state_memory_training(void) {
    fe_log_print("Starting Memory Training...");
    tx_thread_sleep(500); // 内存训练耗时
    fe_log_print("Memory Training Completed.");
    g_fe_ctrl.state = FE_STATE_CXL_CONFIG;
}

void fe_handle_state_cxl_config(void) {
    fe_cxl_initialization_sequence();
    g_fe_ctrl.state = FE_STATE_WAITING_FOR_HOST;
}

void fe_handle_state_waiting_for_host(void) {
    fe_log_print("CXL waiting for host to Enable CXL.mem...");
    // 实际应由中断触发，此处轮询等待
    for (int i = 0; i < 200; ++i) { // 等待200 * 100ms = 20s
        if (g_fe_ctrl.host_cxl_enabled) break;
        tx_thread_sleep(100);
    }
    
    // 主机在一段时间后使能了CXL.mem
    g_fe_ctrl.host_cxl_enabled = 1; 
    fe_log_print("Good News, Host has Enable CXL.mem");
    
    g_fe_ctrl.state = FE_STATE_HOST_HANDSHAKE;
}

void fe_handle_state_host_handshake(void) {
    fe_host_handshake_sequence();
    g_fe_ctrl.state = FE_STATE_NORMAL_OPERATION;
}

void fe_handle_state_normal_operation(void) {
    fe_media_status_monitor_task();
    // 正常工作时，可以接收和处理来自主机的邮箱命令或其他事件
    // 此处仅做周期性状态检查
    tx_thread_sleep(100);
}

void fe_handle_state_fw_update(void) {
    fe_log_print("Now in Firmware Update mode. Waiting for activation to complete...");
    // 固件更新过程，这通常会涉及重启
    fe_log_print("Firmware update complete. Rebooting system...");
    // 实际应触发硬件重启
    tx_thread_sleep(1000);
    g_fe_ctrl.state = FE_STATE_RESET; // 重启后回到Reset状态
}


/* ThreadX 应用程序定义 */
void tx_application_define(void *first_unused_memory) {
    UINT status;

    // 创建字节池
    status = tx_byte_pool_create(&fe_byte_pool, "FE Byte Pool", fe_byte_pool_memory, BYTE_POOL_SIZE);
    if (status != TX_SUCCESS) return;

    // 创建BE到FE的消息队列
    status = tx_queue_create(&be_to_fe_queue, "BE to FE Queue", QUEUE_MESSAGE_SIZE,
                             be_to_fe_queue_storage, QUEUE_LENGTH * sizeof(be_fe_message_t));
    if (status != TX_SUCCESS) return;

    // 创建FE主线程
    status = tx_thread_create(&fe_main_thread, "FE Main Thread", fe_main_thread_entry, 0,
                              fe_thread_stack, FE_THREAD_STACK_SIZE,
                              5, 5, TX_NO_TIME_SLICE, TX_AUTO_START); // 优先级设为5
    if (status != TX_SUCCESS) return;
    
    // (BE) 在启动后发送一条消息给FE
    // 在真实场景中，这将由另一个处理器上的BE固件完成
    TX_THREAD be_simulator_thread;
    UCHAR be_sim_stack[1024];
    tx_thread_create(&be_simulator_thread, "BE Simulator", 
        [](ULONG) {
            tx_thread_sleep(200); 
            printf("BE: Power up sequence started...\n");
            printf("BE: Blank Drive Format ...\n");
            tx_thread_sleep(100);
            printf("BE: BE power up restore done\n");
            
            be_fe_message_t msg;
            msg.event_id = BE_EVENT_POWER_UP_DONE;
            tx_queue_send(&be_to_fe_queue, &msg, TX_WAIT_FOREVER);
        }, 
        0, be_sim_stack, 1024, 6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);
}

/* 日志和时间戳实现 */
void fe_log_init(void) {
    tx_mutex_create(&g_fe_ctrl.log_mutex, "Log Mutex", TX_NO_INHERIT);
}

void fe_log_print(const char* format, ...) {
    char buffer[256];
    va_list args;

    tx_mutex_get(&g_fe_ctrl.log_mutex, TX_WAIT_FOREVER);
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("FE:%04d-%02d-%02d %02d:%02d:%02d %s\n",
           g_fe_ctrl.current_time.year, g_fe_ctrl.current_time.month, g_fe_ctrl.current_time.day,
           g_fe_ctrl.current_time.hour, g_fe_ctrl.current_time.minute, g_fe_ctrl.current_time.second,
           buffer);
           
    tx_mutex_put(&g_fe_ctrl.log_mutex);
}

void fe_timestamp_update(void) {
    // 仅为，简单递增秒
    g_fe_ctrl.current_time.second++;
    if (g_fe_ctrl.current_time.second >= 60) {
        g_fe_ctrl.current_time.second = 0;
        g_fe_ctrl.current_time.minute++;
        if (g_fe_ctrl.current_time.minute >= 60) {
            g_fe_ctrl.current_time.minute = 0;
            g_fe_ctrl.current_time.hour++;
        }
    }
}

