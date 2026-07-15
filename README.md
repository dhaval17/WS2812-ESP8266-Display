# WS2812-ESP8266-Display

An Arduino sketch for an ESP8266 microcontroller that drives four chained 8×8 WS2812 addressable LED matrices to display a real-time clock with ambient sensor data.

## Description

This repository contains an Arduino sketch file that controls 4 units of 8x8 WS2812 LED matrices. The sketch fetches time via NTP and environmental readings from a DHT11 sensor, then renders the time in HHMM format across the matrix grid with live temperature and humidity progress bars at the bottom.

## Purpose

The main purpose of this project is to display:
- **Time in HHMM format** — each 8×8 matrix unit displays a single character (4 total for HH:MM)
- **Temperature bar** — left 16 pixels of the bottom row, showing 20–36 °C range in amber
- **Humidity bar** — right 16 pixels of the bottom row, showing 0–100% range in green

## Hardware Setup

- **Microcontroller:** ESP8266 (or ESP32 with `#include <WiFi.h>` instead)
- **LED Matrix:** 4× Adafruit NeoMatrix 8×8 (32×8 total), chained via GPIO4 (D2)
- **Sensor:** DHT11 temperature/humidity sensor on GPIO5 (D1)
- **Wiring:** See `IMG_2242.jpg` for reference photo

## Stack

- **Language:** C++ (Arduino)
- **Runtime:** ESP8266 microcontroller
- **Key Libraries:**
  - Adafruit_NeoMatrix / Adafruit_NeoPixel (LED control)
  - Adafruit_GFX (graphics primitives)
  - DHT (DHT11 sensor reading)
  - NTPClient (network time synchronization)
  - ESP8266WiFi / WiFiUdp (networking)

## How It Works

**Initialization:**
- Connects to Wi-Fi using configured SSID and password
- Starts NTP client for time synchronization
- Initializes DHT11 sensor
- Sets up LED matrix with brightness and color profiles

**Main Loop:**
- Updates time from NTP server every 1 second
- Reads DHT11 sensor every 2 seconds
- Renders display:
  - **Top 7 rows:** Time digits in light blue (RGB 140, 240, 240)
  - **Bottom row left (16 pixels):** Temperature progress bar in amber (RGB 255, 100, 0)
  - **Bottom row right (16 pixels):** Humidity progress bar in green (RGB 0, 255, 80)
  - **Empty slots:** Dim background (RGB 15, 15, 15)
- Refreshes display every 100 ms

## Getting Started

### 1. Install Required Libraries

In the Arduino IDE, go to **Sketch → Include Library → Manage Libraries** and install:
- Adafruit NeoMatrix
- Adafruit NeoPixel
- Adafruit GFX
- NTPClient
- DHT sensor library

### 2. Configure Settings

Edit `main.ino` with your setup:

```cpp
// Wi-Fi Credentials (lines 10-11)
const char* ssid     = "your_ssid";
const char* password = "your_password";

// GPIO Pins (lines 14, 26)
#define PIN 4      // NeoMatrix data pin (GPIO4 / D2)
#define DHTPIN 5   // DHT11 signal pin (GPIO5 / D1)

// Timezone (line 38) - example: 19800 for IST (UTC+5:30)
const long timeOffsetInSeconds = 19800;
```

### 3. Customize Colors & Scaling

Adjust color values (lines 56–59):
```cpp
lightBlue          = matrix.Color(140, 240, 240);  // Time text
amberColor         = matrix.Color(255, 100, 0);    // Temp bar
greenColor         = matrix.Color(0, 255, 80);     // Humidity bar
backgroundBarColor = matrix.Color(15, 15, 15);     // Empty slots
```

Adjust sensor ranges (lines 122, 135):
```cpp
int tempBarLength = map(temperature, 20, 36, 0, 16);   // 20–36 °C
int humBarLength = map(humidity, 0, 100, 0, 16);       // 0–100 %
```

### 4. Upload & Verify

- Select **ESP8266** board in Arduino IDE
- Choose the correct COM port
- Click **Upload**
- Open **Serial Monitor** (115200 baud) to verify Wi-Fi connection and sensor updates

## Image

![WS2812 LED Matrix Display](https://raw.githubusercontent.com/dhaval17/WS2812-ESP8266-Display/refs/heads/main/IMG_2242.jpg)

## Notes

- Time is displayed in 12-hour format (noon as 12, midnight as 12)
- DHT11 readings are clamped to integer values
- Update intervals are non-blocking (time: 1s, sensors: 2s)
- Brightness is set low (level 2) to prevent power draw issues; adjust as needed
- Wi-Fi credentials are hardcoded; consider using a config file or secrets management for production
