package main

import (
	"image/color"
	"machine"
	"time"

	"tinygo.org/x/drivers/axp192/m5stack-core2-axp192"
	"tinygo.org/x/drivers/i2csoft"
	"tinygo.org/x/drivers/ili9341"
	"tinygo.org/x/drivers/pixel"
	"tinygo.org/x/tinygl-font"
	"tinygo.org/x/tinygl-font/roboto"
)

func main() {
	// Initialize display
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	axp := axp192.New(i2c)
	axp.Begin()
	axp.SetLCDVoltage(3300)
	axp.SetLDO2Voltage(3300)
	axp.SetDCDC3(3300)
	axp.EnableLCD(true)
	axp.EnableBacklight(true)

	machine.SPI2.Configure(machine.SPIConfig{
		SCK:       machine.LCD_SCK_PIN,
		SDO:       machine.LCD_SDO_PIN,
		SDI:       machine.LCD_SDI_PIN,
		Frequency: 40e6,
	})

	display := ili9341.NewSPI(
		machine.SPI2,
		machine.LCD_DC_PIN,
		machine.LCD_SS_PIN,
		machine.NoPin,
	)

	display.Configure(ili9341.Config{
		Width:            320,
		Height:           240,
		DisplayInversion: true,
	})
	display.SetRotation(ili9341.Rotation0Mirror)

	display.FillScreen(color.RGBA{20, 20, 60, 255})

	// Create off-screen buffer for text
	textDisplay := pixel.NewImage[pixel.RGB565BE](300, 60)

	// Colors
	white := pixel.NewRGB565BE(color.RGBA{255, 255, 255, 255})
	bgColor := pixel.NewRGB565BE(color.RGBA{20, 20, 60, 255})

	// Clear buffer with background color
	textDisplay.FillSolidColor(bgColor)

	// Draw text using Roboto 48pt font
	font.Draw(roboto.Regular48, "Hello!", 0, 48, white, textDisplay)

	// Get pixel data and display
	pixelData := textDisplay.RawBuffer()
	width, height := textDisplay.Size()
	display.DrawRGBBitmap8(10, 90, pixelData, int16(width), int16(height))

	for {
		time.Sleep(time.Second)
	}
}
