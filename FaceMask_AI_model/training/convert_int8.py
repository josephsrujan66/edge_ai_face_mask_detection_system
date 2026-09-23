import tensorflow as tf
import numpy as np
import os

# ============================================================
# Configuration
# ============================================================

MODEL_PATH = "training/best_model.keras"
OUTPUT_PATH = "training/face_mask_model_int8.tflite"

REPRESENTATIVE_DIR = "dataset_split/train"

IMG_SIZE = (64, 64)

CLASS_NAMES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

REPRESENTATIVE_IMAGES = 300

# ============================================================
# Load model
# ============================================================

print("Loading Keras model...")

model = tf.keras.models.load_model(
    MODEL_PATH
)

print("Model loaded.")

# ============================================================
# Collect representative images
# ============================================================

image_paths = []

for class_name in CLASS_NAMES:

    class_dir = os.path.join(
        REPRESENTATIVE_DIR,
        class_name
    )

    for filename in os.listdir(class_dir):

        if filename.lower().endswith(
            (".jpg", ".jpeg", ".png")
        ):
            image_paths.append(
                os.path.join(
                    class_dir,
                    filename
                )
            )

# Shuffle and select representative samples
np.random.seed(42)

np.random.shuffle(image_paths)

image_paths = image_paths[:REPRESENTATIVE_IMAGES]

print(
    f"Using {len(image_paths)} representative images."
)

# ============================================================
# Representative dataset generator
# ============================================================

def representative_dataset():

    for image_path in image_paths:

        image = tf.keras.utils.load_img(
            image_path,
            target_size=IMG_SIZE
        )

        image = tf.keras.utils.img_to_array(
            image
        )

        # IMPORTANT:
        # The model expects raw [0,255] input because
        # Rescaling(1/255) is already inside the model.
        image = np.expand_dims(
            image,
            axis=0
        ).astype(np.float32)

        yield [image]


# ============================================================
# Convert
# ============================================================

print("\nStarting INT8 quantization...")

converter = tf.lite.TFLiteConverter.from_keras_model(
    model
)

converter.optimizations = [
    tf.lite.Optimize.DEFAULT
]

converter.representative_dataset = (
    representative_dataset
)

# Force integer operations
converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS_INT8
]

# INT8 input/output
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# ============================================================
# Save
# ============================================================

with open(
    OUTPUT_PATH,
    "wb"
) as f:

    f.write(tflite_model)

size_kb = (
    os.path.getsize(OUTPUT_PATH) / 1024
)

print("\n" + "=" * 50)
print("INT8 CONVERSION COMPLETE")
print("=" * 50)

print(f"Output : {OUTPUT_PATH}")
print(f"Size   : {size_kb:.2f} KB")	
