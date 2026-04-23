package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

var lastX, lastY, lastZ float32
const motionThreshold = 0.5

func main() {
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	sensor := bmi260.New(i2c)
	sensor.Configure()

	serial.WriteString("Motion detection started\r\n")

	for {
		accelX, accelY, accelZ := sensor.ReadAcceleration()

		if detectMotion(accelX, accelY, accelZ) {
			serial.WriteString("Motion detected!\r\n")
		}

		time.Sleep(time.Millisecond * 100)
	}
}

func detectMotion(x, y, z float32) bool {
	deltaX := abs(x - lastX)
	deltaY := abs(y - lastY)
	deltaZ := abs(z - lastZ)

	lastX = x
	lastY = y
	lastZ = z

	return (deltaX > motionThreshold ||
			deltaY > motionThreshold ||
			deltaZ > motionThreshold)
}

func abs(x float32) float32 {
	if x < 0 {
		return -x
	}
	return x
}
