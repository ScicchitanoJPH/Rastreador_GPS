#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "libs/hd44780/HD44780.h"
#include "libs/GPS/gps.h"

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

static const char *TAG = "GPS_APP";

void init_i2c() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void gps_task(void *pvParameter) {
    init_gps();
    while (1) {
        gps_data_t data = {0};
        if(read_data(&data)) {
            printf("Time: %s\n", data.time);
            printf("Latitude: %.7f\n", data.latitude);
            printf("Longitude: %.7f\n", data.longitude);
            printf("Fix quality: %c\n", data.fix_quality);
            printf("Number of satellites: %s\n", data.satellites);
            printf("HDOP: %s\n", data.hdop);
            printf("Altitude: %s %s\n", data.altitude, data.altitude_unit);
            printf("Geoid separation: %s %s\n", data.geoid_sep, data.geoid_sep_unit);
        } else {
            printf("No signal.\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    ESP_LOGI(TAG, "Inicializando...");

    init_i2c();
    
    // lcd.addr = 0x27;
    // lcd.cols = 20;
    // lcd.rows = 4;
    // hd44780_init(&lcd);
    // hd44780_backlight(&lcd, true);
    // hd44780_puts(&lcd, "Iniciando GPS...");

    xTaskCreate(&gps_task, "gps_task", 4096, NULL, 5, NULL);
}
