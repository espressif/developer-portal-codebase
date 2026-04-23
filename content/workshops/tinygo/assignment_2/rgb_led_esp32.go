package main

import (
	"image/color"
	"machine"
	"time"

	"tinygo.org/x/drivers/ws2812"
)

func main() {
	// ESP32: RGB LED on GPIO8
	led := machine.GPIO8
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	// NeoPixel driver
	neo := ws2812.New(led)

	// Brightness: 0-255 scale. RGB LEDs are extremely bright, so using 20%
	brightness := uint8(51)

	// Base colors at full brightness
	baseColors := []color.RGBA{
		{255, 0, 0, 255},     // Red
		{0, 255, 0, 255},     // Green
		{0, 0, 255, 255},     // Blue
		{255, 255, 0, 255},   // Yellow
		{0, 255, 255, 255},   // Cyan
		{255, 0, 255, 255},   // Magenta
		{255, 255, 255, 255}, // White
	}

	// Apply brightness scaling
	colors := make([]color.RGBA, len(baseColors))
	for i, c := range baseColors {
		colors[i] = color.RGBA{
			R: uint8(uint16(c.R) * uint16(brightness) / 255),
			G: uint8(uint16(c.G) * uint16(brightness) / 255),
			B: uint8(uint16(c.B) * uint16(brightness) / 255),
			A: 255,
		}
	}

	for {
		for _, c := range colors {
			neo.WriteColors([]color.RGBA{c})
			time.Sleep(time.Millisecond * 500)
		}
	}
}
