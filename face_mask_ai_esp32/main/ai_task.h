#ifndef AI_TASK_H
#define AI_TASK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char status[32];
    float confidence;
} ai_prediction_result_t;

void ai_task_start(void);
bool ai_task_run_inference(const uint8_t *jpg_buf, size_t jpg_len, ai_prediction_result_t *out_result);

#ifdef __cplusplus
}
#endif

#endif // AI_TASK_H
