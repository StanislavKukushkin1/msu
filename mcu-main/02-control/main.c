#include "stdio-task/stdio-task.h"
#include "pico/stdlib.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "stdio.h"

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

void led_blink_set_period_ms_callback(const char *args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);

    if (period_ms == 0)
    {
        printf("Error: period cannot be zero\n");
        return;
    }

    led_task_set_blink_period_ms(period_ms);
    printf("LED blink period set to %u ms\n", period_ms);
}

void help_callback(const char *args);

void mem_callback(const char *args)
{
    uint32_t addr = 0;

    if (sscanf(args, "%x", &addr) != 1)
    {
        printf("Error: invalid address format. Usage: mem <addr>\n");
        return;
    }

    volatile uint32_t *ptr = (volatile uint32_t *)addr;
    uint32_t value = *ptr;

    printf("Memory at 0x%08X: 0x%08X (dec: %u)\n", addr, value, value);
}

void wmem_callback(const char *args)
{
    uint32_t addr = 0;
    uint32_t value = 0;

    if (sscanf(args, "%x %x", &addr, &value) != 2)
    {
        printf("Error: invalid arguments. Usage: wmem <addr> <value>\n");
        return;
    }

    volatile uint32_t *ptr = (volatile uint32_t *)addr;
    *ptr = value;

    printf("Written 0x%08X to address 0x%08X\n", value, addr);
}

api_t device_api[] =
    {
        {"help", help_callback, "help"},
        {"version", version_callback, "get device name and firmware version"},
        {"on", led_on_callback, "led on"},
        {"off", led_off_callback, "led off"},
        {"blink", led_blink_callback, "led blink"},
        {"set_period", led_blink_set_period_ms_callback, "set period"},

        {"mem", mem_callback, "read memory at address"},
        {"wmem", wmem_callback, "write memory at address"},
        {NULL, NULL, NULL},
};

void help_callback(const char *args)
{
    printf("\nAvailable commands:\n");

    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %-20s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
    printf("\n");
}

int main()
{
    stdio_init_all();

    stdio_task_init();

    protocol_task_init(device_api);

    led_task_init();

    while (1)
    {
        // stdio_task_handle();

        protocol_task_handle(stdio_task_handle());

        led_task_handle();
    }
}