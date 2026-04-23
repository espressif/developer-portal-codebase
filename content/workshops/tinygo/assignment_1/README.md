# Assignment 1: Install TinyGo

## Overview

This assignment contains a simple test program to verify your TinyGo installation.

## Board Support

This example runs on the host machine (not embedded).

## Build Instructions

```bash
# Build for host
tinygo build -o tinygo-test .

# Run
./tinygo-test
```

Expected output:
```
Hello from TinyGo! 0
Hello from TinyGo! 1
Hello from TinyGo! 2
Hello from TinyGo! 3
Hello from TinyGo! 4
```

## Requirements

- Go 1.26+
- TinyGo 0.41+

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-1/
