import os
import random
import matplotlib.pyplot as plt
from PIL import Image

DATASET_DIR = "dataset"

CLASSES = [
    "with_mask",
    "without_mask",
    "incorrect_mask"
]

SAMPLES_PER_CLASS = 5


def get_images(folder):
    files = []

    for filename in os.listdir(folder):
        filepath = os.path.join(folder, filename)

        if os.path.isfile(filepath):
            files.append(filepath)

    return files


def main():

    fig, axes = plt.subplots(
        len(CLASSES),
        SAMPLES_PER_CLASS,
        figsize=(15, 9)
    )

    for row, class_name in enumerate(CLASSES):

        folder = os.path.join(DATASET_DIR, class_name)

        images = get_images(folder)

        selected = random.sample(
            images,
            SAMPLES_PER_CLASS
        )

        for col, image_path in enumerate(selected):

            image = Image.open(image_path)

            axes[row, col].imshow(image)
            axes[row, col].axis("off")

            if col == 0:
                axes[row, col].set_title(
                    class_name,
                    fontsize=12
                )

    plt.tight_layout()

    plt.savefig(
        "dataset_samples.png",
        dpi=150
    )

    plt.show()


if __name__ == "__main__":
    main()

