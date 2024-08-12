// lcd.c

#include "lcd.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void lcd_write_byte(uint8_t data, uint8_t mode) {
    uint8_t upper_nibble = data & 0xF0;
    uint8_t lower_nibble = (data << 4) & 0xF0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, upper_nibble | mode | 0x08, true); // 0x08 for backlight
    i2c_master_write_byte(cmd, upper_nibble | mode | 0x0C, true); // Enable high
    i2c_master_write_byte(cmd, upper_nibble | mode | 0x08, true); // Enable low
    i2c_master_write_byte(cmd, lower_nibble | mode | 0x08, true);
    i2c_master_write_byte(cmd, lower_nibble | mode | 0x0C, true);
    i2c_master_write_byte(cmd, lower_nibble | mode | 0x08, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void lcd_send_command(uint8_t command) {
    lcd_write_byte(command & 0xF0, 0);
    lcd_write_byte((command << 4) & 0xF0, 0);
}


void lcd_return_home(void) {
    lcd_send_command(0x02);  // Return home
    vTaskDelay(2 / portTICK_PERIOD_MS);  // Delay for command execution
}

void lcd_clear(void) {
    lcd_send_command(0x01);  // Clear display
    vTaskDelay(2 / portTICK_PERIOD_MS);  // Delay for command execution
    lcd_set_cursor(0, 0);  // Move cursor to home position
}


void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_send_command(0x80 | (col + row_offsets[row]));
}

void lcd_init() {
    i2c_master_init();
    vTaskDelay(50 / portTICK_PERIOD_MS);
    lcd_write_byte(0x30, 0); // Wake-up
    vTaskDelay(5 / portTICK_PERIOD_MS);
    lcd_write_byte(0x30, 0); // Wake-up
    vTaskDelay(5 / portTICK_PERIOD_MS);
    lcd_write_byte(0x30, 0); // Wake-up
    vTaskDelay(1 / portTICK_PERIOD_MS);
    lcd_write_byte(0x20, 0); // 4-bit mode

    lcd_send_command(0x2C); // Function set: 4-bit, 4 line, 5x8 dots
    vTaskDelay(1 / portTICK_PERIOD_MS);
    lcd_send_command(0x0C); // Display on, cursor off
    vTaskDelay(1 / portTICK_PERIOD_MS);
    lcd_send_command(0x06); // Entry mode, move cursor right
    vTaskDelay(1 / portTICK_PERIOD_MS);
    lcd_send_command(0x00); // Cursor OFF
    vTaskDelay(1 / portTICK_PERIOD_MS);
    lcd_clear();            // Clear display
}

void lcd_display_text(const char *text) {
    while (*text) {
        lcd_write_byte(*text++, 1);  // Send text
    }
}

void lcd_showData(float latitude, float longitude, char *time) {
    char buffer[32];

    lcd_clear();  // Limpiar la pantalla

    // Mostrar Latitud y Hora en la primera fila
    snprintf(buffer, sizeof(buffer), "Lat: %.7f ", latitude);
    lcd_set_cursor(0, 0);  // Ir al principio de la primera línea
    lcd_display_text(buffer);

    snprintf(buffer, sizeof(buffer), "Lon: %.7f", longitude);
    lcd_set_cursor(0, 1);  // Ir al principio de la segunda línea
    lcd_display_text(buffer);

    snprintf(buffer, sizeof(buffer), "Time: %s", time);
    lcd_set_cursor(0, 2);  // Ir al principio de la segunda línea
    lcd_display_text(buffer);

    snprintf(buffer, sizeof(buffer), "Fila 4");
    lcd_set_cursor(0, 3);  // Ir al principio de la segunda línea
    lcd_display_text(buffer);
}

void i2c_scanner() {
    printf("Escaneando I2C...\n");
    for (int i = 1; i < 127; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            printf("Dispositivo encontrado en la dirección 0x%02x\n", i);
        }
    }
    printf("Escaneo completado.\n");
}
