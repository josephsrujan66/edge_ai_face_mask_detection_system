import tensorflow as tf
import numpy as np
from sklearn.metrics import classification_report, confusion_matrix

# ============================================================
# Configuration
# ============================================================

MODEL_PATH = "training/face_mask_model_int8.tflite"
TEST_DIR = "dataset_split/test"

IMG_SIZE = (64, 64)
BATCH_SIZE = 32

CLASS_NAMES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

# ============================================================
# Load test dataset
# ============================================================

test_ds = tf.keras.utils.image_dataset_from_directory(
    TEST_DIR,
    labels="inferred",
    label_mode="int",
    class_names=CLASS_NAMES,
    image_size=IMG_SIZE,
    batch_size=BATCH_SIZE,
    shuffle=False
)

# ============================================================
# Load TFLite model
# ============================================================

interpreter = tf.lite.Interpreter(
    model_path=MODEL_PATH
)

interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("\nInput details:")
print(input_details)

print("\nOutput details:")
print(output_details)

input_scale, input_zero_point = (
    input_details[0]["quantization"]
)

output_scale, output_zero_point = (
    output_details[0]["quantization"]
)

print("\nQuantization:")
print(f"Input scale      : {input_scale}")
print(f"Input zero point : {input_zero_point}")
print(f"Output scale     : {output_scale}")
print(f"Output zero point: {output_zero_point}")

# ============================================================
# Run inference
# ============================================================

y_true = []
y_pred = []

for images, labels in test_ds:

    images = images.numpy()
    labels = labels.numpy()

    for i in range(len(images)):

        image = images[i]

        # ----------------------------------------------------
        # Quantize float image to INT8
        # ----------------------------------------------------

        quantized_image = (
            image / input_scale
            + input_zero_point
        )

        quantized_image = np.clip(
            quantized_image,
            -128,
            127
        ).astype(np.int8)

        quantized_image = np.expand_dims(
            quantized_image,
            axis=0
        )

        # ----------------------------------------------------
        # Run TFLite inference
        # ----------------------------------------------------

        interpreter.set_tensor(
            input_details[0]["index"],
            quantized_image
        )

        interpreter.invoke()

        output = interpreter.get_tensor(
            output_details[0]["index"]
        )

        # ----------------------------------------------------
        # Convert output back to float
        # ----------------------------------------------------

        output = (
            output.astype(np.float32)
            - output_zero_point
        ) * output_scale

        predicted_class = np.argmax(
            output[0]
        )

        y_true.append(
            labels[i]
        )

        y_pred.append(
            predicted_class
        )

# ============================================================
# Results
# ============================================================

y_true = np.array(y_true)
y_pred = np.array(y_pred)

accuracy = np.mean(
    y_true == y_pred
)

print("\n" + "=" * 55)
print("INT8 TEST RESULTS")
print("=" * 55)

print(
    f"Test Accuracy : {accuracy * 100:.2f}%"
)

# ============================================================
# Classification report
# ============================================================

print("\n" + "=" * 55)
print("INT8 CLASSIFICATION REPORT")
print("=" * 55)

print(
    classification_report(
        y_true,
        y_pred,
        target_names=CLASS_NAMES,
        digits=4
    )
)

# ============================================================
# Confusion matrix
# ============================================================

cm = confusion_matrix(
    y_true,
    y_pred
)

print("\n" + "=" * 55)
print("INT8 CONFUSION MATRIX")
print("=" * 55)

print(cm)
