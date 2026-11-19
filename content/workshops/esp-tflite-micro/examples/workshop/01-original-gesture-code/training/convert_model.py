from pathlib import Path

import tensorflow as tf
from tensorflow import keras


KERAS_MODEL_PATH = Path("simple_1dcnn_model.h5")
TFLITE_MODEL_PATH = Path("model.tflite")

model = keras.models.load_model(KERAS_MODEL_PATH)
converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.float32]
converter.inference_input_type = tf.float32
converter.inference_output_type = tf.float32

tflite_model = converter.convert()
TFLITE_MODEL_PATH.write_bytes(tflite_model)

print(f"Saved {TFLITE_MODEL_PATH}")
print(f"Model size: {len(tflite_model)} bytes")

interpreter = tf.lite.Interpreter(model_content=tflite_model)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("Input:", input_details[0]["shape"], input_details[0]["dtype"])
print("Output:", output_details[0]["shape"], output_details[0]["dtype"])

assert input_details[0]["shape"].tolist() == [1, 200, 3]
assert input_details[0]["dtype"] == tf.float32.as_numpy_dtype
assert output_details[0]["shape"].tolist() == [1, 4]
assert output_details[0]["dtype"] == tf.float32.as_numpy_dtype

print("Operators:")
for operation in interpreter._get_ops_details():
    print(f"- {operation['op_name']}")
