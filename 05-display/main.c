#include "stdio-task/stdio-task.h"
#include "pico/stdlib.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "stdio.h"
#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "hardware/spi.h"
#include "ili9341-font.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

static ili9341_display_t ili9341_display = {0};
static ili9341_hal_t ili9341_hal = {0};

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9
// #define PIN_LED -> 3.3V

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

void disp_screen_callback(const char *args)
{
    uint32_t c = 0;
    int result = sscanf(args, "%x", &c);

    uint16_t color = COLOR_BLACK;

    if (result == 1)
    {
        color = RGB888_2_RGB565(c);
    }

    ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char *args)
{
    int x = 0;
    int y = 0;
    uint32_t c = 0;

    int result = sscanf(args, "%d %d %x", &x, &y, &c);

    if (result < 2)
    {
        return;
    }

    uint16_t color = COLOR_WHITE;

    if (result == 3)
    {
        color = RGB888_2_RGB565(c);
    }

    ili9341_draw_pixel(&ili9341_display, x, y, color);
}

void disp_line_callback(const char *args)
{
    int x0, y0, x1, y1;
    uint32_t c;

    int result = sscanf(args, "%d %d %d %d %x", &x0, &y0, &x1, &y1, &c);

    if (result != 5)
        return;

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_line(&ili9341_display, x0, y0, x1, y1, color);
}

void disp_rect_callback(const char *args)
{
    int x, y, w, h;
    uint32_t c;

    int result = sscanf(args, "%d %d %d %d %x",
                        &x, &y, &w, &h, &c);

    if (result != 5)
        return;

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_rect(&ili9341_display,
                      x, y, w, h,
                      color);
}

void disp_frect_callback(const char *args)
{
    int x, y, w, h;
    uint32_t c;

    int result = sscanf(args, "%d %d %d %d %x",
                        &x, &y, &w, &h, &c);

    if (result != 5)
        return;

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_filled_rect(&ili9341_display,
                             x, y, w, h,
                             color);
}

void disp_text_callback(const char *args)
{
    int x = 0;
    int y = 0;

    char text[64];

    uint32_t c;
    uint32_t bg;

    int result = sscanf(args, "%d %d %63s %x %x",
                        &x, &y, text, &c, &bg);

    if (result != 5)
        return;

    uint16_t color = RGB888_2_RGB565(c);
    uint16_t bgcolor = RGB888_2_RGB565(bg);

    ili9341_draw_text(&ili9341_display,
                      x,
                      y,
                      text,
                      &jetbrains_font,
                      color,
                      bgcolor);
}

void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

void ili9341_rp2040_init()
{
    spi_init(spi0, 62500000);

    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RESET);

    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);

    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_RESET, 0);

    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;
}

api_t device_api[] =
    {
        {"version", version_callback, "get device name and firmware version"},
        {"on", led_on_callback, "led on"},
        {"off", led_off_callback, "led off"},
        {"blink", led_blink_callback, "led blink"},
        {"disp_screen", disp_screen_callback, "disp screen"},
        {"disp_px", disp_px_callback, "disp px"},
        {"disp_line", disp_line_callback, "disp line"},
        {"disp_rect", disp_rect_callback, "disp rect"},
        {"disp_frect", disp_frect_callback, "disp frect"},
        {"disp_text", disp_text_callback, "disp text"},
        {NULL, NULL, NULL},
};

int main()
{
    stdio_init_all();

    stdio_task_init();

    protocol_task_init(device_api);

    led_task_init();

    ili9341_rp2040_init();
    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    sleep_ms(300);

    ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
    ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
    ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);

    ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);

    ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
    ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);

    ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);

    ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);

    while (1)
    {
        // stdio_task_handle();

        protocol_task_handle(stdio_task_handle());

        led_task_handle();
    }
}