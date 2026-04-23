package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/bmi260"
	"tinygo.org/x/drivers/i2csoft"
)

// Simple k-NN implementation for small datasets
type Point struct {
	X, Y  float32
	Label string
}

func knnClassify(testPoint Point, trainingData []Point, k int) string {
	distances := make([]float32, len(trainingData))

	// Calculate distances
	for i, point := range trainingData {
		dx := testPoint.X - point.X
		dy := testPoint.Y - point.Y
		distances[i] = sqrt(dx*dx + dy*dy)
	}

	// Find k nearest neighbors
	// Count labels and return most common
	return "class_a" // Simplified
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
	serial.WriteString("k-NN Example\r\n")

	// Training data example
	trainingData := []Point{
		{0.1, 0.2, "stationary"},
		{0.3, 0.1, "stationary"},
		{2.5, 1.8, "walking"},
		{2.8, 2.1, "walking"},
	}

	i2c := i2csoft.New(machine.SCL0_PIN, machine.SDA0_PIN)
	i2c.Configure(i2csoft.I2CConfig{Frequency: 100e3})

	sensor := bmi260.New(i2c)
	sensor.Configure()

	for {
		accelX, accelY, _ := sensor.ReadAcceleration()

		testPoint := Point{X: accelX, Y: accelY}
		result := knnClassify(testPoint, trainingData, 3)

		serial.WriteString("Classification: ")
		serial.WriteString(result)
		serial.WriteString("\r\n")

		time.Sleep(time.Millisecond * 500)
	}
}
