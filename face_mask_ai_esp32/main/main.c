#include <stdio.h>

#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi_manager.h"
#include "http_server.h"
#include "ai_task.h"
#include "mdns_service.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32 Remote Camera AI System");
    ESP_LOGI(TAG, "========================================");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* 1. Start TFLite Inference Queue/Worker Task */
    ai_task_start();

    /* 2. Connect to Wi-Fi */
    wifi_manager_init();

    mdns_service_start();
    
    /* 3. Start HTTP Server for UI and /predict API */
    http_server_start();


    ESP_LOGI(TAG, "System initialization complete");
}