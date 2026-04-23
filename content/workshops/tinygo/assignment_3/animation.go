package main

import (
	"image/color"
	"machine"
	"time"

	"tinygo.org/x/drivers/axp192/m5stack-core2-axp192"
	"tinygo.org/x/drivers/i2csoft"
	"tinygo.org/x/drivers/ili9341"
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

	var ballX int16 = 160
	var ballY int16 = 120
	var velX int16 = 2
	var velY int16 = 3

	for {
		// Clear screen
		display.FillScreen(color.RGBA{20, 20, 60, 255})

		// Update position
		ballX += velX
		ballY += velY

		// Bounce off walls
		if ballX <= 10 || ballX >= 310 {
			velX = -velX
		}
		if ballY <= 10 || ballY >= 230 {
			velY = -velY
		}

		// Draw ball
		display.FillCircle(ballX, ballY, 10, color.RGBA{255, 255, 0, 255})

		time.Sleep(time.Millisecond * 33) // ~30 FPS
	}
}
