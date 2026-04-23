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

// Game Configuration
const (
	BOARD_WIDTH  = 20
	BOARD_HEIGHT = 10
	ADC_RESOLUTION = 65520
	JOYSTICK_CENTER  = 32760
	JOYSTICK_DEADZONE = 10000
)

// Game State
type GameState struct {
	playerX      int
	playerY      int
	goalX        int
	goalY        int
	score        int
	joystickX    *machine.ADC
	joystickY    *machine.ADC
}

func main() {
	// Initialize ADC
	machine.InitADC()

	// Initialize serial
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})

	// Initialize joystick
	game := &GameState{
		playerX:   BOARD_WIDTH / 2,
		playerY:   BOARD_HEIGHT / 2,
		goalX:     5,
		goalY:     5,
		score:     0,
		joystickX: &machine.ADC{Pin: JOYSTICK_X_PIN},
		joystickY: &machine.ADC{Pin: JOYSTICK_Y_PIN},
	}
	game.joystickX.Configure(machine.ADCConfig{})
	game.joystickY.Configure(machine.ADCConfig{})

	// Allow ADC to stabilize
	time.Sleep(time.Millisecond * 100)

	// Clear screen and show title
	serial.Write([]byte("\033[2J\033[H")) // ANSI clear screen and home
	serial.Write([]byte("=== Joystick Game ===\r\n"))
	serial.Write([]byte("Collect stars (*) with your player (@)\r\n"))
	serial.Write([]byte("Use joystick to move\r\n"))
	serial.Write([]byte("========================\r\n\r\n"))

	// Game loop
	lastMove := time.Now()
	moveDelay := time.Millisecond * 150 // Movement speed
	frameCount := uint32(0)

	for {
		frameCount++

		// Read joystick
		rawX := readADC(game.joystickX)
		rawY := readADC(game.joystickY)

		// Convert to direction
		dirX := getDirection(rawX, true)  // X-axis inverted
		dirY := getDirection(rawY, false) // Y-axis normal

		// Move player at controlled speed
		if time.Since(lastMove) > moveDelay {
			if dirX != 0 || dirY != 0 {
				game.movePlayer(dirX, dirY)
			}
			lastMove = time.Now()
		}

		// Render game
		game.render(serial, rawX, rawY, dirX, dirY)

		time.Sleep(time.Millisecond * 50) // Render rate
	}
}

func (g *GameState) movePlayer(dirX, dirY int) {
	// Update position
	g.playerX += dirX
	g.playerY += dirY

	// Keep player in bounds
	if g.playerX < 0 {
		g.playerX = 0
	}
	if g.playerX >= BOARD_WIDTH {
		g.playerX = BOARD_WIDTH - 1
	}
	if g.playerY < 0 {
		g.playerY = 0
	}
	if g.playerY >= BOARD_HEIGHT {
		g.playerY = BOARD_HEIGHT - 1
	}

	// Check if player reached goal
	if g.playerX == g.goalX && g.playerY == g.goalY {
		g.score++
		g.spawnGoal()
	}
}

func (g *GameState) spawnGoal() {
	// Simple goal spawning: random position away from player
	// Using time for pseudo-randomness
	ticks := time.Now().UnixNano()

	for {
		g.goalX = int(ticks % BOARD_WIDTH)
		g.goalY = int((ticks / BOARD_WIDTH) % BOARD_HEIGHT)

		// Make sure goal isn't on player
		if g.goalX != g.playerX || g.goalY != g.playerY {
			break
		}

		ticks += 1
	}
}

func (g *GameState) render(serial machine.Serialer, rawX, rawY uint32, dirX, dirY int) {
	// Move cursor to top of game area
	serial.Write([]byte("\033[6;0f")) // Move to line 6

	// Draw board border
	serial.Write([]byte("+"))
	for i := 0; i < BOARD_WIDTH; i++ {
		serial.Write([]byte("-"))
	}
	serial.Write([]byte("+\r\n"))

	// Draw board rows
	for y := 0; y < BOARD_HEIGHT; y++ {
		serial.Write([]byte("|"))
		for x := 0; x < BOARD_WIDTH; x++ {
			if x == g.playerX && y == g.playerY {
				serial.Write([]byte("@")) // Player
			} else if x == g.goalX && y == g.goalY {
				serial.Write([]byte("*")) // Goal
			} else {
				serial.Write([]byte("."))
			}
		}
		serial.Write([]byte("|\r\n"))
	}

	// Draw bottom border
	serial.Write([]byte("+"))
	for i := 0; i < BOARD_WIDTH; i++ {
		serial.Write([]byte("-"))
	}
	serial.Write([]byte("+\r\n"))

	// Show score and direction
	serial.Write([]byte("\r\nScore: "))
	printInt(serial, uint32(g.score))
	serial.Write([]byte(" | Move: "))

	// Show direction
	if dirY < 0 {
		serial.Write([]byte("UP"))
	} else if dirY > 0 {
		serial.Write([]byte("DOWN"))
	} else if dirX < 0 {
		serial.Write([]byte("LEFT"))
	} else if dirX > 0 {
		serial.Write([]byte("RIGHT"))
	} else {
		serial.Write([]byte("CENTER"))
	}

	// Show raw values on separate line for debugging
	serial.Write([]byte("\r\nX="))
	printInt(serial, rawX)
	serial.Write([]byte(" Y="))
	printInt(serial, rawY)
	serial.Write([]byte("    ")) // Clear extra characters
}

func readADC(adc *machine.ADC) uint32 {
	const samples = 5
	var sum uint32

	for i := 0; i < samples; i++ {
		sum += uint32(adc.Get())
		time.Sleep(time.Microsecond * 50)
	}

	return sum / samples
}

func getDirection(value uint32, isInverted bool) int {
	dir := 0
	if value < JOYSTICK_CENTER-JOYSTICK_DEADZONE {
		dir = -1
	} else if value > JOYSTICK_CENTER+JOYSTICK_DEADZONE {
		dir = 1
	}

	// Invert direction for X-axis (swapped on hardware)
	if isInverted && dir != 0 {
		dir = -dir
	}

	return dir
}

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
