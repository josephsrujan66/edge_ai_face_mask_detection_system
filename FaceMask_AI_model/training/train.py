import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

# ============================================================
# Configuration
# ============================================================

DATASET_DIR = "dataset_split"

IMG_SIZE = (64, 64)
BATCH_SIZE = 32
EPOCHS = 20

SEED = 42

CLASS_NAMES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

# ============================================================
# Load datasets
# ============================================================

train_ds = tf.keras.utils.image_dataset_from_directory(
    f"{DATASET_DIR}/train",
    labels="inferred",
    label_mode="int",
    class_names=CLASS_NAMES,
    image_size=IMG_SIZE,
    batch_size=BATCH_SIZE,
    shuffle=True,
    seed=SEED
)

val_ds = tf.keras.utils.image_dataset_from_directory(
    f"{DATASET_DIR}/val",
    labels="inferred",
    label_mode="int",
    class_names=CLASS_NAMES,
    image_size=IMG_SIZE,
    batch_size=BATCH_SIZE,
    shuffle=False
)

print("\nClass names:")
print(train_ds.class_names)

# ============================================================
# Performance optimization
# ============================================================

AUTOTUNE = tf.data.AUTOTUNE

train_ds = train_ds.prefetch(AUTOTUNE)
val_ds = val_ds.prefetch(AUTOTUNE)

# ============================================================
# Data preprocessing
# ============================================================

normalization = layers.Rescaling(1.0 / 255)

# ============================================================
# Tiny CNN
# ============================================================

model = keras.Sequential([
    
    layers.Input(shape=(64, 64, 3)),

    # Normalize pixels from [0,255] -> [0,1]
    normalization,

    # Block 1
    layers.Conv2D(
        16,
        (3, 3),
        activation="relu",
        padding="same"
    ),
    layers.MaxPooling2D(),

    # Block 2
    layers.Conv2D(
        32,
        (3, 3),
        activation="relu",
        padding="same"
    ),
    layers.MaxPooling2D(),

    # Block 3
    layers.Conv2D(
        64,
        (3, 3),
        activation="relu",
        padding="same"
    ),

    # Reduce feature maps
    layers.GlobalAveragePooling2D(),

    # Small fully-connected layer
    layers.Dense(
        32,
        activation="relu"
    ),

    # 3 output classes
    layers.Dense(
        3,
        activation="softmax"
    )
])

# ============================================================
# Model information
# ============================================================

model.summary()

# ============================================================
# Compile
# ============================================================

model.compile(
    optimizer=keras.optimizers.Adam(
        learning_rate=0.001
    ),
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"]
)

# ============================================================
# Callbacks
# ============================================================

callbacks = [

    keras.callbacks.EarlyStopping(
        monitor="val_loss",
        patience=4,
        restore_best_weights=True
    ),

    keras.callbacks.ModelCheckpoint(
        "training/best_model.keras",
        monitor="val_accuracy",
        save_best_only=True
    )
]

# ============================================================
# Train
# ============================================================

print("\nStarting training...\n")

history = model.fit(
    train_ds,
    validation_data=val_ds,
    epochs=EPOCHS,
    callbacks=callbacks
)

# ============================================================
# Save final model
# ============================================================

model.save("training/final_model.keras")

print("\nTraining complete.")
print("Best model : training/best_model.keras")
print("Final model: training/final_model.keras")
