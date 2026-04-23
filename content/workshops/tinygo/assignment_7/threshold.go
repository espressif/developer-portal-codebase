package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

// Simple gesture detection using thresholds
func detectGesture(accelX, accelY, accelZ float32) string {
	const (
		shakeThreshold = 2.0
		waveThreshold  = 1.0
	)

	motion := calculateMotion(accelX, accelY, accelZ)

	switch {
	case motion > shakeThreshold:
		return "shake"
	case abs(accelX) > waveThreshold:
		return "wave_left"
	case abs(accelY) > waveThreshold:
		return "wave_updown"
	default:
		return "idle"
	}
}

func calculateMotion(x, y, z float32) float32 {
	return sqrt(x*x + y*y + z*z)
}

func sqrt(x float32) float32 {
	// Newton-Raphson square root
	z := float32(1.0)
	for i := 0; i < 10; i++ {
		z -= (z*z - x) / (2 * z)
	}
	return z
}

func abs(x float32) float32 {
	if x < 0 {
		return -x
	}
	return x
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
		gesture := detectGesture(accelX, accelY, accelZ)

		serial.WriteString("Gesture: ")
		serial.WriteString(gesture)
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 100)
	}
}
