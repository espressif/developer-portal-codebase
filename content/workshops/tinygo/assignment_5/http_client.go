package main

import (
	"io"
	"machine"
	"net/http"
	"time"

	"tinygo.org/x/drivers/netdev"
	link "tinygo.org/x/espradio/netlink"
	"tinygo.org/x/espradio"
)

var ssid string
var password string

func main() {
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.Write([]byte("HTTP Client Test\r\n"))

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

	// Connect to Wi-Fi
	radioLink := link.Esplink{}
	netdev.UseNetdev(&radioLink)

	serial.Write([]byte("Connecting to Wi-Fi...\r\n"))
	err = radioLink.NetConnect(&link.ConnectParams{
		Ssid:       ssid,
		Passphrase: password,
	})

	if err != nil {
		serial.Write([]byte("Connection failed\r\n"))
		return
	}

	serial.Write([]byte("Connected!\r\n"))

	// Wait for DHCP
	time.Sleep(5 * time.Second)

	// Fetch webpage
	serial.Write([]byte("Fetching http://example.com...\r\n"))

	resp, err := http.Get("http://example.com")
	if err != nil {
		serial.Write([]byte("HTTP GET failed: "))
		serial.Write([]byte(err.Error()))
		serial.Write([]byte("\r\n"))
		return
	}
	defer resp.Body.Close()

	serial.Write([]byte("Status: "))
	writeInt(serial, resp.StatusCode)
	serial.Write([]byte("\r\n\r\n"))

	// Read response
	buf := make([]byte, 256)
	for {
		n, err := resp.Body.Read(buf)
		if n > 0 {
			serial.Write(buf[:n])
		}
		if err != nil {
			break
		}
	}

	serial.Write([]byte("\r\n\r\nDone!\r\n"))

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
