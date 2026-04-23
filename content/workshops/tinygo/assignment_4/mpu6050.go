package main

import (
	"machine"
	"time"
)

// GPIO Configuration for I2C bus on ESP32-C3
// MPU-6050: SDA=GPIO4, SCL=GPIO5
const (
	I2C_SDA = machine.GPIO4 // I2C SDA pin
	I2C_SCL = machine.GPIO5 // I2C SCL pin
)

// MPU-6050 I2C Address
const (
	MPU6050_ADDR = 0x68 // MPU-6050 default I2C address
)

func main() {
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	// Use machine.I2C0 for ESP32-C3
	i2c := machine.I2C0
	i2c.Configure(machine.I2CConfig{
		SDA:       I2C_SDA,
		SCL:       I2C_SCL,
		Frequency: 400 * machine.KHz, // 400kHz fast mode
	})

	serial.Write([]byte("MPU-6050 I2C Test - ESP32-C3\r\n"))
	serial.Write([]byte("==============================\r\n\r\n"))

	// Check WHO_AM_I register (should return 0x68 for MPU-6050)
	whoami := make([]byte, 1)
	i2c.Tx(uint16(MPU6050_ADDR), []byte{0x75}, whoami)

	serial.Write([]byte("MPU-6050 WHO_AM_I: 0x"))
	serial.WriteByte(hexChar(whoami[0] >> 4))
	serial.WriteByte(hexChar(whoami[0] & 0x0F))
	serial.Write([]byte("\r\n"))

	if whoami[0] != 0x68 {
		serial.Write([]byte("ERROR: WHO_AM_I should be 0x68, got 0x"))
		serial.WriteByte(hexChar(whoami[0] >> 4))
		serial.WriteByte(hexChar(whoami[0] & 0x0F))
		serial.Write([]byte("\r\n"))
		serial.Write([]byte("Check I2C connections: SDA=GPIO4, SCL=GPIO5\r\n"))
		for {
			time.Sleep(time.Second)
		}
	}

	// Wake up MPU-6050 (clear sleep bit in PWR_MGMT_1 register)
	i2c.Tx(uint16(MPU6050_ADDR), []byte{0x6B, 0x00}, nil)
	time.Sleep(time.Millisecond * 100)

	serial.Write([]byte("MPU-6050 initialized successfully!\r\n"))
	serial.Write([]byte("Reading accelerometer data (1 Hz)...\r\n"))
	serial.Write([]byte("================================\r\n\r\n"))

	for {
		// Read accelerometer data (6 bytes starting at ACCEL_XOUT_H)
		data := make([]byte, 6)
		i2c.Tx(uint16(MPU6050_ADDR), []byte{0x3B}, data)

		// Convert to signed 16-bit values (big-endian)
		accelX := int16(uint16(data[0])<<8 | uint16(data[1]))
		accelY := int16(uint16(data[2])<<8 | uint16(data[3]))
		accelZ := int16(uint16(data[4])<<8 | uint16(data[5]))

		// Convert to g-force (16384 LSB/g for MPU-6050)
		accelX_g := float32(accelX) / 16384.0
		accelY_g := float32(accelY) / 16384.0
		accelZ_g := float32(accelZ) / 16384.0

		// Output to serial
		serial.Write([]byte("X: "))
		printFloat(serial, accelX_g)
		serial.Write([]byte(" g  Y: "))
		printFloat(serial, accelY_g)
		serial.Write([]byte(" g  Z: "))
		printFloat(serial, accelZ_g)
		serial.Write([]byte(" g\r\n"))

		time.Sleep(time.Second)
	}
}

func hexChar(b byte) byte {
	if b < 10 {
		return '0' + b
	}
	return 'A' + b - 10
}

func printFloat(serial machine.Serialer, f float32) {
	neg := f < 0
	if neg {
		f = -f
		serial.WriteByte('-')
	}

	intPart := int32(f)
	fracPart := int32((f - float32(intPart)) * 100)

	printInt(serial, intPart)
	serial.WriteByte('.')
	if fracPart < 10 {
		serial.WriteByte('0')
	}
	printInt(serial, fracPart)
}

func printInt(serial machine.Serialer, n int32) {
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
