#!/usr/bin/env python3
"""Download the three-class ESP-SensairShuttle reference dataset."""

from pathlib import Path
from urllib.request import urlopen
from zipfile import ZipFile
import io


DATASET_URL = (
    "https://dl.espressif.com/AE/esp-iot-solution/"
    "gesture_recognition/shuttle_dataset.zip"
)
DATASET_DIR = Path(__file__).resolve().parent / "shuttle_dataset"
EXPECTED_FILES = ("o.csv", "v.csv", "unknown.csv")


def main() -> None:
    DATASET_DIR.mkdir(parents=True, exist_ok=True)
    print(f"Downloading {DATASET_URL}")
    with urlopen(DATASET_URL, timeout=60) as response:
        archive = response.read()

    with ZipFile(io.BytesIO(archive)) as zip_file:
        members = {Path(name).name: name for name in zip_file.namelist()}
        missing = [name for name in EXPECTED_FILES if name not in members]
        if missing:
            raise RuntimeError(f"Dataset archive is missing: {', '.join(missing)}")

        for filename in EXPECTED_FILES:
            target = DATASET_DIR / filename
            target.write_bytes(zip_file.read(members[filename]))
            print(f"Created {target.relative_to(Path.cwd())}")


if __name__ == "__main__":
    main()
