from PIL import Image
import sys

input_file = sys.argv[1]
output_file = sys.argv[2]

img = Image.open(input_file).convert("RGB")
img = img.resize((64, 64))

pixels = list(img.getdata())

with open(output_file, "w") as f:
    f.write('#include <stdint.h>\n\n')
    f.write('const uint8_t test_image[64 * 64 * 3] = {\n')

    for i, pixel in enumerate(pixels):
        r, g, b = pixel

        if i % 12 == 0:
            f.write("    ")

        f.write(f"{r}, {g}, {b}")

        if i != len(pixels) - 1:
            f.write(", ")

        if i % 12 == 11:
            f.write("\n")

    f.write('};\n')

print(f"Converted: {input_file}")
print("Output: 64x64 RGB")
print(f"Pixels: {len(pixels)}")
