import tensorflow as tf
import numpy as np
from sklearn.metrics import (
    confusion_matrix,
    classification_report
)
import matplotlib.pyplot as plt

# ============================================================
# Configuration
# ============================================================

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
# Load best trained model
# ============================================================

model = tf.keras.models.load_model(
    "training/best_model.keras"
)

print("\nModel loaded successfully.")

# ============================================================
# Evaluate
# ============================================================

loss, accuracy = model.evaluate(test_ds)

print("\n" + "=" * 50)
print("TEST RESULTS")
print("=" * 50)

print(f"Test Loss     : {loss:.4f}")
print(f"Test Accuracy : {accuracy * 100:.2f}%")

# ============================================================
# Generate predictions
# ============================================================

y_true = []
y_pred = []

for images, labels in test_ds:

    predictions = model.predict(
        images,
        verbose=0
    )

    predicted_classes = np.argmax(
        predictions,
        axis=1
    )

    y_true.extend(labels.numpy())
    y_pred.extend(predicted_classes)

y_true = np.array(y_true)
y_pred = np.array(y_pred)

# ============================================================
# Classification report
# ============================================================

print("\n" + "=" * 50)
print("CLASSIFICATION REPORT")
print("=" * 50)

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

print("\n" + "=" * 50)
print("CONFUSION MATRIX")
print("=" * 50)

print(cm)

# ============================================================
# Plot confusion matrix
# ============================================================

plt.figure(figsize=(7, 6))

plt.imshow(cm)

plt.title("Face Mask Classification - Confusion Matrix")
plt.colorbar()

plt.xticks(
    range(len(CLASS_NAMES)),
    CLASS_NAMES,
    rotation=30
)

plt.yticks(
    range(len(CLASS_NAMES)),
    CLASS_NAMES
)

plt.xlabel("Predicted")
plt.ylabel("Actual")

# Add numbers
for i in range(len(CLASS_NAMES)):
    for j in range(len(CLASS_NAMES)):

        plt.text(
            j,
            i,
            cm[i, j],
            ha="center",
            va="center"
        )

plt.tight_layout()

plt.savefig(
    "training/confusion_matrix.png",
    dpi=150
)

print("\nConfusion matrix saved to:")
print("training/confusion_matrix.png")
