package main

import (
	"image/color"
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
	"tinygo.org/x/drivers/ili9341"
	"tinygo.org/x/drivers/axp192/m5stack-core2-axp192"
	"tinygo.org/x/drivers/pixel"
	"tinygo.org/x/tinygl-font"
	"tinygo.org/x/tinygl-font/roboto"
)

func main() {
	// Initialize serial
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	// Initialize I2C
	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	// Initialize BMI260
	sensor := bmi260.New(i2c)
	sensor.Configure()

	serial.WriteString("BMI260 initialized\r\n")

	// Initialize display
	i2cDisp := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2cDisp.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	axp := axp192.New(i2cDisp)
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

	// Text buffer
	textDisplay := pixel.NewImage[pixel.RGB565BE](300, 40)
	white := pixel.NewRGB565BE(color.RGBA{255, 255, 255, 255})

	for {
		// Read accelerometer
		accelX, accelY, accelZ := sensor.ReadAcceleration()

		// Clear text buffer
		textDisplay.FillSolidColor(pixel.NewRGB565BE(color.RGBA{20, 20, 60, 255}))

		// Display readings
		displayText := func(text string, y int16) {
			textDisplay.FillSolidColor(pixel.NewRGB565BE(color.RGBA{20, 20, 60, 255}))
			font.Draw(roboto.Regular16, text, 0, 16, white, textDisplay)
			pixelData := textDisplay.RawBuffer()
			w, h := textDisplay.Size()
			display.DrawRGBBitmap8(10, y, pixelData, int16(w), int16(h))
		}

		displayText("Accelerometer Readings:", 10)
		displayText("X: "+formatFloat(accelX)+" g", 50)
		displayText("Y: "+formatFloat(accelY)+" g", 90)
		displayText("Z: "+formatFloat(accelZ)+" g", 130)

		// Calculate total acceleration
		total := calcMagnitude(accelX, accelY, accelZ)
		displayText("Total: "+formatFloat(total)+" g", 170)

		// Detect orientation
		orientation := detectOrientation(accelX, accelY, accelZ)
		displayText("Orientation: "+orientation, 210)

		// Serial output
		serial.WriteString("X: ")
		serial.WriteString(formatFloat(accelX))
		serial.WriteString(" Y: ")
		serial.WriteString(formatFloat(accelY))
		serial.WriteString(" Z: ")
		serial.WriteString(formatFloat(accelZ))
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 200)
	}
}

func formatFloat(f float32) string {
	neg := f < 0
	if neg {
		f = -f
	}

	intPart := int(f)
	fracPart := int((f - float32(intPart)) * 100)

	var result [20]byte
	i := 0

	if neg {
		result[i] = '-'
		i++
	}

	// Integer part
	if intPart == 0 {
		result[i] = '0'
		i++
	} else {
		var buf [10]byte
		j := 10
		for intPart > 0 && j > 0 {
			j--
			buf[j] = byte('0' + intPart%10)
			intPart /= 10
		}
		for j < 10 {
			result[i] = buf[j]
			i++
			j++
		}
	}

	result[i] = '.'
	i++

	// Fraction part
	result[i] = byte('0' + fracPart/10)
	i++
	result[i] = byte('0' + fracPart%10)
	i++

	return string(result[:i])
}

func calcMagnitude(x, y, z float32) float32 {
	return sqrt(x*x + y*y + z*z)
}

func sqrt(x float32) float32 {
	if x == 0 {
		return 0
	}

	z := float32(1.0)
	for i := 0; i < 10; i++ {
		z -= (z*z - x) / (2 * z)
	}
	return z
}

func detectOrientation(x, y, z float32) string {
	const threshold = 0.7

	if x > threshold {
		return "Right"
	} else if x < -threshold {
		return "Left"
	} else if y > threshold {
		return "Down"
	} else if y < -threshold {
		return "Up"
	} else if z > threshold {
		return "Flat"
	} else if z < -threshold {
		return "Upside Down"
	} else {
		return "Tilted"
	}
}
