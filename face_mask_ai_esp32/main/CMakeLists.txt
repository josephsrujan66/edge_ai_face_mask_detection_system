idf_component_register(
    SRCS
        "main.c"
        "ai_task.cc"
        "test_image.cc"
        "model/model_data.cc"
        "wifi_manager.c"
        "http_server.c"
        "mdns_service.c"

    INCLUDE_DIRS
        "."
        "model"

    REQUIRES
        esp_https_server
        esp-tflite-micro
        freertos
        nvs_flash
        esp_timer
        heap
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        mdns
        espressif__esp_jpeg
    
    EMBED_TXTFILES
        "../certs/server_cert.pem"
        "../certs/server_key.pem"
)