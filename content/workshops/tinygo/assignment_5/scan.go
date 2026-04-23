package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/netdev"
	link "tinygo.org/x/espradio/netlink"
	"tinygo.org/x/espradio"
)

func main() {
	// Initialize serial for output
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.Write([]byte("Wi-Fi Scanner\r\n"))

	// Wait for serial to be ready
	time.Sleep(2 * time.Second)

	// Initialize espradio radio
	err := espradio.Enable(espradio.Config{})
	if err != nil {
		serial.Write([]byte("Radio enable failed: "))
		serial.Write([]byte(err.Error()))
		serial.Write([]byte("\r\n"))
		return
	}

	err = espradio.Start()
	if err != nil {
		serial.Write([]byte("Radio start failed: "))
		serial.Write([]byte(err.Error()))
		serial.Write([]byte("\r\n"))
		return
	}

	// Initialize espradio link for netdev
	radioLink := link.Esplink{}
	netdev.UseNetdev(&radioLink)

	serial.Write([]byte("Scanning for networks...\r\n"))

	// Scan for networks
	networks, err := espradio.Scan()
	if err != nil {
		serial.Write([]byte("Scan failed: "))
		serial.Write([]byte(err.Error()))
		serial.Write([]byte("\r\n"))
		return
	}

	serial.Write([]byte("Found "))
	writeInt(serial, len(networks))
	serial.Write([]byte(" networks:\r\n\r\n"))

	// Display networks
	for i, net := range networks {
		writeInt(serial, i+1)
		serial.Write([]byte(". SSID: "))
		serial.Write([]byte(net.SSID))
		serial.Write([]byte("\r\n   RSSI: "))
		writeInt(serial, net.RSSI)
		serial.Write([]byte(" dBm\r\n\r\n"))
	}

	serial.Write([]byte("Scan complete!\r\n"))

	for {
		time.Sleep(time.Second)
	}
}

func writeInt(serial machine.Serialer, n int) {
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
