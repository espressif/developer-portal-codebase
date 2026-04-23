package main

import (
	"machine"
	"time"
)

func main() {
	// Configure LED pin for your board:
	// ESP32/ESP32-S3: GPIO2 (most common)
	// ESP32-C3: GPIO8
	led := machine.GPIO2
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	for {
		led.Low()  // LED ON (active LOW)
		time.Sleep(time.Millisecond * 500)

		led.High() // LED OFF
		time.Sleep(time.Millisecond * 500)
	}
}
