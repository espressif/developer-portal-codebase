from pathlib import Path


model_data = Path("model.tflite").read_bytes()
rows = [
    ", ".join(f"0x{value:02x}" for value in model_data[offset:offset + 12])
    for offset in range(0, len(model_data), 12)
]
formatted_data = ",\n    ".join(rows)

Path("model.cpp").write_text(
    '#include "model.h"\n\n'
    "alignas(16) const unsigned char model_tflite[] = {\n"
    f"    {formatted_data}\n"
    "};\n\n"
    "const unsigned int model_tflite_len = sizeof(model_tflite);\n",
    encoding="utf-8",
)

print("Created model.cpp")
