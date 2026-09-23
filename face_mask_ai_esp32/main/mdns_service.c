#include "mdns_service.h"

#include "esp_log.h"
#include "mdns.h"

static const char *TAG = "MDNS";

void mdns_service_start(void)
{
    esp_err_t err = mdns_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS init failed: %s",
                 esp_err_to_name(err));
        return;
    }

    err = mdns_hostname_set("face-mask");

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set hostname");
        return;
    }

    mdns_instance_name_set("ESP32 Face Mask AI");

    err = mdns_service_add(
        "Face Mask AI",
        "_https",
        "_tcp",
        443,
        NULL,
        0
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add HTTPS service");
        return;
    }

    ESP_LOGI(TAG, "mDNS started");
    ESP_LOGI(TAG, "Open: https://face-mask.local");
} 