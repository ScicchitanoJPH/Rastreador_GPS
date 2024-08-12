#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/queue.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "libs/GPS/gps.h"
#include "libs/WIFI_driver/WIFI_driver.h"
#include "libs/LCD/lcd.h"  // Incluye la cabecera del LCD

static QueueHandle_t queue_gps_data = NULL;
static const char *TAG = "GPS_APP";

void queue_create() {
    queue_gps_data = xQueueCreate(5, sizeof(gps_data_t));
    if (!queue_gps_data) {
        ESP_LOGE(TAG, "Error creating queue");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }
}

void gps_task(void *pvParameter) {
    init_gps();
    while (1) {
        gps_data_t data = {0};
        if (read_data(&data)) {
            ESP_LOGI("gps_task", "Latitude: %.7f, Longitude: %.7f", data.latitude, data.longitude);
            xQueueSend(queue_gps_data, &data, portMAX_DELAY);
        } else {
            ESP_LOGI("gps_task", "No signal.");
            
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void memory_task(void *pvParameter) {
    gps_data_t data;
    char *TAG_memory = "memory_task";
    while (1) {
        xQueueReceive(queue_gps_data, &data, portMAX_DELAY);
        ESP_LOGI(TAG_memory,"Latitude: %.7f\n", data.latitude);
        ESP_LOGI(TAG_memory,"Longitude: %.7f\n", data.longitude);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
    }
}




void display_task(void *pvParameter) {
    gps_data_t data;
    char *TAG_display = "display_task";
    lcd_init();    
    while (1) {
        ESP_LOGI(TAG_display, "Esperando datos GPS...");
        if (xQueueReceive(queue_gps_data, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            lcd_clear();  // Limpiar el LCD
            lcd_showData(data.latitude, data.longitude, data.time);
        } else {
            lcd_clear(); // Limpiar el LCD
            lcd_display_text("No signal.");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




void app_main() {
    ESP_LOGI(TAG, "Inicializando...");
    queue_create();
    

    xTaskCreatePinnedToCore(&gps_task, "gps_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(&memory_task, "memory_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(&display_task, "display_task", 4096, NULL, 5, NULL, 0);
}
