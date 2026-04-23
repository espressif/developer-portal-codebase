package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

func main() {
	// Initialize serial
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.WriteString("Reading sensor data...\r\n")

	// Initialize I2C
	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	// Initialize BMI260 accelerometer
	sensor := bmi260.New(i2c)
	sensor.Configure()

	for {
		// Read accelerometer data
		accelX, accelY, accelZ := sensor.ReadAcceleration()

		// Output to serial
		serial.WriteString("Accelerometer: X=")
		printFloat(serial, accelX)
		serial.WriteString(" Y=")
		printFloat(serial, accelY)
		serial.WriteString(" Z=")
		printFloat(serial, accelZ)
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 500)
	}
}

func printFloat(serial machine.UART, f float32) {
	// Simple float to string conversion
	buffer := make([]byte, 20)
	neg := f < 0
	if neg {
		f = -f
		serial.WriteByte('-')
	}

	intPart := int(f)
	fracPart := int((f - float32(intPart)) * 100)

	itoa(serial, intPart)
	serial.WriteByte('.')
	itoa(serial, fracPart)
}

func itoa(serial machine.UART, n int) {
	if n == 0 {
		serial.WriteByte('0')
		return
	}

	var buf [10]byte
	i := 10
	for n > 0 && i > 0 {
		i--
		buf[i] = byte('0' + n%10)
		n /= 10
	}

	for i < 10 {
		serial.WriteByte(buf[i])
		i++
	}
}
