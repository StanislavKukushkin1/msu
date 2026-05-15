#include "stdio-task/stdio-task.h"
#include "pico/stdlib.h"
#include "protocol-task.h"
#include "stdio.h"
#include "bme280-driver.h"
#include "hardware/i2c.h"
#include "stdlib.h"
#include "pico/stdlib.h"
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

void help_callback(const char *args);

api_t device_api[] =
    {
        {"help", help_callback, "help"},
        {"version", version_callback, "get device name and firmware version"},
        {"temp_raw", temp_raw_callback, "temp raw"},
        {"pres_raw", pres_raw_callback, "pres raw"},
        {"hum_raw", hum_raw_callback, "hum raw"},
        {"disp_screen", disp_screen_callback, "disp screen"},
        {"disp_px", disp_px_callback, "disp px"},
        {"disp_line", disp_line_callback, "disp line"},
        {"disp_rect", disp_rect_callback, "disp rect"},
        {"disp_frect", disp_frect_callback, "disp frect"},
        {"disp_text", disp_text_callback, "disp text"},
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

void rp2040_i2c_read(uint8_t *buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t *data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

void draw_temp_colum(float temp)
{
    uint16_t color;
    int n;
    if (temp < 20)
    {
        color = RGB888_2_RGB565(0x0000FF);
        n = 1;
    }
    else if (20 <= temp && temp < 22)
    {
        color = RGB888_2_RGB565(0x00FFFF);
        n = 2;
    }
    else if (22 <= temp && temp <= 24)
    {
        color = RGB888_2_RGB565(0x00FF00);
        n = 3;
    }
    else if (24 < temp && temp <= 26)
    {
        color = RGB888_2_RGB565(0xFFA500);
        n = 4;
    }
    else if (26 < temp)
    {
        color = RGB888_2_RGB565(0xFF0000);
        n = 5;
    }
    for (int i = 0; i < n; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 10, 200 - i * 40, 80, 30, color);
    }
    for (int i = n; i < 5; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 0, 200 - i * 40, 100, 30, COLOR_BLACK);
    }
}

void draw_pres_colum(float pres)
{
    uint16_t color;
    int n;
    if (pres < 9730)
    {
        color = RGB888_2_RGB565(0x0000FF);
        n = 1;
    }
    else if (9730 <= pres && pres < 9995)
    {
        color = RGB888_2_RGB565(0x00FFFF);
        n = 2;
    }
    else if (9995 <= pres && pres <= 10270)
    {
        color = RGB888_2_RGB565(0x00FF00);
        n = 3;
    }
    else if (10270 < pres && pres <= 10530)
    {
        color = RGB888_2_RGB565(0xFFA500);
        n = 4;
    }
    else if (10530 < pres)
    {
        color = RGB888_2_RGB565(0xFF0000);
        n = 5;
    }
    for (int i = 0; i < n; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 110, 200 - i * 40, 120, 30, color);
    }
    for (int i = n; i < 5; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 100, 200 - i * 40, 140, 30, COLOR_BLACK);
    }
}

void draw_hum_colum(float hum)
{
    uint16_t color;
    int n;
    if (hum < 30)
    {
        color = RGB888_2_RGB565(0x0000FF);
        n = 1;
    }
    else if (30 <= hum && hum < 40)
    {
        color = RGB888_2_RGB565(0x00FFFF);
        n = 2;
    }
    else if (40 <= hum && hum <= 60)
    {
        color = RGB888_2_RGB565(0x00FF00);
        n = 3;
    }
    else if (60 < hum && hum <= 70)
    {
        color = RGB888_2_RGB565(0xFFA500);
        n = 4;
    }
    else if (70 < hum)
    {
        color = RGB888_2_RGB565(0xFF0000);
        n = 5;
    }
    for (int i = 0; i < n; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 250, 200 - i * 40, 120, 30, color);
    }
    for (int i = n; i < 5; i++)
    {
        ili9341_draw_filled_rect(&ili9341_display, 240, 200 - i * 40, 140, 30, COLOR_BLACK);
    }
}

int main()
{
    stdio_init_all();

    stdio_task_init();

    protocol_task_init(device_api);

    i2c_init(i2c1, 100000);

    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    ili9341_rp2040_init();
    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    sleep_ms(300);
    ili9341_draw_text(&ili9341_display, 0, 0, "Hi", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    sleep_ms(300);
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);

    ili9341_draw_text(&ili9341_display, 0, 0, "Temp, C: ", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    ili9341_draw_text(&ili9341_display, 100, 0, "Press, hPa: ", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
    ili9341_draw_text(&ili9341_display, 240, 0, "Hum, %: ", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);

    while (1)
    {
        // stdio_task_handle();

        float temp = bme280_read_temperature();

        float pres = bme280_read_pressure() / 10;

        float hum = bme280_read_humidity();
        // SET TEMP
        char temp_buffer[32];
        sprintf(temp_buffer, "%.2f", temp);
        ili9341_draw_text(&ili9341_display, 60, 0, temp_buffer, &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
        draw_temp_colum(temp);

        // SET PRES
        char pres_buffer[32];
        sprintf(pres_buffer, "%.2f", pres);
        ili9341_draw_text(&ili9341_display, 180, 0, pres_buffer, &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
        draw_pres_colum(pres);

        // SET HUM
        char hum_buffer[32];
        sprintf(hum_buffer, "%.2f", hum);
        ili9341_draw_text(&ili9341_display, 290, 0, hum_buffer, &jetbrains_font, COLOR_WHITE, COLOR_BLACK);
        draw_hum_colum(hum);

        sleep_ms(500);

        protocol_task_handle(stdio_task_handle());
    }
}