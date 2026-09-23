import os

BASE_DIR = "dataset_split"

CLASSES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

SPLITS = [
    "train",
    "val",
    "test"
]

print("=" * 60)
print("ESP32 FACE MASK DATASET SPLIT CHECK")
print("=" * 60)

grand_total = 0

for split in SPLITS:

    print(f"\n{split.upper()}")
    print("-" * 40)

    split_total = 0

    for class_name in CLASSES:

        path = os.path.join(
            BASE_DIR,
            split,
            class_name
        )

        count = len([
            f for f in os.listdir(path)
            if f.lower().endswith(
                (".jpg", ".jpeg", ".png")
            )
        ])

        print(f"{class_name:20s}: {count}")

        split_total += count

    print(f"{'TOTAL':20s}: {split_total}")

    grand_total += split_total

print("\n" + "=" * 60)
print(f"GRAND TOTAL: {grand_total}")
print("=" * 60)
