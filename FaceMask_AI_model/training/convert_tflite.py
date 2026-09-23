import tensorflow as tf
import os

MODEL_PATH = "training/best_model.keras"
OUTPUT_PATH = "training/face_mask_model_fp32.tflite"

print("Loading Keras model...")

model = tf.keras.models.load_model(MODEL_PATH)

print("Model loaded.")

print("\nConverting to TensorFlow Lite...")

converter = tf.lite.TFLiteConverter.from_keras_model(model)

tflite_model = converter.convert()

with open(OUTPUT_PATH, "wb") as f:
    f.write(tflite_model)

size_kb = os.path.getsize(OUTPUT_PATH) / 1024

print("\nConversion complete.")
print(f"Output : {OUTPUT_PATH}")
print(f"Size   : {size_kb:.2f} KB")
