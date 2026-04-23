package main

import (
	"io"
	"machine"
	"net/http"
	"time"

	"tinygo.org/x/drivers/netdev"
	nl "tinygo.org/x/drivers/netlink"
	link "tinygo.org/x/espradio/netlink"
)

var ssid string
var password string
var ledPin = machine.GPIO10

func main() {
	// Initialize LED
	ledPin.Configure(machine.PinConfig{Mode: machine.PinOutput})

	// Initialize serial
	serial := machine.Serial
	serial.Configure(machine.UARTConfig{BaudRate: 115200})
	serial.Write([]byte("HTTP Server\r\n"))

	time.Sleep(2 * time.Second)

	// Connect to Wi-Fi with larger arena pool for HTTP
	radioLink := link.Esplink{
		ArenaPoolSize: 48 * 1024, // Larger pool for HTTP connections
	}
	netdev.UseNetdev(&radioLink)

	serial.Write([]byte("Connecting to Wi-Fi...\r\n"))
	err := radioLink.NetConnect(&nl.ConnectParams{
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

	// Get IP address
	addr, _ := radioLink.Addr()
	host := addr.String()
	serial.Write([]byte("Server: http://"))
	serial.Write([]byte(host))
	serial.Write([]byte(":8080\r\n"))

	// Setup HTTP routes
	http.Handle("/", logRequest(root))
	http.Handle("/led/on", logRequest(ledOn))
	http.Handle("/led/off", logRequest(ledOff))
	http.Handle("/status", logRequest(status))

	// Start server with explicit IP address
	serial.Write([]byte("Starting server...\r\n"))
	err = http.ListenAndServe(host+":8080", nil)
	if err != nil {
		serial.Write([]byte("Server error: "))
		serial.Write([]byte(err.Error()))
		serial.Write([]byte("\r\n"))
	}

	for {
		time.Sleep(time.Second)
	}
}

func logRequest(h http.HandlerFunc) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		serial := machine.Serial
		serial.Write([]byte(r.Method))
		serial.Write([]byte(" "))
		serial.Write([]byte(r.URL.Path))
		serial.Write([]byte("\r\n"))
		h(w, r)
	})
}

func root(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	io.WriteString(w, `<!DOCTYPE html>
<html>
<head>
    <title>ESP32 TinyGo Server</title>
    <style>
        body { font-family: Arial; margin: 20px; }
        h1 { color: #333; }
        button { padding: 10px 20px; font-size: 16px; margin: 5px; }
        .on { background: #4CAF50; color: white; }
        .off { background: #f44336; color: white; }
    </style>
</head>
<body>
    <h1>ESP32 TinyGo Web Server</h1>
    <p>Welcome to the TinyGo HTTP Server!</p>

    <h2>LED Control</h2>
    <button class="on" onclick="fetch('/led/on')">LED ON</button>
    <button class="off" onclick="fetch('/led/off')">LED OFF</button>

    <h2>Status</h2>
    <button onclick="fetch('/status').then(r=>r.text()).then(d=>alert(d))">Check Status</button>

    <h2>About</h2>
    <p>This server is running on ESP32 with TinyGo 0.41</p>
    <p>Board: ESP32-C3 / ESP32-S3</p>
</body>
</html>`)
}

func ledOn(w http.ResponseWriter, r *http.Request) {
	ledPin.Low() // Active LOW
	w.Header().Set("Content-Type", "text/plain")
	io.WriteString(w, "LED ON")
}

func ledOff(w http.ResponseWriter, r *http.Request) {
	ledPin.High() // Active LOW
	w.Header().Set("Content-Type", "text/plain")
	io.WriteString(w, "LED OFF")
}

func status(w http.ResponseWriter, r *http.Request) {
	ledState := "OFF"
	if ledPin.Get() {
		ledState = "OFF" // Active LOW
	} else {
		ledState = "ON"
	}

	w.Header().Set("Content-Type", "application/json")
	io.WriteString(w, `{"led":"`+ledState+`","uptime":"running"}`)
}
