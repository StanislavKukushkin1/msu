#include "stdio-task/stdio-task.h"
#include "pico/stdlib.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "stdio.h"
#include "bme280-driver.h"
#include "hardware/i2c.h"
#include "stdlib.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void version_callback(const char *args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char *args)
{
    led_state_t state = LED_STATE_ON;
    led_task_state_set(state);
    printf("LED On\n");
}

void led_off_callback(const char *args)
{
    led_state_t state = LED_STATE_OFF;
    led_task_state_set(state);
    printf("LED Off\n");
}

void led_blink_callback(const char *args)
{
    led_state_t state = LED_STATE_BLINK;
    led_task_state_set(state);
    printf("LED Blink\n");
}

void read_reg_callback(const char *args)
{
    unsigned int addr = 0, N = 0;

    sscanf(args, "%x %x", &addr, &N);

    uint8_t buffer[256] = {0};
    bme280_read_regs(addr, buffer, N);

    if (addr > 0xFF && N > 0xFF && addr + N > 0x100)
    {
        printf("Error read_reg\n");
        return;
    }

    for (int i = 0; i < N; i++)
    {
        printf("bme280 register [0x%X] = 0x%X\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char *args)
{
    unsigned int addr = 0, N = 0;

    sscanf(args, "%x %x", &addr, &N);

    if (addr > 0xFF && N > 0xFF && addr + N > 0x100)
    {
        printf("Error read_reg\n");
        return;
    }

    bme280_write_reg(addr, N);
}

void temp_raw_callback(const char *args)
{
    uint16_t raw = bme280_read_temp_raw();
    printf("temp_raw: %u (0x%04X)\n", raw, raw);
}

void pres_raw_callback(const char *args)
{
    uint16_t raw = bme280_read_pres_raw();
    printf("pres_raw: %u (0x%04X)\n", raw, raw);
}

void hum_raw_callback(const char *args)
{
    uint16_t raw = bme280_read_hum_raw();
    printf("hum_raw: %u (0x%04X)\n", raw, raw);
}

void temp_callback(const char* args) {
    float temp = bme280_read_temperature();
    printf("%.2f °C\n", temp);
}

void pres_callback(const char* args) {
    float pres = bme280_read_pressure();
    printf("%.2f Pa\n", pres);
}

void hum_callback(const char* args) {
    float hum = bme280_read_humidity();
    printf("%.2f %%\n", hum);
}

api_t device_api[] =
    {
        {"version", version_callback, "get device name and firmware version"},
        {"on", led_on_callback, "led on"},
        {"off", led_off_callback, "led off"},
        {"blink", led_blink_callback, "led blink"},
        {"read_reg", read_reg_callback, "read reg"},
        {"write_reg", write_reg_callback, "write reg"},
        {"temp_raw", temp_raw_callback, "temp raw"},
        {"pres_raw", pres_raw_callback, "pres raw"},
        {"hum_raw", hum_raw_callback, "hum raw"},
        {"temp", temp_callback, "temperature in °C"},
        {"pres", pres_callback, "pressure in hPa"},
        {"hum", hum_callback, "humidity in %"},
        {NULL, NULL, NULL},
};

void rp2040_i2c_read(uint8_t *buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t *data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

int main()
{
    stdio_init_all();

    stdio_task_init();

    protocol_task_init(device_api);

    led_task_init();

    i2c_init(i2c1, 100000);

    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    while (1)
    {
        // stdio_task_handle();

        protocol_task_handle(stdio_task_handle());

        led_task_handle();
    }
}