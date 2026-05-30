#include <ESP8266WiFi.h>  // Use #include <WiFi.h> instead if you are using an ESP32
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// --- Wi-Fi Credentials ---
const char* ssid     = "a";
const char* password = "b#";

// --- Matrix Setup ---
#define PIN 4 // Data pin connected to NeoPixel matrix

// Updated for 4 side-by-side 8x8 matrices (32x8 total resolution)
// Matrix width: 8, Matrix height: 8, Tiles X: 4, Tiles Y: 1
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  8, 8, 4, 1, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_PROGRESSIVE +
  NEO_TILE_TOP       + NEO_TILE_LEFT +
  NEO_TILE_ROWS      + NEO_TILE_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

// --- Static Light Blue Color (RGB565) ---
uint16_t lightBlue;

// --- NTP Time Setup ---
WiFiUDP ntpUDP;
const long timeOffsetInSeconds = 19800; // Adjust for your timezone
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffsetInSeconds, 60000);

String text = "1200"; 
unsigned long lastTimeUpdate = 0;

void setup() {
  Serial.begin(115200);
  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(2); 
  
  // Define light blue and apply it as the static text color
  lightBlue = matrix.Color(200, 100, 100); 
  matrix.setTextColor(lightBlue);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Start NTP client
  timeClient.begin();
}

void loop() {
  // Update time from NTP server
  timeClient.update();

  // Update the HHMM string once every second
  if (millis() - lastTimeUpdate > 1000) {
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    
    // --- 12-Hour Format Conversion ---
    hours = hours % 12;
    if (hours == 0) {
      hours = 12; // Midnight or Noon should display as 12, not 00
    }
    
    char buffer[5];
    sprintf(buffer, "%02d%02d", hours, minutes);
    text = String(buffer);
    
    lastTimeUpdate = millis();
  }

  matrix.fillScreen(0);

  // --- Render Each Digit into Its Own 8x8 Unit ---
  // The standard Adafruit font is 5x7 pixels. 
  // To center a 5-pixel wide digit inside an 8-pixel wide tile, we offset by +1 pixel.
  for (int i = 0; i < 4; i++) {
    int tileXOffset = i * 8;       // Finds the starting X coordinate for tile 0, 1, 2, or 3
    matrix.setCursor(tileXOffset + 1, 0); 
    matrix.print(text[i]);         // Print just the single character for this tile
  }
  
  matrix.show();
  delay(200); 
}
