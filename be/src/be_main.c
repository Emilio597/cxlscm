#include "be_common.h"
#include "media_manager.h"
#include "bbm.h"
#include "address_translation.h"
#include "media_health.h"
#include "pcm_controller.h"
#include "wear_leveling.h"
#include "telemetry.h" 
#include <string.h>

be_control_t g_be_ctrl;
extern TX_QUEUE be_fe_queue;  

void be_log_print(const char* format, ...) {
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

void be_main_thread_entry(ULONG thread_input) {
    be_log_print("BE Main Thread Started on Core %d.", (int)thread_input);
    g_be_ctrl.state = STATE_INIT;
    media_manager_init(); bbm_init(); at_init();

    g_be_ctrl.state = STATE_FORMATTING;
    be_log_print("Blank Drive Format ..."); tx_thread_sleep(10); be_log_print("Blank Drive Format done");

    g_be_ctrl.state = STATE_RESTORING;
    be_log_print("fc init, swBwMb=5120(MB). srBwMb=17066(MB). pl submit gap=2355(us).");
    be_log_print("copy wd tbl from Nor"); be_log_print("wl cnt average recovered, 0.");
    be_log_print("BE power up restore done");
    
    fe_be_message_t msg; msg.event_id = BE_EVENT_POWER_UP_DONE;
    tx_queue_send(&fe_be_queue, &msg, TX_WAIT_FOREVER);
    
    g_be_ctrl.state = STATE_RUNNING_BG_TASKS;
    TX_THREAD patrol_thread; UCHAR patrol_stack[2048];
    tx_thread_create(&patrol_thread, "Media Patrol Task", media_patrol_task, 0, patrol_stack, 2048, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    TX_THREAD wl_thread; UCHAR wl_stack[2048];
    tx_thread_create(&wl_thread, "Wear Leveling Task", wear_leveling_task, 0, wl_stack, 2048, 12, 12, TX_NO_TIME_SLICE, TX_AUTO_START);
    be_log_print("Background tasks (Patrol, WL) started.");
    
    status_t status;  
    // 初始化PCM控制器  
    status = pcm_controller_init();  
    if (status != STATUS_SUCCESS) {  
        // 错误处理  
        return;  
    }  
    // 初始化其他BE组件  
    status = media_manager_init();  
    if (status != STATUS_SUCCESS) {  
        pcm_controller_deinit();  
        return;  
    }
    // 通知FE初始化完成  
    msg.event_id = BE_EVENT_POWER_UP_DONE;  
    tx_queue_send(&fe_be_queue, &msg, TX_WAIT_FOREVER); 

    // BE主线程循环，处理来自FE的消息
    while(1) {
        fe_be_message_t received_msg;
        UINT status = tx_queue_receive(&fe_be_queue, &received_msg, 5000); // 每5秒检查一次消息或超时
        if (status == TX_SUCCESS) {
            switch(received_msg.event_id) {
                case FE_CMD_GET_SMART_DATA: {
                    be_log_print("BE: Received SMART data request from FE.");
                    smart_health_info_t current_health;
                    current_health.temperature_celsius = 42 + (rand() % 5);
                    current_health.power_on_hours = g_fe_ctrl.current_time.hour - 8; // simplified
                    current_health.wear_level_percentage = 98; // a more complex calculation needed in reality
                    current_health.spare_blocks_percentage = (g_be_ctrl.spare_blocks_available * 100) / 100; // SPARE_BLOCK_POOL_SIZE
                    current_health.uncorrectable_errors = g_be_ctrl.uncorrectable_errors;
                    
                    fe_be_message_t resp_msg;
                    resp_msg.event_id = BE_RESPONSE_SMART_DATA;
                    memcpy(resp_msg.payload, &current_health, sizeof(current_health));
                    tx_queue_send(&fe_be_queue, &resp_msg, TX_WAIT_FOREVER);
                    break;
                }
                default:
                    be_log_print("BE: Received unknown command from FE: 0x%X", received_msg.event_id);
                    break;
            }
        } else {
            be_log_print("BE main loop: No message from FE. BG tasks running.");
        }
    }
}

// 处理来自FE的消息  
void be_process_fe_message(fe_be_message_t *msg) {  
    if (!msg) return;  
      
    switch(msg->event_id) {  
        case FE_CMD_GET_SMART_DATA: {  
            be_log_print("BE: Received SMART data request from FE.");  
              
            // 收集介质健康数据  
            media_health_stats_t media_stats;  
            smart_health_info_t health_info = {0};  
            

            // 发送响应  
            fe_be_message_t resp_msg;  
            resp_msg.event_id = BE_RESPONSE_SMART_DATA;  
            memcpy(resp_msg.payload, &health_info, sizeof(health_info));  
            tx_queue_send(&be_fe_queue, &resp_msg, TX_WAIT_FOREVER);  
            break;  
        }  
        case FE_CMD_READ_DATA: {  
            // 处理读取请求  
            be_log_print("BE: Processing read request from FE");  
            // 实现读取逻辑  
            break;  
        }  
        case FE_CMD_WRITE_DATA: {  
            // 处理写入请求  
            be_log_print("BE: Processing write request from FE");  
            // 实现写入逻辑  
            break;  
        }  
        default:  
            be_log_print("BE: Received unknown command from FE: 0x%X", msg->event_id);  
            break;  
    }  
}  
  
// BE周期性任务  
void be_periodic_tasks(void) {  
    static uint32_t task_counter = 0;  
    task_counter++;  
      
    // 每10次循环执行一次健康检查  
    if (task_counter % 10 == 0) {  
        // 更新介质健康统计  
        media_health_stats_t stats;  
        if (media_health_get_stats(&stats) == STATUS_SUCCESS) {  
            if (stats.health_level <= MEDIA_HEALTH_POOR) {  
                be_log_print("BE: Media health warning - level %d", stats.health_level);  
            }  
        }  
    }  
      
    // 每50次循环检查一次磨损均衡状态  
    if (task_counter % 50 == 0) {  
        wl_statistics_t wl_stats;  
        if (wl_get_statistics(&wl_stats) == STATUS_SUCCESS) {  
            uint32_t wear_diff = wl_stats.max_wear_count - wl_stats.min_wear_count;  
            if (wear_diff > 1000) {  
                be_log_print("BE: High wear difference detected: %d", wear_diff);  
            }  
        }  
    }  
      
    // 每100次循环输出状态信息  
    if (task_counter % 100 == 0) {  
        be_log_print("BE: Periodic task cycle %d completed", task_counter);  
    }  
}