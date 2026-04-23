package main

import (
	"machine"
	"time"
)

func main() {
	// Initialize serial (USB)
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{
		BaudRate: 115200,
	})

	// Configure LED pin for your board:
	// ESP32/ESP32-S3: GPIO2
	// ESP32-C3: GPIO2
	led := machine.GPIO2
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	serial.Write([]byte("Blinky starting!\r\n"))

	for {
		serial.Write([]byte("LED ON\r\n"))
		led.High()
		time.Sleep(time.Millisecond * 500)

		serial.Write([]byte("LED OFF\r\n"))
		led.Low()
		time.Sleep(time.Millisecond * 500)
	}
}
