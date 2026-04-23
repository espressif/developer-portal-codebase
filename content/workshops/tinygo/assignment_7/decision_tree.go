package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

// Decision tree for activity classification
func classifyActivity(x, y, z float32) string {
	magnitude := calculateMotion(x, y, z)

	switch {
	case magnitude > 2.5:
		return "running"
	case magnitude > 1.5:
		return "walking"
	case magnitude > 0.8:
		return "sitting"
	default:
		return "stationary"
	}
}

func calculateMotion(x, y, z float32) float32 {
	return sqrt(x*x + y*y + z*z)
}

func sqrt(x float32) float32 {
	z := float32(1.0)
	for i := 0; i < 10; i++ {
		z -= (z*z - x) / (2 * z)
	}
	return z
}

var serial machine.UART

func main() {
	serial = machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	sensor := bmi260.New(i2c)
	sensor.Configure()

	for {
		accelX, accelY, accelZ := sensor.ReadAcceleration()
		activity := classifyActivity(accelX, accelY, accelZ)

		serial.WriteString("Activity: ")
		serial.WriteString(activity)
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 200)
	}
}
