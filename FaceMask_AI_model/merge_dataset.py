import os
import shutil
import re

# Dataset root
DATASET_DIR = os.path.expanduser(
    "~/Documents/ESP32_FaceMask_AI/dataset"
)

# Source folders -> destination class folder
MERGE_MAP = {
    "with_mask": ["complex", "simple"],
    "without_mask": ["complex", "simple"],
    "incorrect_mask": ["mc", "mmc"],
}

# Supported image extensions
IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}


def get_last_number(folder):
    """
    Find the highest existing image number in the destination folder.
    Expected names:
        image_00001.jpg
        image_00002.jpg
        ...
    """

    highest = 0

    if not os.path.exists(folder):
        return highest

    pattern = re.compile(r"image_(\d+)\.", re.IGNORECASE)

    for filename in os.listdir(folder):
        match = pattern.match(filename)

        if match:
            number = int(match.group(1))
            highest = max(highest, number)

    return highest


def merge_class(class_name, source_folders):
    destination = os.path.join(DATASET_DIR, class_name)

    os.makedirs(destination, exist_ok=True)

    # Continue from the last existing image number
    count = get_last_number(destination)

    print(f"\n[{class_name}]")
    print(f"Starting number: {count + 1}")

    total_added = 0

    for source in source_folders:

        source_path = os.path.join(destination, source)

        if not os.path.exists(source_path):
            print(f"WARNING: {source_path} does not exist")
            continue

        print(f"Reading: {source_path}")

        for filename in sorted(os.listdir(source_path)):

            source_file = os.path.join(source_path, filename)

            if not os.path.isfile(source_file):
                continue

            extension = os.path.splitext(filename)[1].lower()

            if extension not in IMAGE_EXTENSIONS:
                continue

            count += 1

            new_filename = f"image_{count:05d}{extension}"

            destination_file = os.path.join(
                destination,
                new_filename
            )

            shutil.move(source_file, destination_file)

            total_added += 1

    print(f"Added: {total_added} images")
    print(f"Last number: {count}")


def main():

    print("======================================")
    print(" ESP32 Face Mask Dataset Merger")
    print("======================================")

    for class_name, source_folders in MERGE_MAP.items():
        merge_class(class_name, source_folders)

    print("\nDataset merge completed.")


if __name__ == "__main__":
    main()
