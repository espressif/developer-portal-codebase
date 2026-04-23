// LED Connection Guide for ESP32:
//
// Identify LED legs:
// - Anode (+): Longer leg
// - Cathode (-): Shorter leg, flat side on LED casing
//
// Resistor values for ESP32 (3.3V):
// - Red LED: 470Ω (range: 330Ω - 1kΩ)
// - Green LED: 330Ω (range: 220Ω - 680Ω)
// - Blue LED: 220Ω (range: 100Ω - 330Ω)
// - Orange/Yellow: 470Ω (range: 330Ω - 680Ω)
//
// Wiring: ESP32 GPIO → Resistor → LED Anode → LED Cathode → GND
//
package main

import (
	"machine"
	"time"
)

func main() {
	// Configure LED pin for your board:
	// ESP32/ESP32-S3: GPIO2 (most common)
	// ESP32-C3: GPIO2
	led := machine.GPIO3
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	for {
		led.High() // LED ON (active HIGH)
		time.Sleep(time.Millisecond * 500)

		led.Low() // LED OFF
		time.Sleep(time.Millisecond * 500)
	}
}
