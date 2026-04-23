package main

import (
	"machine"
	"time"

	"tinygo.org/x/drivers/netdev"
	nl "tinygo.org/x/drivers/netlink"
	link "tinygo.org/x/espradio/netlink"
)

var ssid string
var password string

func main() {
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.Write([]byte("Wi-Fi Connection Test\r\n"))

	time.Sleep(2 * time.Second)

	// Initialize radio link for netdev
	radioLink := link.Esplink{}
	netdev.UseNetdev(&radioLink)

	// Connect to Wi-Fi
	serial.Write([]byte("Connecting to "))
	serial.Write([]byte(ssid))
	serial.Write([]byte("...\r\n"))

	err := radioLink.NetConnect(&nl.ConnectParams{
		Ssid:       ssid,
		Passphrase: password,
	})

	if err != nil {
		serial.Write([]byte("Connection failed\r\n"))
		return
	}

	serial.Write([]byte("Connected!\r\n"))

	// Get IP address
	addr, err := radioLink.Addr()
	if err != nil {
		serial.Write([]byte("Error getting address\r\n"))
		return
	}

	serial.Write([]byte("IP Address: "))
	serial.Write([]byte(addr.String()))
	serial.Write([]byte("\r\n"))

	// Keep connection alive
	for {
		time.Sleep(time.Second)
	}
}
