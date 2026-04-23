package main

import (
	"image/color"
	"machine"
	"time"

	"tinygo.org/x/drivers/ws2812"
)

func main() {
	// ESP32-C3: RGB LED on GPIO7
	led := machine.GPIO7
	led.Configure(machine.PinConfig{Mode: machine.PinOutput})

	// NeoPixel driver
	neo := ws2812.New(led)
	neo.SetBrightness(51) // 20% brightness - RGB LEDs are extremely bright!

	colors := []color.RGBA{
		{255, 0, 0, 255},     // Red
		{0, 255, 0, 255},     // Green
		{0, 0, 255, 255},     // Blue
		{255, 255, 0, 255},   // Yellow
		{0, 255, 255, 255},   // Cyan
		{255, 0, 255, 255},   // Magenta
		{255, 255, 255, 255}, // White
	}

	for {
		for _, c := range colors {
			neo.WriteColors([]color.RGBA{c})
			time.Sleep(time.Millisecond * 500)
		}
	}
}
