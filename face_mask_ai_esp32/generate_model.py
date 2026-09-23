from pathlib import Path

input_file = Path("model/face_mask_model_int8.tflite")
output_file = Path("main/model/model_data.cc")

data = input_file.read_bytes()

with output_file.open("w") as f:
    f.write('#include "model_data.h"\n\n')
    f.write("alignas(16) const unsigned char g_model[] = {\n")

    for i in range(0, len(data), 12):
        chunk = data[i:i + 12]
        f.write("    ")
        f.write(", ".join(f"0x{byte:02x}" for byte in chunk))
        f.write(",\n")

    f.write("};\n\n")
    f.write("const unsigned int g_model_len = sizeof(g_model);\n")

print(f"Generated: {output_file}")
print(f"Model size: {len(data)} bytes")