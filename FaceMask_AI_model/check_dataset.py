import os
from PIL import Image

DATASET_DIR = "dataset"

CLASSES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

VALID_EXTENSIONS = {
    ".jpg",
    ".jpeg",
    ".png",
    ".bmp",
    ".webp"
}


def check_class(class_name):

    folder = os.path.join(DATASET_DIR, class_name)

    total = 0
    valid = 0
    corrupted = 0

    print(f"\nChecking: {class_name}")
    print("-" * 40)

    for filename in sorted(os.listdir(folder)):

        filepath = os.path.join(folder, filename)

        if not os.path.isfile(filepath):
            continue

        extension = os.path.splitext(filename)[1].lower()

        if extension not in VALID_EXTENSIONS:
            continue

        total += 1

        try:
            with Image.open(filepath) as img:
                img.verify()

            valid += 1

        except Exception as e:
            corrupted += 1
            print(f"Corrupted: {filepath}")
            print(f"Reason: {e}")

    print(f"Total images : {total}")
    print(f"Valid images : {valid}")
    print(f"Corrupted    : {corrupted}")

    return total, valid, corrupted


def main():

    print("=" * 50)
    print("ESP32 Face Mask Dataset Checker")
    print("=" * 50)

    total_images = 0
    total_corrupted = 0

    for class_name in CLASSES:

        total, valid, corrupted = check_class(class_name)

        total_images += total
        total_corrupted += corrupted

    print("\n" + "=" * 50)
    print("SUMMARY")
    print("=" * 50)

    print(f"Total images     : {total_images}")
    print(f"Corrupted images : {total_corrupted}")

    if total_corrupted == 0:
        print("\nDataset looks good!")
    else:
        print("\nWARNING: Remove/fix corrupted images before training.")


if __name__ == "__main__":
    main()
