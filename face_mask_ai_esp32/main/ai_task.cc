#include "ai_task.h"
#include "model/model_data.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "jpeg_decoder.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

// Wrap C header for C++ compatibility
extern "C" {
#include "jpeg_decoder.h"
}

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

static const char *TAG = "AI_TASK";

constexpr size_t TENSOR_ARENA_SIZE = 96 * 1024;
static uint8_t *tensor_arena = nullptr;

static const char *class_names[] = {
    "With Mask",
    "No Mask",
    "Incorrect Mask"
};

typedef struct {
    const uint8_t *jpg_buf;
    size_t jpg_len;
    ai_prediction_result_t *result_ptr;
    SemaphoreHandle_t done_sem;
} ai_request_t;

static QueueHandle_t xAiQueue = nullptr;
static tflite::MicroMutableOpResolver<5> resolver;
static tflite::MicroInterpreter *interpreter = nullptr;
static TfLiteTensor *input_tensor = nullptr;
static TfLiteTensor *output_tensor = nullptr;

static bool decode_jpeg_to_rgb888(
    const uint8_t *jpg_buf,
    size_t jpg_len,
    uint8_t *out_rgb,
    int target_w,
    int target_h)
{
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)jpg_buf,
        .indata_size = jpg_len,
        .outbuf = out_rgb,
        .outbuf_size =
            static_cast<size_t>(target_w * target_h * 3),
        .out_format = JPEG_IMAGE_FORMAT_RGB888,
    };

    esp_jpeg_image_output_t out_info;

    esp_err_t res =
        esp_jpeg_decode(&jpeg_cfg, &out_info);

    if (res != ESP_OK) {
        ESP_LOGE(
            TAG,
            "esp_jpeg_decode failed: 0x%x",
            res
        );
        return false;
    }

    ESP_LOGI(
        TAG,
        "JPEG decoded: %dx%d",
        out_info.width,
        out_info.height
    );

    return true;
}

static bool register_model_ops() {
    if (resolver.AddConv2D() != kTfLiteOk) return false;
    if (resolver.AddMaxPool2D() != kTfLiteOk) return false;
    if (resolver.AddMean() != kTfLiteOk) return false;
    if (resolver.AddFullyConnected() != kTfLiteOk) return false;
    if (resolver.AddSoftmax() != kTfLiteOk) return false;
    return true;
}

static void preprocess_and_quantize(const uint8_t *rgb888_data) {
    for (int i = 0; i < 64 * 64 * 3; i++) {
        int value = (int)rgb888_data[i] - 128;
        input_tensor->data.int8[i] = (int8_t)value;
    }
}

static float dequantize_output(int8_t value, float scale, int zero_point) {
    return (static_cast<int>(value) - zero_point) * scale;
}

static void ai_inference_task(void *pvParameters) {
    (void)pvParameters;

    if (!register_model_ops()) {
        ESP_LOGE(TAG, "Operator registration failed");
        vTaskDelete(nullptr);
        return;
    }

    const tflite::Model *model = tflite::GetModel(g_model);
    if (model == nullptr || model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Model loading/schema verification failed");
        vTaskDelete(nullptr);
        return;
    }

    tensor_arena = (uint8_t *)heap_caps_malloc(TENSOR_ARENA_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!tensor_arena) {
        tensor_arena = (uint8_t *)malloc(TENSOR_ARENA_SIZE);
    }

    if (!tensor_arena) {
        ESP_LOGE(TAG, "Arena allocation failed");
        vTaskDelete(nullptr);
        return;
    }

    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors failed");
        vTaskDelete(nullptr);
        return;
    }

    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    ESP_LOGI(TAG, "AI Engine initialized and ready for requests");

    ai_request_t req;
    while (true) {
        if (xQueueReceive(xAiQueue, &req, portMAX_DELAY) == pdTRUE) {

            uint8_t *rgb_buf =
                (uint8_t *)heap_caps_malloc(
                    64 * 64 * 3,
                    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
                );

            if (!rgb_buf) {
                rgb_buf = (uint8_t *)malloc(64 * 64 * 3);
            }

            if (rgb_buf != nullptr) {
                bool converted = decode_jpeg_to_rgb888(req.jpg_buf, req.jpg_len, rgb_buf, 64, 64);

                if (converted) {
                    preprocess_and_quantize(rgb_buf);

                    if (interpreter->Invoke() == kTfLiteOk) {
                        int best_idx = 0;
                        int8_t max_val = output_tensor->data.int8[0];
                        for (int i = 1; i < 3; i++) {
                            if (output_tensor->data.int8[i] > max_val) {
                                max_val = output_tensor->data.int8[i];
                                best_idx = i;
                            }
                        }

                        float conf = dequantize_output(max_val, output_tensor->params.scale, output_tensor->params.zero_point);

                        snprintf(req.result_ptr->status, sizeof(req.result_ptr->status), "%s", class_names[best_idx]);
                        req.result_ptr->confidence = conf * 100.0f;
                    }
                } else {
                    ESP_LOGE(TAG, "JPEG Decoding failed");
                    snprintf(req.result_ptr->status, sizeof(req.result_ptr->status), "Decode Error");
                    req.result_ptr->confidence = 0.0f;
                }
                free(rgb_buf);
            }

            xSemaphoreGive(req.done_sem);
        }
    }
}

extern "C" {

void ai_task_start(void) {
    xAiQueue = xQueueCreate(2, sizeof(ai_request_t));
    xTaskCreate(ai_inference_task, "ai_task", 8192, nullptr, 5, nullptr);
}

bool ai_task_run_inference(const uint8_t *fb_buf, size_t fb_len, ai_prediction_result_t *out_result) {
    if (!xAiQueue) return false;

    ai_request_t req;
    req.jpg_buf = fb_buf;
    req.jpg_len = fb_len;
    req.result_ptr = out_result;
    req.done_sem = xSemaphoreCreateBinary();

    if (!req.done_sem) return false;

    if (xQueueSend(xAiQueue, &req, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if (xSemaphoreTake(req.done_sem, pdMS_TO_TICKS(3000)) == pdTRUE) {
            vSemaphoreDelete(req.done_sem);
            return true;
        }
    }

    vSemaphoreDelete(req.done_sem);
    return false;
}

}