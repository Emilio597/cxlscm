#include "hw_registers.h"  
#include "sensor_driver.h"  
#include "fe_common.h"  
  
static bool sensor_initialized = false;  
  
// 传感器初始化  
fe_status_t sensor_init(void) {  
    // 启用温度传感器  
    REG_WRITE(TEMP_CTRL_REG, 0x01);  // 使能温度传感器  
    REG_WRITE(TEMP_THRESHOLD_REG, 85);  // 设置85°C阈值  
      
    // 启用电压传感器  
    REG_WRITE(VOLTAGE_CTRL_REG, 0x07);  // 使能所有电压通道  
      
    // 启用电流传感器  
    REG_WRITE(CURRENT_CTRL_REG, 0x03);  // 使能电流监测  
      
    sensor_initialized = true;  
    return FE_STATUS_SUCCESS;  
}  
  
// 读取温度  
fe_status_t sensor_read_temperature(uint32_t *temp_celsius) {  
    if (!sensor_initialized || !temp_celsius) {  
        return FE_STATUS_INVALID_PARAM;  
    }  
      
    // 触发温度转换  
    REG_WRITE(TEMP_CTRL_REG, REG_READ(TEMP_CTRL_REG) | 0x02);  
      
    // 等待转换完成  
    while (!(REG_READ(TEMP_STATUS_REG) & 0x01)) {  
        // 等待转换完成标志  
    }  
      
    // 读取温度数据 (假设返回值为摄氏度*10)  
    uint32_t raw_temp = REG_READ(TEMP_DATA_REG);  
    *temp_celsius = raw_temp / 10;  
      
    return FE_STATUS_SUCCESS;  
}  
  
// 读取电压  
fe_status_t sensor_read_voltages(uint32_t *v3v3, uint32_t *v1v8, uint32_t *v1v2) {  
    if (!sensor_initialized || !v3v3 || !v1v8 || !v1v2) {  
        return FE_STATUS_INVALID_PARAM;  
    }  
      
    // 读取各路电压 (假设返回值为毫伏)  
    *v3v3 = REG_READ(VOLTAGE_3V3_REG);  
    *v1v8 = REG_READ(VOLTAGE_1V8_REG);  
    *v1v2 = REG_READ(VOLTAGE_1V2_REG);  
      
    return FE_STATUS_SUCCESS;  
}  
  
// 读取电流  
fe_status_t sensor_read_currents(uint32_t *total_ma, uint32_t *pcm_ma) {  
    if (!sensor_initialized || !total_ma || !pcm_ma) {  
        return FE_STATUS_INVALID_PARAM;  
    }  
      
    // 读取电流数据 (假设返回值为毫安)  
    *total_ma = REG_READ(CURRENT_TOTAL_REG);  
    *pcm_ma = REG_READ(CURRENT_PCM_REG);  
      
    return FE_STATUS_SUCCESS;  
}  
  
// 获取所有传感器数据  
fe_status_t sensor_get_all_data(sensor_data_t *data) {  
    if (!data) {  
        return FE_STATUS_INVALID_PARAM;  
    }  
      
    fe_status_t status;  
      
    // 读取温度  
    status = sensor_read_temperature(&data->temperature_celsius);  
    if (status != FE_STATUS_SUCCESS) return status;  
      
    // 读取电压  
    status = sensor_read_voltages(&data->voltage_3v3_mv, &data->voltage_1v8_mv, &data->voltage_1v2_mv);  
    if (status != FE_STATUS_SUCCESS) return status;  
      
    // 读取电流  
    status = sensor_read_currents(&data->current_total_ma, &data->current_pcm_ma);  
    if (status != FE_STATUS_SUCCESS) return status;  
      
    // 检查状态  
    data->thermal_alert = (data->temperature_celsius > 85);  
    data->power_good = (data->voltage_3v3_mv > 3200 && data->voltage_3v3_mv < 3400) &&  
                       (data->voltage_1v8_mv > 1700 && data->voltage_1v8_mv < 1900) &&  
                       (data->voltage_1v2_mv > 1100 && data->voltage_1v2_mv < 1300);  
      
    return FE_STATUS_SUCCESS;  
}