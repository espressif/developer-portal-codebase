# Supplied baseline model

`model.tflite` is the three-class float32 model embedded in the pinned upstream
ESP-SensairShuttle gesture-recognition example.

Model contract:

- Input: `float32 [1, 200, 3]`
- Output: `float32 [1, 3]`
- Labels: `Counterclockwise circle`, `V`, `Unknown`
- Upstream revision: `4fad56d6b337281626c00ccd9a82403c54979179`

`SHA256SUMS` records the exact model checksum. The firmware starts with this
model so it can be built and run before the participant collects a fourth
class. Assignments 1–3 produce a participant-specific four-class model that
replaces `main/model.cpp`.
