#include "http_server.h"
#include "ai_task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_https_server.h"
#include "esp_log.h"

/*
 * HTTPS certificate and private key embedded into the firmware
 * through EMBED_TXTFILES in main/CMakeLists.txt.
 */
extern const unsigned char server_cert_pem_start[]
    asm("_binary_server_cert_pem_start");

extern const unsigned char server_cert_pem_end[]
    asm("_binary_server_cert_pem_end");

extern const unsigned char server_key_pem_start[]
    asm("_binary_server_key_pem_start");

extern const unsigned char server_key_pem_end[]
    asm("_binary_server_key_pem_end");

static const char *TAG = "HTTPS_SERVER";


/*
 * Web page served by ESP32.
 *
 * Access using:
 *
 *     https://face-mask.local
 *
 * The browser camera requires a secure context.
 */
static const char *HTML_PAGE =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>ESP32 Face Mask AI</title>"

    "<style>"

    "body {"
    "  font-family: Arial, sans-serif;"
    "  text-align: center;"
    "  background-color: #f4f7f6;"
    "  margin: 0;"
    "  padding: 20px;"
    "}"

    ".container {"
    "  max-width: 440px;"
    "  margin: 0 auto;"
    "  background: white;"
    "  padding: 20px;"
    "  border-radius: 12px;"
    "  box-shadow: 0 4px 10px rgba(0,0,0,0.1);"
    "}"

    "h1 {"
    "  color: #333;"
    "  font-size: 20px;"
    "  margin-bottom: 15px;"
    "}"

    ".portal {"
    "  position: relative;"
    "  width: 100%;"
    "  height: 280px;"
    "  background-color: #222;"
    "  border-radius: 8px;"
    "  display: flex;"
    "  align-items: center;"
    "  justify-content: center;"
    "  overflow: hidden;"
    "  margin-bottom: 15px;"
    "}"

    "video {"
    "  width: 100%;"
    "  height: 100%;"
    "  object-fit: cover;"
    "}"

    ".camera-overlay {"
    "  position: absolute;"
    "  inset: 0;"
    "  background: rgba(0,0,0,0.78);"
    "  color: white;"
    "  display: flex;"
    "  flex-direction: column;"
    "  align-items: center;"
    "  justify-content: center;"
    "  gap: 12px;"
    "  border-radius: 8px;"
    "}"

    ".camera-icon {"
    "  font-size: 42px;"
    "}"

    ".camera-title {"
    "  font-size: 18px;"
    "  font-weight: bold;"
    "}"

    ".camera-message {"
    "  font-size: 14px;"
    "  color: #ddd;"
    "  padding: 0 20px;"
    "}"
    

    ".btn {"
    "  width: 100%;"
    "  padding: 12px;"
    "  font-size: 16px;"
    "  font-weight: bold;"
    "  color: white;"
    "  background-color: #007bff;"
    "  border: none;"
    "  border-radius: 6px;"
    "  cursor: pointer;"
    "}"

    ".btn:disabled {"
    "  background-color: #ccc;"
    "  cursor: not-allowed;"
    "}"

    ".allow-camera-btn {"
    "  width: auto;"
    "  min-width: 180px;"
    "}"

    ".status-card {"
    "  margin-top: 20px;"
    "  padding: 15px;"
    "  border-radius: 8px;"
    "  background: #e9ecef;"
    "  text-align: left;"
    "}"

    ".status-item {"
    "  font-size: 16px;"
    "  margin: 8px 0;"
    "  color: #495057;"
    "}"

    ".status-value {"
    "  font-weight: bold;"
    "  color: #111;"
    "}"

    ".mask-ok {"
    "  color: #28a745;"
    "}"

    ".mask-none {"
    "  color: #dc3545;"
    "}"

    ".mask-bad {"
    "  color: #fd7e14;"
    "}"

    ".connection-info {"
    "  margin-top: 12px;"
    "  font-size: 13px;"
    "  color: #6c757d;"
    "}"

    "</style>"
    "</head>"

    "<body>"

    "<div class='container'>"

    "<h1>Face Mask AI Detector</h1>"

    "<div class='portal'>"

    "<video id='webcam' autoplay playsinline></video>"

    "<div class='camera-overlay' id='camera-overlay'>"

    "<div class='camera-icon'>📷</div>"

    "<div class='camera-title'>Camera Access Required</div>"

    "<div class='camera-message'>"
    "Allow camera access to use the Face Mask AI detector."
    "</div>"

    "<button class='btn allow-camera-btn'"
    " id='allow-camera-btn'"
    " onclick='startCamera()'>"
    "Allow Camera"
    "</button>"

    "</div>"
    "</div>"

    "<button class='btn'"
    " id='predict-btn'"
    " onclick='captureAndPredict()'"
    " disabled>"
    "Predict"
    "</button>"

    "<div class='status-card'>"

    "<div class='status-item'>"
    "Status: "
    "<span id='mask-status' class='status-value'>"
    "Camera required"
    "</span>"
    "</div>"

    "<div class='status-item'>"
    "Confidence: "
    "<span id='confidence-val' class='status-value'>--%</span>"
    "</div>"

    "</div>"

    "<div class='connection-info'>"
    "ESP32 Face Mask AI"
    "</div>"

    "</div>"

    "<canvas id='capture-canvas' style='display:none;'></canvas>"


    "<script>"

    "const video = document.getElementById('webcam');"
    "const canvas = document.getElementById('capture-canvas');"
    "const overlay = document.getElementById('camera-overlay');"
    "const allowButton = document.getElementById('allow-camera-btn');"
    "const predictButton = document.getElementById('predict-btn');"
    "const statusEl = document.getElementById('mask-status');"


    /*
     * Start camera only after the user clicks
     * "Allow Camera".
     */
    "async function startCamera() {"

    "  allowButton.disabled = true;"
    "  allowButton.innerText = 'Requesting Camera...';"

    "  try {"

    "    if (!navigator.mediaDevices ||"
    "        !navigator.mediaDevices.getUserMedia) {"

    "      throw new Error("
    "        'Camera API is not available. HTTPS may be required.'"
    "      );"

    "    }"

    "    const stream = await navigator.mediaDevices.getUserMedia({"

    "      video: {"
    "        facingMode: 'user',"
    "        width: { ideal: 320 },"
    "        height: { ideal: 240 }"
    "      },"

    "      audio: false"

    "    });"

    "    video.srcObject = stream;"

    "    overlay.style.display = 'none';"

    "    predictButton.disabled = false;"

    "    statusEl.innerText = 'Ready';"
    "    statusEl.className = 'status-value';"

    "  }"

    "  catch (err) {"

    "    console.error('Camera error:', err);"

    "    allowButton.disabled = false;"
    "    allowButton.innerText = 'Allow Camera';"

    "    if (err.name === 'NotAllowedError') {"

    "      statusEl.innerText = 'Camera permission denied';"

    "      alert("
    "        'Camera permission was denied. '"
    "        + 'Please allow camera access in the browser.'"
    "      );"

    "    }"

    "    else if (err.name === 'NotFoundError') {"

    "      statusEl.innerText = 'No camera found';"
    "      alert('No camera was found on this device.');"

    "    }"

    "    else {"

    "      statusEl.innerText = 'Camera unavailable';"
    "      alert('Unable to access camera: ' + err.message);"

    "    }"

    "  }"
    "}"


    /*
     * Capture current video frame and send JPEG
     * to ESP32 /predict.
     */
    "function captureAndPredict() {"

    "  if (!video.srcObject) {"
    "    alert('Please allow camera access first.');"
    "    return;"
    "  }"

    "  const btn = document.getElementById('predict-btn');"
    "  const confEl = document.getElementById('confidence-val');"

    "  btn.disabled = true;"

    "  statusEl.innerText = 'Analyzing...';"
    "  statusEl.className = 'status-value';"

    "  confEl.innerText = '--%';"


    /*
     * Model input is 64x64 RGB.
     */
    "  canvas.width = 64;"
    "  canvas.height = 64;"

    "  const ctx = canvas.getContext('2d');"

    "  const videoWidth = video.videoWidth || 320;"
    "  const videoHeight = video.videoHeight || 240;"


    /*
     * Full-frame resize.
     *
     * Current preprocessing:
     * 320x240 -> 64x64.
     */
    "  ctx.drawImage("
    "    video,"
    "    0,"
    "    0,"
    "    videoWidth,"
    "    videoHeight,"
    "    0,"
    "    0,"
    "    64,"
    "    64"
    "  );"


    /*
     * Convert frame to JPEG.
     */
    "  canvas.toBlob(blob => {"

    "    if (!blob) {"

    "      statusEl.innerText = 'Capture Error';"
    "      btn.disabled = false;"
    "      return;"

    "    }"


    /*
     * Relative URL.
     *
     * Since the page is loaded from:
     *
     * https://face-mask.local
     *
     * this becomes:
     *
     * https://face-mask.local/predict
     */
    "    fetch('/predict', {"

    "      method: 'POST',"

    "      headers: {"
    "        'Content-Type': 'image/jpeg'"
    "      },"

    "      body: blob"

    "    })"

    "    .then(res => {"

    "      if (!res.ok) {"
    "        throw new Error('HTTP status ' + res.status);"
    "      }"

    "      return res.json();"

    "    })"

    "    .then(data => {"

    "      statusEl.innerText = data.status || 'Unknown';"

    "      confEl.innerText ="
    "        (data.confidence !== undefined"
    "          ? Number(data.confidence).toFixed(1)"
    "          : '--')"
    "        + '%';"


    "      if (data.status === 'With Mask') {"

    "        statusEl.className ="
    "          'status-value mask-ok';"

    "      }"

    "      else if (data.status === 'No Mask') {"

    "        statusEl.className ="
    "          'status-value mask-none';"

    "      }"

    "      else {"

    "        statusEl.className ="
    "          'status-value mask-bad';"

    "      }"

    "    })"

    "    .catch(err => {"

    "      console.error('Fetch error:', err);"

    "      statusEl.innerText = 'Inference Error';"
    "      statusEl.className = 'status-value mask-none';"

    "    })"

    "    .finally(() => {"

    "      btn.disabled = false;"

    "    });"

    "  }, 'image/jpeg', 0.8);"

    "}"

    "</script>"

    "</body>"
    "</html>";


/*
 * GET /
 *
 * Serves the HTTPS web page.
 */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    /*
     * Allow camera access from this HTTPS origin.
     */
    httpd_resp_set_hdr(
        req,
        "Permissions-Policy",
        "camera=(self)"
    );

    return httpd_resp_send(
        req,
        HTML_PAGE,
        HTTPD_RESP_USE_STRLEN
    );
}


/*
 * POST /predict
 *
 * Receives JPEG frame from browser.
 */
static esp_err_t predict_post_handler(httpd_req_t *req)
{
    size_t total_len = req->content_len;

    if (total_len == 0 || total_len > 128 * 1024) {

        ESP_LOGE(
            TAG,
            "Invalid content length: %d",
            (int)total_len
        );

        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid frame size"
        );

        return ESP_FAIL;
    }


    /*
     * Allocate memory for incoming JPEG.
     */
    uint8_t *img_buf = malloc(total_len);

    if (!img_buf) {

        ESP_LOGE(
            TAG,
            "Failed to allocate %d bytes for image buffer",
            (int)total_len
        );

        httpd_resp_send_500(req);

        return ESP_FAIL;
    }


    /*
     * Receive complete JPEG.
     */
    size_t received = 0;

    while (received < total_len) {

        int ret = httpd_req_recv(
            req,
            (char *)(img_buf + received),
            total_len - received
        );

        if (ret <= 0) {

            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }

            ESP_LOGE(
                TAG,
                "Receive failed or socket closed, ret=%d",
                ret
            );

            free(img_buf);

            httpd_resp_send_500(req);

            return ESP_FAIL;
        }

        received += ret;
    }


    ESP_LOGI(
        TAG,
        "Received JPEG: %d bytes",
        (int)total_len
    );


    /*
     * Run AI inference.
     */
    ai_prediction_result_t result = {0};

    bool ok = ai_task_run_inference(
        img_buf,
        total_len,
        &result
    );


    /*
     * AI task has finished using JPEG buffer.
     */
    free(img_buf);


    if (!ok) {

        ESP_LOGE(
            TAG,
            "AI inference failed"
        );

        httpd_resp_send_500(req);

        return ESP_FAIL;
    }


    /*
     * Convert floating-point confidence to
     * integer + fractional part.
     *
     * Example:
     *
     * 93.8123 -> 93.81
     */
    int int_part = (int)result.confidence;

    int frac_part =
        (int)((result.confidence - int_part) * 100);

    if (frac_part < 0) {
        frac_part = -frac_part;
    }


    /*
     * JSON response.
     */
    char json_resp[128];

    snprintf(
        json_resp,
        sizeof(json_resp),
        "{\"status\":\"%s\",\"confidence\":%d.%02d}",
        result.status,
        int_part,
        frac_part
    );


    ESP_LOGI(
        TAG,
        "Prediction: %s (%.2f%%)",
        result.status,
        result.confidence
    );


    httpd_resp_set_type(
        req,
        "application/json"
    );

    return httpd_resp_send(
        req,
        json_resp,
        HTTPD_RESP_USE_STRLEN
    );
}


/*
 * Start HTTPS server.
 *
 * Server URL:
 *
 *     https://face-mask.local
 *
 * HTTPS port:
 *
 *     443
 */
void http_server_start(void)
{
    /*
     * HTTPS server configuration.
     */
    httpd_ssl_config_t config =
        HTTPD_SSL_CONFIG_DEFAULT();


    /*
     * HTTPS uses port 443.
     */
    config.httpd.server_port = 443;


    /*
     * Configure underlying HTTP server.
     */
    config.httpd.stack_size = 8192;
    config.httpd.max_uri_handlers = 8;


    /*
     * Embedded server certificate.
     */
    config.servercert =
        server_cert_pem_start;

    config.servercert_len =
        server_cert_pem_end -
        server_cert_pem_start;


    /*
     * Embedded private key.
     */
    config.prvtkey_pem =
        server_key_pem_start;

    config.prvtkey_len =
        server_key_pem_end -
        server_key_pem_start;


    httpd_handle_t server = NULL;


    /*
     * Start HTTPS server.
     */
    if (httpd_ssl_start(&server, &config) == ESP_OK) {

        /*
         * GET /
         */
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler
        };


        /*
         * POST /predict
         */
        httpd_uri_t predict_uri = {
            .uri = "/predict",
            .method = HTTP_POST,
            .handler = predict_post_handler
        };


        /*
         * Register URI handlers.
         */
        if (httpd_register_uri_handler(
                server,
                &root_uri
            ) != ESP_OK) {

            ESP_LOGE(
                TAG,
                "Failed to register / handler"
            );
        }


        if (httpd_register_uri_handler(
                server,
                &predict_uri
            ) != ESP_OK) {

            ESP_LOGE(
                TAG,
                "Failed to register /predict handler"
            );
        }


        ESP_LOGI(
            TAG,
            "HTTPS server started successfully"
        );

        ESP_LOGI(
            TAG,
            "Open: https://face-mask.local"
        );
    }
    else {

        ESP_LOGE(
            TAG,
            "Failed to start HTTPS server"
        );
    }
}