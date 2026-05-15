#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void help_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args)
{
    led_task_state_set(LED_STATE_ON);
    printf("led on\n");
}

void led_off_callback(const char* args)
{
    led_task_state_set(LED_STATE_OFF);
    printf("led off\n");
}

void led_blink_callback(const char* args)
{
    led_task_state_set(LED_STATE_BLINK);
    printf("led blink\n");
}

void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);
    if (period_ms == 0)
    {
        printf("ERROR! Period can't be 0 ms\n");
        return;
    }
    led_task_set_blink_period_ms(period_ms);
    printf("blink period set to %u ms\n", period_ms);
}

api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
	{"on", led_on_callback, "turn led on"},
	{"off", led_off_callback, "turn led off"},
	{"blink", led_blink_callback, "blink led"},
	{"set_period", led_blink_set_period_ms_callback, "set blink period in ms"},
	{"mem", mem_callback, "read memory: mem <hex_addr>"},
	{"wmem", wmem_callback, "write memory: wmem <hex_addr> <hex_value>"},
	{"help", help_callback, "show available commands"},
	{NULL, NULL, NULL},
};

void help_callback(const char* args)
{
    printf("available commands:\n");
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

void mem_callback(const char* args)
{
    uint32_t addr = 0;
    sscanf(args, "%x", &addr);
    
    if (addr == 0)
    {
        printf("ERROR! Invalid address\n");
        return;
    }
    
    uint32_t value = *(uint32_t*)addr;
    printf("mem[0x%08X] = 0x%08X (%u)\n", addr, value, value);
}

void wmem_callback(const char* args)
{
    uint32_t addr = 0;
    uint32_t value = 0;
    sscanf(args, "%x %x", &addr, &value);
    
    if (addr == 0)
    {
        printf("ERROR! Invalid address\n");
        return;
    }
    
    *(uint32_t*)addr = value;
    printf("wrote 0x%08X to address 0x%08X\n", value, addr);
}

int main() {
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    while(1) {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();
    }
    
    return 0;
}