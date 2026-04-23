package main

import (
	"machine"
	"time"
)

func shortBlink() {
	led.High()
	time.Sleep(time.Millisecond * 200)
	led.Low()
	time.Sleep(time.Millisecond * 200)
}

func longBlink() {
	led.High()
	time.Sleep(time.Millisecond * 600)
	led.Low()
	time.Sleep(time.Millisecond * 200)
}

var led machine.Pin

func main() {
	// Configure LED pin for your board:
	// ESP32/ESP32-S3: GPIO2
	// ESP32-C3: GPIO2
	led = machine.GPIO2
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	for {
		// S: ***
		shortBlink()
		shortBlink()
		shortBlink()
		time.Sleep(time.Millisecond * 400)

		// O: ---
		longBlink()
		longBlink()
		longBlink()
		time.Sleep(time.Millisecond * 400)

		// S: ***
		shortBlink()
		shortBlink()
		shortBlink()

		time.Sleep(time.Second * 2) // Pause between SOS
	}
}
