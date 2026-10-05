# ESP32 Edge AI Face Mask Detection System

An embedded Edge AI face-mask detection system using an **ESP32-WROOM-32**, **TensorFlow Lite Micro**, and a browser-based camera interface.

The project trains a lightweight CNN on a PC, converts it to an INT8 TensorFlow Lite model, deploys the model to the ESP32, and performs local inference on images captured from a phone/laptop browser.

## Project Overview

The complete system is divided into two main parts:

```text
edge_ai_face_mask_detection_system/
│
├── FaceMask_AI_model/
│   ├── Dataset preparation
│   ├── Model training
│   ├── TFLite conversion
│   ├── INT8 quantization
│   ├── Evaluation
│   └── ESP32 model/image generation tools
│
├── face_mask_ai_esp32/
│   ├── ESP-IDF firmware
│   ├── Wi-Fi
│   ├── HTTPS web server
│   ├── JPEG decoding
│   ├── TensorFlow Lite Micro
│   └── AI inference
│
└── README.md
```

## System Architecture

```text
                   Phone / Laptop
                        │
                        │ Camera
                                    ▼
                 ┌──────────────┐
                 │ Web Browser  │
                 └──────┬───────┘
                        │
                        │ Wi-Fi / HTTPS
                        │ JPEG
                                    ▼
              ┌──────────────────────┐
              │        ESP32         │
              │                      │
              │   HTTPS Web Server   │
              │          │           │
              │          ▼           │
              │    JPEG Decoder      │
              │          │           │
              │          ▼           │
              │  Image Preprocessing │
              │          │           │
              │          ▼           │
              │ TensorFlow Lite Micro│
              │       INT8 CNN       │
              │          │           │
              │          ▼           │
              │     Prediction       │
              └──────────┬───────────┘
                         │
                                     ▼
                     JSON Result
```

## Detection Classes

The model classifies images into three categories:

- `with_mask`
- `without_mask`
- `incorrect_mask`

---

# Repository Directory Structure

The GitHub repository is organized as follows:

```text
edge_ai_face_mask_detection_system/
│
├── FaceMask_AI_model/
│   ├── check_dataset.py
│   ├── check_split.py
│   ├── dataset_samples.png
│   ├── dataset_split/
│   │   ├── train/
│   │   ├── val/
│   │   └── test/
│   │
│   ├── merge_dataset.py
│   ├── split_dataset.py
│   ├── visualize_dataset.py
│   │
│   ├── tools/
│   │   ├── convert_test_image.py
│   │   ├── image.jpg
│   │   └── test_image.cc
│   │
│   └── training/
│       ├── best_model.keras
│       ├── convert_int8.py
│       ├── evaluate_int8.py
│       ├── face_mask_model_fp32.tflite
│       ├── face_mask_model_int8.tflite
│       ├── confusion_matrix.png
│       ├── convert_tflite.py
│       ├── evaluate.py
│       ├── final_model.keras
│       └── train.py
│
├── face_mask_ai_esp32/
│   ├── main/
│   ├── CMakeLists.txt
│   └── ...
│
└── README.md
```

---

# 1. FaceMask_AI_model

The `FaceMask_AI_model` directory contains everything required on the **host/PC side** for dataset preparation, training, evaluation, model conversion, and generation of files used by the ESP32 firmware.

Move into the directory:

```bash
cd FaceMask_AI_model/
```

Expected location:

```text
~/Documents/edge_ai_face_mask_detection_system/FaceMask_AI_model/
```

Check the directory:

```bash
ls
```

Expected contents:

```text
check_dataset.py
check_split.py
dataset_samples.png
dataset_split
merge_dataset.py
split_dataset.py
tools
training
visualize_dataset.py
```

---

# 2. Dataset Preparation

The dataset preparation scripts are located directly under:

```text
FaceMask_AI_model/
```

### Dataset checking

```bash
python check_dataset.py
```

### Dataset merging

```bash
python merge_dataset.py
```

### Dataset splitting

```bash
python split_dataset.py
```

The resulting dataset is organized as:

```text
dataset_split/
├── train/
├── val/
└── test/
```

The split is used for model training, validation, and final testing.

---

# 3. Dataset Visualization

The project contains:

```text
visualize_dataset.py
dataset_samples.png
```

Run:

```bash
python visualize_dataset.py
```

This can be used to inspect the prepared dataset and verify the image classes before training.

---

# 4. Model Training

Training scripts are located in:

```text
FaceMask_AI_model/training/
```

Directory:

```text
training/
├── train.py
├── best_model.keras
├── final_model.keras
├── evaluate.py
├── convert_tflite.py
├── convert_int8.py
├── evaluate_int8.py
├── face_mask_model_fp32.tflite
├── face_mask_model_int8.tflite
└── confusion_matrix.png
```

Run training from:

```bash
cd FaceMask_AI_model/training/
```

Then:

```bash
python train.py
```

The trained Keras model is generated/saved as:

```text
best_model.keras
final_model.keras
```

---

# 5. TensorFlow Lite Conversion

The trained Keras model is converted to TensorFlow Lite.

Run:

```bash
python convert_tflite.py
```

This generates:

```text
face_mask_model_fp32.tflite
```

The FP32 model is used for TensorFlow Lite evaluation before quantization.

---

# 6. INT8 Quantization

The final ESP32 model uses full INT8 quantization.

Run:

```bash
python convert_int8.py
```

The resulting model is:

```text
face_mask_model_int8.tflite
```

This is the model deployed to the ESP32.

---

# 7. Model Evaluation

Evaluate the FP32 model:

```bash
python evaluate.py
```

Evaluate the INT8 model:

```bash
python evaluate_int8.py
```

The project also contains:

```text
confusion_matrix.png
```

for evaluating classification performance.

---

# 8. Model Performance

The final INT8 model achieved approximately:

```text
Test Accuracy: 93.72%
```

The FP32 model achieved approximately:

```text
Test Accuracy: 93.81%
```

Model sizes:

```text
FP32: ~104.94 KB
INT8: ~33.90 KB
```

The INT8 model significantly reduces model size while maintaining nearly the same classification accuracy.

---

# 9. ESP32 Test Image Generation

The `tools` directory contains utilities used to prepare an image for ESP32-side testing.

```text
tools/
├── convert_test_image.py
├── image.jpg
└── test_image.cc
```

The input image is:

```text
image.jpg
```

The generated C/C++ representation is:

```text
test_image.cc
```

Run:

```bash
cd FaceMask_AI_model/tools/
python convert_test_image.py
```

The generated image data can then be copied into the ESP32 firmware project as required.

---

# 10. ESP32 Firmware

The ESP32 firmware is located in:

```text
face_mask_ai_esp32/
```

This is an **ESP-IDF** project.

The firmware performs:

```text
Wi-Fi
  ↓
HTTPS Web Server
  ↓
JPEG Reception
  ↓
JPEG Decode
  ↓
RGB888 Image
  ↓
64 × 64 Preprocessing
  ↓
INT8 Tensor
  ↓
TensorFlow Lite Micro
  ↓
Inference
  ↓
Prediction
  ↓
JSON Response
```

---

# 11. ESP32 Software Requirements

The ESP32 firmware requires:

- ESP-IDF 5.5.x
- ESP32 target
- TensorFlow Lite Micro
- ESP JPEG decoder
- ESP HTTPS server
- Wi-Fi
- FreeRTOS
- mDNS

The project was developed using:

```text
ESP-IDF 5.5.5
Target: ESP32
```

---

# 12. ESP32 Project Setup

Open an ESP-IDF terminal and move to the ESP32 project:

```bash
cd face_mask_ai_esp32
```

Set the ESP32 target:

```bash
idf.py set-target esp32
```

Build the project:

```bash
idf.py build
```

If required, perform a clean build:

```bash
idf.py fullclean
idf.py build
```

---

# 13. Flash the ESP32

Connect the ESP32-WROOM-32 through USB.

Flash the firmware:

```bash
idf.py flash
```

Flash and open the serial monitor:

```bash
idf.py flash monitor
```

Or monitor an already flashed device:

```bash
idf.py monitor
```

---

# 14. ESP32 AI Model

The INT8 model generated on the host side:

```text
FaceMask_AI_model/training/face_mask_model_int8.tflite
```

is converted into a C/C++ source representation for the ESP32 firmware.

The firmware contains the model as:

```text
model_data.cc
model_data.h
```

The model is stored in ESP32 flash and loaded by TensorFlow Lite Micro at runtime.

---

# 15. TensorFlow Lite Micro

The ESP32 uses TensorFlow Lite Micro for inference.

The model input is:

```text
1 × 64 × 64 × 3
```

Input type:

```text
INT8
```

The ESP32 uses a tensor arena of approximately:

```text
96 KB
```

The project is designed for the ESP32-WROOM-32 without external PSRAM.

---

# 16. Browser Interface

The ESP32 hosts a web interface that provides:

- Camera preview
- Camera permission request
- Predict button
- Prediction result
- Confidence value

The browser uses:

```javascript
navigator.mediaDevices.getUserMedia()
```

to access the camera.

The user explicitly presses:

```text
Allow Camera
```

before camera access is requested.

---

# 17. HTTPS

The browser camera API requires a secure context in modern browsers.

Therefore, the ESP32 web server is configured for HTTPS.

The development server uses:

```text
Port: 443
```

The project uses a development certificate for:

```text
face-mask.local
```

Certificate files are stored under:

```text
certs/
├── cert.cnf
├── server_cert.pem
└── server_key.pem
```

The private key should not be committed to the public repository.

Add to `.gitignore`:

```gitignore
certs/server_key.pem
```

---

# 18. mDNS

mDNS provides a friendly hostname instead of requiring the user to remember the ESP32 IP address.

The intended address is:

```text
https://face-mask.local
```

The ESP32 advertises the HTTPS service on:

```text
TCP 443
```

If mDNS is unavailable on the client network, the ESP32 IP address can be used as a fallback.

---

# 19. Prediction API

The browser sends an image to:

```text
POST /predict
```

The ESP32 receives the JPEG image and performs inference.

Example response:

```json
{
    "status": "With Mask",
    "confidence": 95.32
}
```

Possible classification results include:

```text
With Mask
Without Mask
Incorrect Mask
```

---

# 20. AI Inference Pipeline

```text
Browser Camera
      │
         ▼
Capture Frame
      │
         ▼
Convert to JPEG
      │
         ▼
  POST/predict
      │
         ▼
    ESP32
      │
         ▼
 JPEG Decoder
      │
         ▼
    RGB888
      │
         ▼
64 × 64 × 3
      │
         ▼
INT8 Quantization
      │
         ▼
TensorFlow Lite Micro
      │
         ▼
CNN Inference
      │
         ▼
Class + Confidence
      │
        ▼
    JSON
      │
        ▼
   Browser
```

---

# 21. Complete Host-to-ESP32 Workflow

The complete development workflow is:

```text
Dataset
   │
    ▼
Dataset Preparation
   │
    ▼
Train CNN
   │
    ▼
Evaluate FP32
   │
    ▼
Convert to TFLite
   │
    ▼
INT8 Quantization
   │
    ▼
Evaluate INT8
   │
    ▼
Generate ESP32 Model Files
   │
    ▼
Copy/Update ESP32 Firmware
   │
    ▼
ESP-IDF Build
   │
    ▼
Flash ESP32
   │
    ▼
Open Browser
   │
    ▼
Camera
   │
    ▼
ESP32 Inference
```

---

# 22. Host-Side Commands

From the repository root:

```bash
cd FaceMask_AI_model
```

Dataset preparation:

```bash
python check_dataset.py
python merge_dataset.py
python split_dataset.py
python visualize_dataset.py
```

Training:

```bash
cd training
python train.py
python convert_tflite.py
python convert_int8.py
python evaluate.py
python evaluate_int8.py
```

Generate ESP32 test image:

```bash
cd ../tools
python convert_test_image.py
```

---

# 23. ESP32-Side Commands

From the ESP32 project:

```bash
cd face_mask_ai_esp32
```

Configure:

```bash
idf.py set-target esp32
```

Build:

```bash
idf.py build
```

Flash:

```bash
idf.py flash
```

Flash and monitor:

```bash
idf.py flash monitor
```

---

# 24. Hardware

Required hardware:

 ESP32-WROOM-32 for Edge AI inference
 USB cable for Programming and power
 Phone/Laptop for Camera and browser 

No external camera module is required.

---

# 25. Why Edge AI?

A conventional implementation could send the image to a cloud server:

```text
Camera
   ↓
Internet
   ↓
Cloud AI
   ↓
Prediction
```

This project instead performs inference locally:

```text
Camera
   ↓
Browser
   ↓
ESP32
   ↓
Local AI
   ↓
Prediction
```

Benefits include:

- Reduced cloud dependency
- Reduced bandwidth
- Local processing
- Better privacy
- Demonstration of embedded AI
- Suitable for IoT/edge applications

---

# 26. Technologies Used

### Embedded

- ESP32-WROOM-32
- ESP-IDF
- FreeRTOS
- C/C++

### AI/ML

- Python
- TensorFlow
- TensorFlow Lite
- TensorFlow Lite Micro
- CNN
- INT8 quantization

### Networking

- Wi-Fi
- HTTP/HTTPS
- JSON
- mDNS

### Image Processing

- JPEG
- RGB888
- Image resizing
- INT8 preprocessing

---




# 27. Limitations

This project is intended as an embedded AI demonstration and portfolio project.

Prediction performance can be affected by:

- Lighting
- Camera quality
- Distance from camera
- Face position
- Mask type
- Occlusion
- Image quality
- Dataset characteristics

It is not intended to be used as a certified safety or medical system.

---

# 38. Author

**Joseph Srujan**

Embedded Software / Edge AI

Technologies:

```text
C
C++
Python
ESP32
ESP-IDF
FreeRTOS
TensorFlow
TensorFlow Lite
TensorFlow Lite Micro
Wi-Fi
HTTP/HTTPS
mDNS
JPEG
JSON
```

---
