#include "media_health.h"  
  
static media_health_stats_t g_media_stats = {0};  
static bool health_initialized = false;  
  
// 介质健康管理初始化  
status_t media_health_init(void) {  
    memset(&g_media_stats, 0, sizeof(g_media_stats));  
    g_media_stats.spare_block_count = 100;  // 初始备用块数量  
    g_media_stats.health_level = MEDIA_HEALTH_EXCELLENT;  
    g_media_stats.remaining_life_percent = 100;  
    health_initialized = true;  
      
    be_log_print("Media health management initialized.");  
    return STATUS_SUCCESS;  
}  
  
// 更新读取统计  
status_t media_health_update_read_stats(bool success, uint32_t retry_count) {  
    if (!health_initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    g_media_stats.total_read_count++;  
    if (retry_count > 0) {  
        g_media_stats.read_retry_count += retry_count;  
    }  
      
    return STATUS_SUCCESS;  
}  
  
// 更新写入统计  
status_t media_health_update_write_stats(bool success, uint32_t retry_count) {  
    if (!health_initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    g_media_stats.total_write_count++;  
    if (retry_count > 0) {  
        g_media_stats.write_retry_count += retry_count;  
    }  
      
    return STATUS_SUCCESS;  
}  
  
// 更新ECC统计  
status_t media_health_update_ecc_stats(uint32_t correctable, uint32_t uncorrectable) {  
    if (!health_initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    g_media_stats.ecc_correctable_errors += correctable;  
    g_media_stats.ecc_uncorrectable_errors += uncorrectable;  
      
    // 更新健康等级  
    g_media_stats.health_level = media_health_calculate_level();  
      
    return STATUS_SUCCESS;  
}  
  
// 更新磨损统计  
status_t media_health_update_wear_stats(uint32_t block_index, uint32_t wear_count) {  
    if (!health_initialized) {  
        return STATUS_NOT_INITIALIZED;  
    }  
      
    // 更新最大磨损次数  
    if (wear_count > g_media_stats.wear_level_max) {  
        g_media_stats.wear_level_max = wear_count;  
    }  
      
    // 简化的平均磨损计算  
    g_media_stats.wear_level_avg = (g_media_stats.wear_level_avg + wear_count) / 2;  
      
    // 计算剩余寿命  
    uint32_t max_endurance = 1000000;  // 假设最大耐久性为100万次  
    if (g_media_stats.wear_level_max < max_endurance) {  
        g_media_stats.remaining_life_percent =   
            ((max_endurance - g_media_stats.wear_level_max) * 100) / max_endurance;  
    } else {  
        g_media_stats.remaining_life_percent = 0;  
    }  
      
    return STATUS_SUCCESS;  
}

// 计算健康等级  
media_health_level_t media_health_calculate_level(void) {  
    uint32_t score = 100;  // 满分100  
      
    // 根据重试率扣分  
    if (g_media_stats.total_read_count > 0) {  
        uint32_t read_retry_rate = (g_media_stats.read_retry_count * 100) / g_media_stats.total_read_count;  
        if (read_retry_rate > 10) score -= 20;  
        else if (read_retry_rate > 5) score -= 10;  
    }  
      
    if (g_media_stats.total_write_count > 0) {  
        uint32_t write_retry_rate = (g_media_stats.write_retry_count * 100) / g_media_stats.total_write_count;  
        if (write_retry_rate > 10) score -= 20;  
        else if (write_retry_rate > 5) score -= 10;  
    }  
      
    // 根据ECC错误扣分  
    if (g_media_stats.ecc_uncorrectable_errors > 100) score -= 30;  
    else if (g_media_stats.ecc_uncorrectable_errors > 50) score -= 20;  
    else if (g_media_stats.ecc_uncorrectable_errors > 10) score -= 10;  
      
    if (g_media_stats.ecc_correctable_errors > 10000) score -= 15;  
    else if (g_media_stats.ecc_correctable_errors > 5000) score -= 10;  
    else if (g_media_stats.ecc_correctable_errors > 1000) score -= 5;  
      
    // 根据坏块数量扣分  
    if (g_media_stats.bad_block_count > 50) score -= 25;  
    else if (g_media_stats.bad_block_count > 20) score -= 15;  
    else if (g_media_stats.bad_block_count > 5) score -= 5;  
      
    // 根据剩余备用块扣分  
    if (g_media_stats.spare_block_count < 10) score -= 20;  
    else if (g_media_stats.spare_block_count < 30) score -= 10;  
    else if (g_media_stats.spare_block_count < 50) score -= 5;  
      
    // 根据磨损程度扣分  
    if (g_media_stats.wear_level_max > 800000) score -= 25;  // 80%磨损  
    else if (g_media_stats.wear_level_max > 600000) score -= 15;  // 60%磨损  
    else if (g_media_stats.wear_level_max > 400000) score -= 10;  // 40%磨损  
    else if (g_media_stats.wear_level_max > 200000) score -= 5;   // 20%磨损  
      
    // 根据分数确定健康等级  
    if (score >= 90) return MEDIA_HEALTH_EXCELLENT;  
    else if (score >= 75) return MEDIA_HEALTH_GOOD;  
    else if (score >= 60) return MEDIA_HEALTH_FAIR;  
    else if (score >= 40) return MEDIA_HEALTH_POOR;  
    else return MEDIA_HEALTH_CRITICAL;  
}  
  
// 获取介质统计信息  
status_t media_health_get_stats(media_health_stats_t *stats) {  
    if (!health_initialized || !stats) {  
        return STATUS_INVALID_PARAM;  
    }  
      
    // 更新健康等级  
    g_media_stats.health_level = media_health_calculate_level();  
      
    // 复制统计信息  
    memcpy(stats, &g_media_stats, sizeof(media_health_stats_t));  
      
    return STATUS_SUCCESS;  
}