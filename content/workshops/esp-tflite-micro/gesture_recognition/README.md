# Gesture Recognition 

This is an edge-side gesture recognition project based on TensorFlow Lite Micro. By collecting BMI270 data from the Sensairshuttle onboard sensor, it achieves local recognition of counterclockwise circles and V gestures.

| Counterclockwise circle | V gesture |
|:---:|:---:|
| ![counterclockwise](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/counterclockwise.gif) | ![v](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/v.gif) |

In this example, we will demonstrate how to collect data, build model training, and deploy.

## Data Collection Flow

Long press and release the Boot button to enter data collection mode. In data collection mode, each press of the Boot button will trigger data collection when the sum of absolute values of three-axis angular velocity exceeds the trigger threshold. Therefore, please complete the gesture immediately after pressing the button.

![Data Collection](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/data_collect.png)

You can organize the data for each gesture type into separate txt files and use a Python script to convert them into CSV files. You can download the dataset used in this example [here](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/shuttle_dataset.zip).

Next, you can refer to the following script to split the dataset for subsequent training:

```python
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split


def extract_data():
    # Read data files
    o_data = pd.read_csv('./o.csv', sep=',', header=None)
    v_data = pd.read_csv('./v.csv', sep=',', header=None)
    unknown_data = pd.read_csv('./unknown.csv', sep=',', header=None)

    # Create labels
    o_label = np.zeros(o_data.shape[0], dtype=int)
    v_label = np.ones(v_data.shape[0], dtype=int)
    unknown_label = np.full(unknown_data.shape[0], 2, dtype=int)

    # Combine feature data and labels (use .values for all to ensure numpy arrays)
    X_raw = np.vstack([o_data.values, v_data.values, unknown_data.values])
    y = np.concatenate([o_label, v_label, unknown_label])

    # Display the number of samples for each class
    print(f"o samples: {o_label.shape[0]}")
    print(f"v samples: {v_label.shape[0]}")
    print(f"unknown samples: {unknown_label.shape[0]}")

    print(f"Total samples: {len(y)}")

    num_samples = X_raw.shape[0]
    num_timesteps = 200  # 200 timesteps
    num_axes = 3  # x, y, z three axes

    # Reshape data: reshape 600 data points in each row into 200x3 matrix
    X = X_raw.reshape(num_samples, num_timesteps, num_axes)

    return X, y


if __name__ == "__main__":
    X, y = extract_data()

    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.int32)

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)
```

## AI Processing Flow

#### Model Training

Based on the TensorFlow framework, we build the following neural network model:

```python
from tensorflow import keras

model = keras.Sequential([
    keras.layers.Conv1D(filters=8, kernel_size=5, padding='same', activation='relu', input_shape=(200, 3)),
    keras.layers.MaxPooling1D(pool_size=4),

    keras.layers.Conv1D(filters=16, kernel_size=5, padding='same', activation='relu'),
    keras.layers.MaxPooling1D(pool_size=4),

    keras.layers.GlobalAveragePooling1D(),

    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dropout(0.2),

    # Output layer
    keras.layers.Dense(3, activation='softmax')
])

model.compile(
    optimizer=keras.optimizers.Adam(learning_rate=0.001),
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

history = model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=300,
    batch_size=64,
    verbose=1,
    shuffle=True
)
```

![Training Curves](https://dl.espressif.com/AE/esp-iot-solution/gesture_recognition/training_curves.png)

#### Model Quantization and Deployment

```
import tensorflow as tf
from tensorflow import keras

model = keras.models.load_model(model_path)
converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.float32]
converter.inference_input_type = tf.float32
converter.inference_output_type = tf.float32

tflite_model = converter.convert()

with open(output_path, "wb") as f:
    f.write(tflite_model)
```

To run on the edge side, the tflite file needs to be converted to C++:

```
xxd -i model.tflite > model.cpp
```

## Example output:

The firmware uses the Boot button (GPIO28) to control the current operating mode by default, including data collection mode and automatic inference mode. The default mode is automatic inference mode. You can long-press and release the Boot button to switch modes:

* **Collection Mode**: Press the Boot button to start collection. When the sum of absolute values of three-axis angular velocity exceeds the set threshold, all sampled values will be printed.
* **Automatic Inference Mode**: No button press required. When the sum of absolute values of three-axis angular velocity exceeds the set threshold, inference will be performed automatically.

```
 (21) boot: ESP-IDF v5.5.2-737-g0da66850d91 2nd stage bootloader
I (21) boot: compile time Feb 11 2026 17:25:25
I (23) boot: chip revision: v1.0
I (23) boot: efuse block revision: v0.3
I (26) qio_mode: Enabling default flash chip QIO
I (31) boot.esp32c5: SPI Speed      : 80MHz
I (34) boot.esp32c5: SPI Mode       : QIO
I (38) boot.esp32c5: SPI Flash Size : 8MB
I (42) boot: Enabling RNG early entropy source...
I (46) boot: Partition Table:
I (49) boot: ## Label            Usage          Type ST Offset   Length
I (55) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (62) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (68) boot:  2 factory          factory app      00 00 00010000 00100000
I (75) boot: End of partition table
I (78) esp_image: segment 0: paddr=00010020 vaddr=42040020 size=1a20ch (107020) map
I (102) esp_image: segment 1: paddr=0002a234 vaddr=40800000 size=05de4h ( 24036) load
I (107) esp_image: segment 2: paddr=00030020 vaddr=42000020 size=342f4h (213748) map
I (140) esp_image: segment 3: paddr=0006431c vaddr=40805de4 size=07f58h ( 32600) load
I (147) esp_image: segment 4: paddr=0006c27c vaddr=4080dd80 size=02084h (  8324) load
I (152) boot: Loaded app from partition at offset 0x10000
I (152) boot: Disabling RNG early entropy source...
I (166) MSPI Timing: Enter flash timing tuning
I (167) MSPI Timing: Enter psram timing tuning
I (169) esp_psram: Found 8MB PSRAM device
I (169) esp_psram: Speed: 80MHz
I (170) cpu_start: Unicore app
I (1213) esp_psram: SPI SRAM memory test OK
I (1222) cpu_start: GPIO 12 and 11 are used as console UART I/O pins
I (1222) cpu_start: Pro cpu start user code
I (1222) cpu_start: cpu freq: 240000000 Hz
I (1224) app_init: Application information:
I (1228) app_init: Project name:     motion_estimation
I (1233) app_init: App version:      42627e3-dirty
I (1238) app_init: Compile time:     Feb 11 2026 19:31:13
I (1243) app_init: ELF file SHA256:  dcd9a806d...
I (1247) app_init: ESP-IDF:          v5.5.2-737-g0da66850d91
I (1253) efuse_init: Min chip rev:     v1.0
I (1257) efuse_init: Max chip rev:     v1.99 
I (1261) efuse_init: Chip rev:         v1.0
I (1265) heap_init: Initializing. RAM available for dynamic allocation:
I (1271) heap_init: At 4081DFD0 len 0003E5D0 (249 KiB): RAM
I (1276) heap_init: At 4085C5A0 len 00002F58 (11 KiB): RAM
I (1281) heap_init: At 50000000 len 00003FE8 (15 KiB): RTCRAM
W (1287) esp_psram: Due to hardware issue on ESP32-C5/C61 (Rev v1.0), PSRAM contents won't be encrypted (for flash encryption enabled case)
W (1299) esp_psram: Please avoid using PSRAM for security sensitive data e.g., TLS stack allocations (CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC)
I (1311) esp_psram: Adding pool of 8192K of PSRAM memory to heap allocator
I (1318) spi_flash: detected chip: generic
I (1321) spi_flash: flash io: qio
W (1324) spi_flash: Detected size(16384k) larger than the size in the binary image header(8192k). Using the size in the binary image header.
W (1337) spi_flash: CPU frequency is set to 240MHz. esp_flash_write_encrypted() will automatically limit CPU frequency to 80MHz during execution.
I (1350) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (1356) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (1371) main_task: Started on CPU0
I (1374) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations
I (1381) main_task: Calling app_main()
I (1385) button: IoT Button Version: 4.1.5
E (1389) i2c.master: this port has not been initialized, please initialize it first
I (1397) i2c_bus: i2c0 bus inited
I (1399) i2c_bus: I2C Bus V2 Config Succeed, Version: 1.5.0
I (1405) BMI2_ESP32: Creating I2C device with address 0x68
I (1410) BMI2_ESP32: I2C device created successfully
I (1434) BMI2_ESP32: Detected FreeRTOS tick rate: 1050 Hz
I (2212) bmi270_api: BMI270 sensor created successfully
I (2223) main_task: Returned from app_main()
I (3941) Model Pipeline: Motion detected: Unknown (confidence: 100.0%)
I (5022) Model Pipeline: Motion detected: Unknown (confidence: 99.9%)
I (8263) Model Pipeline: Motion detected: Unknown (confidence: 98.0%)
I (9992) Model Pipeline: Motion detected: Counterclockwise circle (confidence: 100.0%)
I (11719) Model Pipeline: Motion detected: Unknown (confidence: 95.0%)
I (13232) Model Pipeline: Motion detected: V (confidence: 99.9%)
I (14525) Model Pipeline: Motion detected: Unknown (confidence: 100.0%)
I (15605) Model Pipeline: Motion detected: Unknown (confidence: 99.2%)
```
