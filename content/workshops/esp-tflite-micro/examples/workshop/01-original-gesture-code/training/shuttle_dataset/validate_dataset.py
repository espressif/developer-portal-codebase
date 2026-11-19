import pandas as pd

data = pd.read_csv("new_gesture.csv", header=None)

print("Samples:", data.shape[0])
print("Values per sample:", data.shape[1])

assert data.shape[0] >= 25
assert data.shape[1] == 600
assert not data.isnull().any().any()
