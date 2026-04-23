package main

import (
	"machine"
	"time"
)

// GPIO Configuration for Joystick ADC pins
const (
	JOYSTICK_X_PIN = machine.ADC4 // X-axis - GPIO4 (ADC1_CH0)
	JOYSTICK_Y_PIN = machine.ADC6 // Y-axis - GPIO6 (ADC1_CH2)
)

// ADC Configuration
const (
	ADC_RESOLUTION = 65520 // ESP32-S3 ADC returns scaled 0-65520 (not raw 12-bit)
	JOYSTICK_CENTER  = 32760 // Center position (approximately 50%)
	JOYSTICK_DEADZONE = 5000  // Deadzone around center (adjusted for scaled range)
)

func main() {
	// Initialize ADC
	machine.InitADC()

	// Initialize serial
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.Write([]byte("Joystick ADC Reader\r\n"))
	serial.Write([]byte("====================\r\n\r\n"))

	// Initialize ADC pins
	joystickX := machine.ADC{Pin: JOYSTICK_X_PIN}
	joystickX.Configure(machine.ADCConfig{})

	joystickY := machine.ADC{Pin: JOYSTICK_Y_PIN}
	joystickY.Configure(machine.ADCConfig{})

	// Allow ADC to stabilize
	time.Sleep(time.Millisecond * 100)

	serial.Write([]byte("Reading joystick on GPIO4 (X) and GPIO6 (Y)...\r\n\r\n"))
	serial.Write([]byte("Format: X=[0-65520] Y=[0-65520] | X=[0.0-1.0] Y=[0.0-1.0]\r\n"))
	serial.Write([]byte("Center: ~32760 (~0.5) Deadzone: ±5000\r\n\r\n"))

	for {
		// Read raw ADC values (0-65520)
		rawX := readADC(joystickX)
		rawY := readADC(joystickY)

		// Diagnostic: Check if value is stuck (possible connection issue)
		// Value around 30400 suggests ADC pin constraint or not connected
		if rawX > 30000 && rawX < 31000 {
			serial.Write([]byte("WARNING: X ADC stuck at ~30400 - Check GPIO4 connection\r\n"))
		}
		if rawY > 30000 && rawY < 31000 {
			serial.Write([]byte("WARNING: Y ADC stuck at ~30400 - Check GPIO6 connection\r\n"))
		}

		// Apply deadzone (center ± 5000)
		inDeadzoneX := rawX > (JOYSTICK_CENTER-JOYSTICK_DEADZONE) && rawX < (JOYSTICK_CENTER+JOYSTICK_DEADZONE)
		inDeadzoneY := rawY > (JOYSTICK_CENTER-JOYSTICK_DEADZONE) && rawY < (JOYSTICK_CENTER+JOYSTICK_DEADZONE)

		// Convert to normalized values (0.0-1.0)
		normX := float32(rawX) / float32(ADC_RESOLUTION)
		normY := float32(rawY) / float32(ADC_RESOLUTION)

		// Convert to joystick direction (-1.0 to 1.0, center at 0)
		dirX := (normX - 0.5) * 2.0
		dirY := (normY - 0.5) * 2.0

		// Clamp to valid range
		if dirX < -1.0 {
			dirX = -1.0
		} else if dirX > 1.0 {
			dirX = 1.0
		}
		if dirY < -1.0 {
			dirY = -1.0
		} else if dirY > 1.0 {
			dirY = 1.0
		}

		// Output to serial
		serial.Write([]byte("X="))
		printInt(serial, rawX)
		serial.Write([]byte(" Y="))
		printInt(serial, rawY)
		serial.Write([]byte(" | X="))
		printFloat(serial, normX)
		serial.Write([]byte(" Y="))
		printFloat(serial, normY)
		serial.Write([]byte(" | Dir: ["))
		printFloat(serial, dirX)
		serial.Write([]byte(","))
		printFloat(serial, dirY)
		serial.Write([]byte("]"))

		// Indicate deadzone position
		if inDeadzoneX && inDeadzoneY {
			serial.Write([]byte(" [CENTER]"))
		}

		serial.Write([]byte("\r\n"))

		time.Sleep(time.Millisecond * 100)
	}
}

// readADC performs oversampling for more stable readings
func readADC(adc machine.ADC) uint32 {
	const samples = 10
	var sum uint32

	for i := 0; i < samples; i++ {
		sum += uint32(adc.Get())
		time.Sleep(time.Microsecond * 100)
	}

	return sum / samples
}

// printInt prints integer value
func printInt(serial machine.Serialer, n uint32) {
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

// printFloat prints float value with 2 decimal places
func printFloat(serial machine.Serialer, f float32) {
	neg := f < 0
	if neg {
		f = -f
		serial.WriteByte('-')
	}

	intPart := uint32(f)
	fracPart := uint32((f - float32(intPart)) * 100)

	printInt(serial, intPart)
	serial.WriteByte('.')
	if fracPart < 10 {
		serial.WriteByte('0')
	}
	printInt(serial, fracPart)
}

// Example: Reading other ADC sensors
//
// Temperature sensor (TMP36):
//   temp := (float32(adcValue) * 3300.0 / 65520.0 - 500) / 10.0 // in Celsius
//
// Light sensor (photoresistor with voltage divider):
//   light := float32(adcValue) / 65520.0 // 0.0 (dark) to 1.0 (bright)
//
// Potentiometer:
//   position := float32(adcValue) / 65520.0 // 0.0 to 1.0 rotation
