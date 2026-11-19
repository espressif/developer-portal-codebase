# Gesture datasets

Run the following command from `training/` to download the official reference
dataset:

```bash
python3.12 download_dataset.py
```

This creates:

- `o.csv`: counterclockwise-circle samples
- `v.csv`: V-gesture samples
- `unknown.csv`: other and idle movements

After completing Assignment 1, add `new_gesture.csv` here. It must contain at
least 25 rows. Every file is headerless, and every row contains 600
comma-separated gyroscope values in this order:

```text
x0,y0,z0,x1,y1,z1,...,x199,y199,z199
```

From this directory, validate the new gesture before training:

```bash
python3.12 validate_dataset.py
```

The CSV files are generated/downloaded inputs and are intentionally not stored
in this project.
