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
	// Initialize serial for debug output
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.WriteString("Initializing display...\r\n")

	// Initialize I2C for AXP192 power management
	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	// Initialize AXP192 PMIC (powers display and backlight)
	axp := axp192.New(i2c)
	axp.Begin()
	axp.SetLCDVoltage(3300)  // 3.3V for LCD
	axp.SetLDO2Voltage(3300)  // LDO2 for peripherals
	axp.SetDCDC3(3300)        // DCDC3 for LCD backlight
	axp.EnableLCD(true)       // Enable LCD power
	axp.EnableBacklight(true) // Enable backlight

	serial.WriteString("AXP192 initialized\r\n")

	// Initialize SPI for display
	machine.SPI2.Configure(machine.SPIConfig{
		SCK:       machine.LCD_SCK_PIN,
		SDO:       machine.LCD_SDO_PIN,
		SDI:       machine.LCD_SDI_PIN,
		Frequency: 40e6, // 40MHz
	})

	serial.WriteString("SPI initialized\r\n")

	// Initialize ILI9342C display
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

	serial.WriteString("Display initialized!\r\n")

	// Clear screen with blue background
	display.FillScreen(color.RGBA{20, 20, 60, 255})

	// Keep display on
	for {
		time.Sleep(time.Second)
	}
}
