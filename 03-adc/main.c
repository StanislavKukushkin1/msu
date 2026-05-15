#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "adc-task/adc-task.h"

#define CMD_BUFFER_SIZE 64
#define LED_PIN 25

static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

// Инициализация светодиода
void led_init(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
}

void led_set(bool state) {
    gpio_put(LED_PIN, state);
}

// Функция обработки команд
void process_command(const char* cmd) {
    if (strcmp(cmd, "get_adc") == 0) {
        float voltage = adc_task_get_voltage();
        printf("%f\n", voltage);
    }
    else if (strcmp(cmd, "get_temp") == 0) {
        float temperature = adc_task_get_temperature();
        printf("%f\n", temperature);
    }
    else if (strcmp(cmd, "tm_start") == 0) {
        adc_task_set_state(ADC_TASK_STATE_RUN);
        printf("Telemetry started\n");
    }
    else if (strcmp(cmd, "tm_stop") == 0) {
        adc_task_set_state(ADC_TASK_STATE_IDLE);
        printf("Telemetry stopped\n");
    }
    else if (strcmp(cmd, "led_on") == 0) {
        led_set(true);
        printf("LED ON\n");
    }
    else if (strcmp(cmd, "led_off") == 0) {
        led_set(false);
        printf("LED OFF\n");
    }
    else if (strcmp(cmd, "help") == 0) {
        printf("\nAvailable commands:\n");
        printf("  get_adc  - read ADC voltage on GPIO 26\n");
        printf("  get_temp - read internal temperature sensor\n");
        printf("  tm_start - start continuous telemetry\n");
        printf("  tm_stop  - stop continuous telemetry\n");
        printf("  led_on   - turn LED on\n");
        printf("  led_off  - turn LED off\n");
        printf("  help     - show this help\n");
        printf("  clear    - clear screen\n\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        printf("\033[2J\033[H");
        printf("> ");
        fflush(stdout);
    }
    else if (strlen(cmd) > 0) {
        printf("Unknown command: %s\n", cmd);
    }
}

int main() {
    // Инициализация
    stdio_init_all();
    adc_task_init();
    led_init();
    
    // Задержка для стабилизации USB/UART
    sleep_ms(2000);
    
    // Приветственное сообщение
    printf("\033[2J\033[H");
    printf("========================================\n");
    printf("   RP2040 ADC Control System\n");
    printf("   with Telemetry Mode\n");
    printf("========================================\n");
    printf("Type 'help' for available commands\n\n");
    printf("> ");
    fflush(stdout);
    
    // Основной цикл
    while (1) {
        int ch = getchar_timeout_us(0);
        
        if (ch != PICO_ERROR_TIMEOUT) {
            // Выводим нажатый символ (эхо)
            putchar(ch);
            
            // Обработка Enter
            if (ch == '\n' || ch == '\r') {
                if (cmd_index > 0) {
                    cmd_buffer[cmd_index] = '\0';
                    printf("\n");
                    
                    process_command(cmd_buffer);
                    
                    // Очистка буфера и вывод приглашения
                    cmd_index = 0;
                    printf("> ");
                    fflush(stdout);
                } else {
                    printf("> ");
                    fflush(stdout);
                }
            }
            // Обработка Backspace
            else if (ch == 127 || ch == 8) {
                if (cmd_index > 0) {
                    cmd_index--;
                    printf("\b \b");
                    fflush(stdout);
                }
            }
            // Обычные символы
            else if (ch >= 32 && ch <= 126 && cmd_index < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[cmd_index++] = (char)ch;
                fflush(stdout);
            }
            // Если буфер переполнен
            else if (cmd_index >= CMD_BUFFER_SIZE - 1) {
                printf("\nCommand too long!\n> ");
                cmd_index = 0;
                fflush(stdout);
            }
        }
        
        // Обработчик телеметрии (периодические измерения)
        adc_task_handle();
        
        tight_loop_contents();
    }
    
    return 0;
}