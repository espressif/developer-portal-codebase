package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

// Smooth sensor data and detect patterns
type MovingAverage struct {
	buffer [10]float32
	index  int
	sum    float32
}

func (ma *MovingAverage) Update(value float32) float32 {
	ma.sum -= ma.buffer[ma.index]
	ma.buffer[ma.index] = value
	ma.sum += value
	ma.index = (ma.index + 1) % 10
	return ma.sum / 10
}

func detectPeak(data []float32) int {
	if len(data) < 3 {
		return -1
	}

	for i := 1; i < len(data)-1; i++ {
		if data[i] > data[i-1] && data[i] > data[i+1] {
			return i
		}
	}
	return -1
}

var serial machine.UART

func main() {
	serial = machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	sensor := bmi260.New(i2c)
	sensor.Configure()

	maX := MovingAverage{}
	maY := MovingAverage{}
	maZ := MovingAverage{}

	for {
		accelX, accelY, accelZ := sensor.ReadAcceleration()

		// Smooth data
		smoothX := maX.Update(accelX)
		smoothY := maY.Update(accelY)
		smoothZ := maZ.Update(accelZ)

		serial.WriteString("Smoothed: X=")
		printFloat(smoothX)
		serial.WriteString(" Y=")
		printFloat(smoothY)
		serial.WriteString(" Z=")
		printFloat(smoothZ)
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 100)
	}
}

func printFloat(f float32) {
	neg := f < 0
	if neg {
		f = -f
		serial.WriteByte('-')
	}

	intPart := int(f)
	fracPart := int((f - float32(intPart)) * 100)

	itoa(intPart)
	serial.WriteByte('.')
	itoa(fracPart)
}

func itoa(n int) {
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
