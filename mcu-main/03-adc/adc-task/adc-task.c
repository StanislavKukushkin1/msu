#include "hardware/adc.h"

#include "stdio.h"
#include "pico/stdlib.h"

#include "adc-task.h"

static int GPIO = 26;
static int ADC = 0;
static int ADC_TEMP_PIN = 4;

const uint64_t ADC_TASK_MEAS_PERIOD_US = 100000;

static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t last_measure_ts = 0;

void adc_task_init()
{
    adc_init();

    adc_gpio_init(GPIO);

    adc_set_temp_sensor_enabled(true);
}

float adc_task_measurement()
{
    adc_select_input(ADC);
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts * 3.3f / 4096.0f;
    return voltage_V;
}

float adc_task_measure_temperature()
{
    adc_select_input(ADC_TEMP_PIN);

    uint16_t voltage_counts = adc_read();

    float temp_V = voltage_counts * 3.3f / 4096.0f;

    float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;

    return temp_C;
}

void adc_task_set_state(adc_task_state_t state)
{
    adc_state = state;
    if (state == ADC_TASK_STATE_RUN)
    {
        last_measure_ts = time_us_64();
    }
}

void adc_task_handle()
{
    if (adc_state != ADC_TASK_STATE_RUN)
    {
        return;
    }

    uint64_t now = time_us_64();
    if (now >= last_measure_ts + ADC_TASK_MEAS_PERIOD_US)
    {
        last_measure_ts = now;

        float voltage = adc_task_measurement();
        float temperature = adc_task_measure_temperature();

        printf("%f %f\n", voltage, temperature);
    }
}