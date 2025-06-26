#include "be_common.h"
#include "media_manager.h"
#include "bbm.h"
#include "address_translation.h"
#include "telemetry.h" // For smart_health_info_t

be_control_t g_be_ctrl;

void be_main_thread_entry(ULONG thread_input) {
    be_log_print("BE Main Thread Started on Core %d.", (int)thread_input);
    g_be_ctrl.state = BE_STATE_INIT;
    media_manager_init(); bbm_init(); at_init();

    g_be_ctrl.state = BE_STATE_FORMATTING;
    be_log_print("Blank Drive Format ..."); tx_thread_sleep(10); be_log_print("Blank Drive Format done");

    g_be_ctrl.state = BE_STATE_RESTORING;
    be_log_print("fc init, swBwMb=5120(MB). srBwMb=17066(MB). pl submit gap=2355(us).");
    be_log_print("copy wd tbl from Nor"); be_log_print("wl cnt average recovered, 0.");
    be_log_print("BE power up restore done");
    
    fe_be_message_t msg; msg.event_id = BE_EVENT_POWER_UP_DONE;
    tx_queue_send(&fe_be_queue, &msg, TX_WAIT_FOREVER);
    
    g_be_ctrl.state = BE_STATE_RUNNING_BG_TASKS;
    TX_THREAD patrol_thread; UCHAR patrol_stack[2048];
    tx_thread_create(&patrol_thread, "Media Patrol Task", media_patrol_task, 0, patrol_stack, 2048, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    TX_THREAD wl_thread; UCHAR wl_stack[2048];
    tx_thread_create(&wl_thread, "Wear Leveling Task", wear_leveling_task, 0, wl_stack, 2048, 12, 12, TX_NO_TIME_SLICE, TX_AUTO_START);
    be_log_print("Background tasks (Patrol, WL) started.");
    
    be_status_t status;  
    // 初始化PCM控制器  
    status = pcm_controller_init();  
    if (status != BE_STATUS_SUCCESS) {  
        // 错误处理  
        return;  
    }  
    // 初始化其他BE组件  
    status = media_manager_init();  
    if (status != BE_STATUS_SUCCESS) {  
        pcm_controller_deinit();  
        return;  
    }
    // 通知FE初始化完成  
    fe_be_message_t msg;  
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
