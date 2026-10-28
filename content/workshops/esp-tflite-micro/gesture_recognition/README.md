# ESP-TFLite-Micro gesture recognition workshop

This project accompanies the ESP-TFLite-Micro workshop. It runs on the
ESP-SensairShuttle (ESP32-C5 + BMI270) and follows the workshop from data
collection through four-class on-device inference.

The firmware starts with Espressif's supplied three-class model so the project
builds and runs before a participant has collected a new gesture:

| Index | Baseline class |
| ---: | --- |
| 0 | Counterclockwise circle |
| 1 | V |
| 2 | Unknown |

Assignments 1–3 add a participant-specific fourth class. No synthetic fourth
class is included: `new_gesture.csv` must contain real BMI270 captures from the
board.

## Source and toolchain

- Upstream example:
  `espressif/esp-dev-kits@4fad56d6b337281626c00ccd9a82403c54979179`
- Target: ESP32-C5
- ESP-IDF: 6.1.x
- ESP-TFLite-Micro: 1.3.4
- BMI270 Sensor: 0.4.0
- Button: 4.1.5
- Model training: Python 3.12 and TensorFlow 2.20.0

## Project structure

```text
gesture_recognition/
├── main/                         # ESP-IDF application
│   ├── app_bmi270.cpp
│   ├── app_btn.cpp
│   ├── app_model.cpp
│   ├── app_model_pipeline.cpp
│   ├── model.cpp                 # model currently compiled into firmware
│   └── include/
├── training/
│   ├── download_dataset.py
│   ├── capture_gesture.py
│   ├── train_model.py
│   ├── convert_model.py
│   ├── embed_model.py
│   ├── pretrained/model.tflite  # supplied three-class baseline
│   └── shuttle_dataset/
│       └── validate_dataset.py
├── CMakeLists.txt
└── sdkconfig.defaults
```

## Build the supplied baseline

Install and select ESP-IDF 6.1 with EIM, then run from this project directory:

```bash
eim install -i v6.1
eim select v6.1
eim run "idf.py set-target esp32c5"
eim run "idf.py build"
```

To flash and monitor, replace `<PORT>` with the ESP-SensairShuttle serial port:

```bash
eim run "idf.py -p <PORT> flash monitor"
```

The default mode performs continuous inference over a sliding 200-sample
window. Long-press and release Boot to switch between inference and collection
modes.

## Assignment 1: Collect a fourth class

In collection mode, briefly press Boot and immediately perform the gesture.
When the motion exceeds the trigger threshold, the firmware prints one row
between these markers:

```text
=== Data Collection Results ===
x0,y0,z0,...,x199,y199,z199
=== End of Data Collection ===
```

Save at least 25 valid rows as:

```text
training/shuttle_dataset/new_gesture.csv
```

The latest workshop script can capture the serial rows automatically:

```bash
cd training
python3.12 capture_gesture.py --count 25
```

Alternatively, copy each row between the collection markers manually.

Download the three supplied classes and validate the new class:

```bash
python3.12 download_dataset.py
cd shuttle_dataset
python3.12 validate_dataset.py
```

## Assignment 2: Train the four-class model

Use a separate terminal from the ESP-IDF environment:

```bash
cd training
python3.12 -m venv .venv
source .venv/bin/activate
python3.12 -m pip install --upgrade pip
python3.12 -m pip install -r requirements.txt
python3.12 train_model.py
```

The script loads all four CSV files, performs a repeatable 70/30 split, trains
the workshop's 1D CNN, prints test metrics, and creates:

- `simple_1dcnn_model.h5`

The script also contains an optional `plot_training_curves()` helper.

Class order is fixed:

| Index | Training file |
| ---: | --- |
| 0 | `o.csv` |
| 1 | `v.csv` |
| 2 | `unknown.csv` |
| 3 | `new_gesture.csv` |

## Assignment 3: Convert and embed the model

Convert the trained model and generate its C++ array:

```bash
python3.12 convert_model.py
python3.12 embed_model.py
cp model.cpp ../main/model.cpp
```

`convert_model.py` requires a float32 input tensor shaped `[1, 200, 3]` and a
float32 output tensor shaped `[1, 4]`. Review its printed operator list and
confirm that every operator is supported by the resolver in
`main/app_model.cpp` before building.

`embed_model.py` creates `training/model.cpp` with an aligned constant byte
array. Copy that generated source into `main/` as shown above.

## Assignment 4: Update labels and deploy

After embedding the four-class model, update `kLabels` in
`main/app_model.cpp`:

```cpp
static const char *kLabels[] = {
    "Counterclockwise circle",
    "V",
    "Unknown",
    "YOUR_GESTURE",
};
```

The firmware validates the model's input shape, type, and output count during
startup. A four-class model with only three labels is rejected instead of
indexing beyond the label array.

Rebuild and deploy:

```bash
eim run "idf.py build"
eim run "idf.py -p <PORT> flash monitor"
```

Perform at least ten trials for each class using the same board grip and
orientation used during data collection.

## Baseline model provenance

`training/pretrained/model.tflite` was reconstructed byte-for-byte from the
model embedded in the pinned upstream example. Verify it with:

```bash
cd training/pretrained
shasum -a 256 -c SHA256SUMS
```

The baseline contract is float32 `[1, 200, 3]` to float32 `[1, 3]`. The
firmware's `main/model.cpp` initially embeds this same model.

## Authoritative training scripts

The scripts under `training/` are synchronized from:

```text
https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/training.zip
SHA-256: f1327d4d5c6df93da258b4c178ca648df684115c5bdf3954e77bc2f0c85dcb12
```
