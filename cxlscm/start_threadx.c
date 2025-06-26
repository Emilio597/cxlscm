#include "tx_api.h"  
#include "fe_common.h"  
#include "be_common.h"  
#include "hw_registers.h"  
#include "sensor_driver.h"  
#include "pcm_controller.h"  
  
/* 定义线程堆栈大小 */  
#define FE_THREAD_STACK_SIZE    4096  
#define BE_THREAD_STACK_SIZE    4096  
#define DEMO_BYTE_POOL_SIZE     (64 * 1024)  
  
/* 线程控制块 */  
TX_THREAD fe_main_thread;  
TX_THREAD be_main_thread;  
  
/* 内存池 */  
TX_BYTE_POOL byte_pool_0;  
  
/* FE和BE之间的消息队列 */  
TX_QUEUE fe_be_queue;  
TX_QUEUE be_fe_queue;  
  
/* 消息队列缓冲区 */  
#define QUEUE_SIZE 16  
ULONG fe_be_queue_buffer[QUEUE_SIZE * (sizeof(fe_be_message_t) / sizeof(ULONG))];  
ULONG be_fe_queue_buffer[QUEUE_SIZE * (sizeof(fe_be_message_t) / sizeof(ULONG))];  
  
/* 事件标志组 */  
TX_EVENT_FLAGS_GROUP system_events;  
  
/* 定义主入口点 */  
int main(void)  
{  
    /* 硬件初始化 */  
    // 这里可以添加必要的硬件初始化代码  
      
    /* 进入ThreadX内核 */  
    tx_kernel_enter();  
      
    return 0;  
}  
  
/* 定义初始系统外观 */  
void tx_application_define(void *first_unused_memory)  
{  
    CHAR *pointer = TX_NULL;  
    UINT status;  
  
    /* 创建字节内存池 */  
    status = tx_byte_pool_create(&byte_pool_0, "byte pool 0",   
                                first_unused_memory, DEMO_BYTE_POOL_SIZE);  
    if (status != TX_SUCCESS) {  
        /* 错误处理 */  
        return;  
    }  
  
    /* 创建系统事件标志组 */  
    status = tx_event_flags_create(&system_events, "System Events");  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 创建FE到BE的消息队列 */  
    status = tx_queue_create(&fe_be_queue, "FE to BE Queue",  
                            sizeof(fe_be_message_t) / sizeof(ULONG),  
                            fe_be_queue_buffer, sizeof(fe_be_queue_buffer));  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 创建BE到FE的消息队列 */  
    status = tx_queue_create(&be_fe_queue, "BE to FE Queue",  
                            sizeof(fe_be_message_t) / sizeof(ULONG),  
                            be_fe_queue_buffer, sizeof(be_fe_queue_buffer));  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 为FE主线程分配堆栈 */  
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer,   
                             FE_THREAD_STACK_SIZE, TX_NO_WAIT);  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 创建FE主线程 */  
    status = tx_thread_create(&fe_main_thread, "FE Main Thread",  
                             fe_main_thread_entry, 0,  
                             pointer, FE_THREAD_STACK_SIZE,  
                             1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 为BE主线程分配堆栈 */  
    status = tx_byte_allocate(&byte_pool_0, (VOID **) &pointer,   
                             BE_THREAD_STACK_SIZE, TX_NO_WAIT);  
    if (status != TX_SUCCESS) {  
        return;  
    }  
  
    /* 创建BE主线程 */  
    status = tx_thread_create(&be_main_thread, "BE Main Thread",  
                             be_main_thread_entry, 0,  
                             pointer, BE_THREAD_STACK_SIZE,  
                             2, 2, TX_NO_TIME_SLICE, TX_AUTO_START);  
    if (status != TX_SUCCESS) {  
        return;  
    }  
}  
  
/* FE主线程入口函数 */  
void fe_main_thread_entry(ULONG thread_input)  
{  
    fe_status_t status;  
      
    fe_log_print("FE Main Thread Started on ARC HS Core.");  
      
    /* 初始化硬件寄存器访问 */  
    // 这里可以添加FE特定的硬件初始化  
      
    /* 初始化传感器驱动 */  
    status = sensor_init();  
    if (status != FE_STATUS_SUCCESS) {  
        fe_log_print("FE: Failed to initialize sensors");  
        return;  
    }  
      
    /* 初始化UART */  
    status = uart_init(115200);  
    if (status != FE_STATUS_SUCCESS) {  
        fe_log_print("FE: Failed to initialize UART");  
        return;  
    }  
      
    /* 初始化GPIO */  
    status = gpio_init();  
    if (status != FE_STATUS_SUCCESS) {  
        fe_log_print("FE: Failed to initialize GPIO");  
        return;  
    }  
      
    /* 等待BE初始化完成 */  
    ULONG actual_flags;  
    tx_event_flags_get(&system_events, 0x01, TX_OR_CLEAR,   
                       &actual_flags, TX_WAIT_FOREVER);  
      
    fe_log_print("FE: BE initialization completed, starting main loop");  
      
    /* FE主循环 */  
    while (1) {  
        fe_be_message_t msg;  
        UINT queue_status;  
          
        /* 检查来自BE的消息 */  
        queue_status = tx_queue_receive(&be_fe_queue, &msg, 100); // 100ms超时  
        if (queue_status == TX_SUCCESS) {  
            /* 处理来自BE的消息 */  
            fe_process_be_message(&msg);  
        }  
          
        /* 执行FE的周期性任务 */  
        fe_periodic_tasks();  
          
        /* 线程休眠一段时间 */  
        tx_thread_sleep(10); // 100ms  
    }  
}  
  
/* BE主线程入口函数 */  
void be_main_thread_entry(ULONG thread_input)  
{  
    be_status_t status;  
      
    be_log_print("BE Main Thread Started.");  
      
    /* 初始化PCM控制器 */  
    status = pcm_controller_init();  
    if (status != BE_STATUS_SUCCESS) {  
        be_log_print("BE: Failed to initialize PCM controller");  
        return;  
    }  
      
    /* 初始化介质管理器 */  
    status = media_manager_init();  
    if (status != BE_STATUS_SUCCESS) {  
        be_log_print("BE: Failed to initialize media manager");  
        pcm_controller_deinit();  
        return;  
    }  
      
    /* 初始化介质健康管理 */  
    status = media_health_init();  
    if (status != BE_STATUS_SUCCESS) {  
        be_log_print("BE: Failed to initialize media health management");  
        return;  
    }  
      
    /* 通知FE初始化完成 */  
    tx_event_flags_set(&system_events, 0x01, TX_OR);  
      
    /* 向FE发送启动完成消息 */  
    fe_be_message_t msg;  
    msg.event_id = BE_EVENT_POWER_UP_DONE;  
    tx_queue_send(&be_fe_queue, &msg, TX_WAIT_FOREVER);  
      
    be_log_print("BE: Initialization completed, starting main loop");  
      
    /* BE主循环 */  
    while (1) {  
        fe_be_message_t msg;  
        UINT queue_status;  
          
        /* 检查来自FE的消息 */  
        queue_status = tx_queue_receive(&fe_be_queue, &msg, 100); // 100ms超时  
        if (queue_status == TX_SUCCESS) {  
            /* 处理来自FE的消息 */  
            be_process_fe_message(&msg);  
        }  
          
        /* 执行BE的周期性任务 */  
        be_periodic_tasks();  
          
        /* 线程休眠一段时间 */  
        tx_thread_sleep(10); // 100ms  
    }  
}