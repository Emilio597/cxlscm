#ifndef SENSOR_DRIVER_H  
#define SENSOR_DRIVER_H  
  
#include <stdint.h>  
#include <stdbool.h>  
  
// 传感器寄存器基地址  
#define SENSOR_BASE                 (CXL_SCM_PERIPH_BASE + 0x300000)  
  
// 温度传感器寄存器  
#define TEMP_SENSOR_BASE            (SENSOR_BASE + 0x1000)  
#define TEMP_CTRL_REG               (TEMP_SENSOR_BASE + 0x00)  
#define TEMP_DATA_REG               (TEMP_SENSOR_BASE + 0x04)  
#define TEMP_STATUS_REG             (TEMP_SENSOR_BASE + 0x08)  
#define TEMP_THRESHOLD_REG          (TEMP_SENSOR_BASE + 0x0C)  
  
// 电压传感器寄存器  
#define VOLTAGE_SENSOR_BASE         (SENSOR_BASE + 0x2000)  
#define VOLTAGE_CTRL_REG            (VOLTAGE_SENSOR_BASE + 0x00)  
#define VOLTAGE_3V3_REG             (VOLTAGE_SENSOR_BASE + 0x04)  
#define VOLTAGE_1V8_REG             (VOLTAGE_SENSOR_BASE + 0x08)  
#define VOLTAGE_1V2_REG             (VOLTAGE_SENSOR_BASE + 0x0C)  
  
// 电流传感器寄存器  
#define CURRENT_SENSOR_BASE         (SENSOR_BASE + 0x3000)  
#define CURRENT_CTRL_REG            (CURRENT_SENSOR_BASE + 0x00)  
#define CURRENT_TOTAL_REG           (CURRENT_SENSOR_BASE + 0x04)  
#define CURRENT_PCM_REG             (CURRENT_SENSOR_BASE + 0x08)  
  
// 传感器数据结构  
typedef struct {  
    uint32_t temperature_celsius;  
    uint32_t voltage_3v3_mv;  
    uint32_t voltage_1v8_mv;  
    uint32_t voltage_1v2_mv;  
    uint32_t current_total_ma;  
    uint32_t current_pcm_ma;  
    bool thermal_alert;  
    bool power_good;  
} sensor_data_t;  
  
// 传感器接口函数  
fe_status_t sensor_init(void);  
fe_status_t sensor_read_temperature(uint32_t *temp_celsius);  
fe_status_t sensor_read_voltages(uint32_t *v3v3, uint32_t *v1v8, uint32_t *v1v2);  
fe_status_t sensor_read_currents(uint32_t *total_ma, uint32_t *pcm_ma);  
fe_status_t sensor_get_all_data(sensor_data_t *data);  
  
#endif // SENSOR_DRIVER_H