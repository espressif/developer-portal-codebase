package main

import (
	"machine"
	"time"
)

// GPIO Configuration for I2C bus
// Update these pins based on your board configuration
const (
	I2C_SDA = machine.GPIO10 // I2C SDA pin
	I2C_SCL = machine.GPIO8  // I2C SCL pin
)

// I2C Sensor Address
const (
	IMU_I2C_ADDR = 0x68 // ICM-42670-P or BMI160
)

var lastX, lastY, lastZ float32
const motionThreshold = 0.5

func main() {
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	i2c := machine.I2C0
	i2c.Configure(machine.I2CConfig{
		SDA:       I2C_SDA,
		SCL:       I2C_SCL,
		Frequency: 100 * machine.KHz, // Use safer 100kHz instead of 1kHz
	})

	// Wake up ICM-42670-P
	i2c.Tx(uint16(IMU_I2C_ADDR), []byte{0x4D, 0x0F}, nil)
	time.Sleep(time.Millisecond * 50)

	serial.Write([]byte("Motion detection started\r\n"))

	for {
		// Read accelerometer data
		data := make([]byte, 6)
		i2c.Tx(uint16(IMU_I2C_ADDR), []byte{0x1D}, data)

		accelX := int16(uint16(data[0])<<8 | uint16(data[1]))
		accelY := int16(uint16(data[2])<<8 | uint16(data[3]))
		accelZ := int16(uint16(data[4])<<8 | uint16(data[5]))

		// Convert to g-force
		accelX_g := float32(accelX) / 2048.0
		accelY_g := float32(accelY) / 2048.0
		accelZ_g := float32(accelZ) / 2048.0

		if detectMotion(accelX_g, accelY_g, accelZ_g) {
			serial.Write([]byte("Motion detected!\r\n"))
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
