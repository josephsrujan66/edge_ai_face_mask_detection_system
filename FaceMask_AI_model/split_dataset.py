import os
import random
import shutil

# ==============================
# Configuration
# ==============================

SOURCE_DIR = "dataset"
OUTPUT_DIR = "dataset_split"

CLASSES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

TRAIN_RATIO = 0.70
VAL_RATIO = 0.15
TEST_RATIO = 0.15

RANDOM_SEED = 42

random.seed(RANDOM_SEED)


# ==============================
# Create output directories
# ==============================

for split in ["train", "val", "test"]:
    for class_name in CLASSES:
        os.makedirs(
            os.path.join(OUTPUT_DIR, split, class_name),
            exist_ok=True
        )


# ==============================
# Split each class
# ==============================

total_images = 0

for class_name in CLASSES:

    source_class_dir = os.path.join(SOURCE_DIR, class_name)

    images = [
        f for f in os.listdir(source_class_dir)
        if f.lower().endswith((".jpg", ".jpeg", ".png"))
    ]

    random.shuffle(images)

    total = len(images)

    train_end = int(total * TRAIN_RATIO)
    val_end = train_end + int(total * VAL_RATIO)

    train_images = images[:train_end]
    val_images = images[train_end:val_end]
    test_images = images[val_end:]

    splits = {
        "train": train_images,
        "val": val_images,
        "test": test_images
    }

    print(f"\nClass: {class_name}")
    print(f"Total : {total}")
    print(f"Train : {len(train_images)}")
    print(f"Val   : {len(val_images)}")
    print(f"Test  : {len(test_images)}")

    for split_name, split_images in splits.items():

        destination_dir = os.path.join(
            OUTPUT_DIR,
            split_name,
            class_name
        )

        for image_name in split_images:

            source_path = os.path.join(
                source_class_dir,
                image_name
            )

            destination_path = os.path.join(
                destination_dir,
                image_name
            )

            shutil.copy2(
                source_path,
                destination_path
            )

    total_images += total


# ==============================
# Summary
# ==============================

print("\n" + "=" * 50)
print("DATASET SPLIT COMPLETE")
print("=" * 50)

for split in ["train", "val", "test"]:

    split_total = 0

    for class_name in CLASSES:

        class_dir = os.path.join(
            OUTPUT_DIR,
            split,
            class_name
        )

        count = len([
            f for f in os.listdir(class_dir)
            if f.lower().endswith((".jpg", ".jpeg", ".png"))
        ])

        split_total += count

    print(f"{split:>5} : {split_total}")

print(f"{'Total':>5} : {total_images}")
