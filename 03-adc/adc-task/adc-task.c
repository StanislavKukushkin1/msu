#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "adc-task.h"

#define ADC_GPIO_PIN 26
#define ADC_CHANNEL_GPIO 0
#define ADC_CHANNEL_TEMP 4
#define ADC_TASK_MEAS_PERIOD_US 100000  // 100 мс = 100 000 мкс

static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t last_measure_time_us = 0;

void adc_task_init(void) {
    adc_init();
    adc_gpio_init(ADC_GPIO_PIN);
    
    // Включение встроенного датчика температуры
    adc_set_temp_sensor_enabled(true);
}

float adc_task_get_voltage(void) {
    adc_select_input(ADC_CHANNEL_GPIO);
    uint16_t voltage_counts = adc_read();
    
    // Опорное напряжение 3.3 В, разрядность 12 бит (0-4095)
    float voltage_V = (voltage_counts * 3.3f) / 4095.0f;
    
    return voltage_V;
}

float adc_task_get_temperature(void) {
    adc_select_input(ADC_CHANNEL_TEMP);
    uint16_t temp_counts = adc_read();
    
    // Преобразование отсчётов в напряжение (вольты)
    float temp_V = (temp_counts * 3.3f) / 4095.0f;
    
    // Преобразование напряжения в температуру (°C)
    // Формула из datasheet RP2040
    float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;
    
    return temp_C;
}

void adc_task_set_state(adc_task_state_t state) {
    adc_state = state;
    if (state == ADC_TASK_STATE_RUN) {
        // Сброс таймера при запуске, чтобы измерение произошло немедленно
        last_measure_time_us = time_us_64();
    }
}

void adc_task_handle(void) {
    if (adc_state == ADC_TASK_STATE_RUN) {
        uint64_t now_us = time_us_64();
        
        // Проверяем, прошло ли достаточно времени для следующего измерения
        if (now_us - last_measure_time_us >= ADC_TASK_MEAS_PERIOD_US) {
            last_measure_time_us = now_us;
            
            // Измеряем напряжение и температуру
            float voltage_V = adc_task_get_voltage();
            float temp_C = adc_task_get_temperature();
            
            // Выводим в формате "напряжение температура\n"
            printf("%f %f\n", voltage_V, temp_C);
        }
    }
}