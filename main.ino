#include <ESP8266WiFi.h>  // Use #include <WiFi.h> instead if you are using an ESP32
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// --- Wi-Fi Credentials ---
const char* ssid     = "a";
const char* password = "b#";

// --- Matrix Setup ---
#define PIN 4 // Data pin connected to NeoPixel matrix (GPIO4 -> D2)

// Matrix width: 8, Matrix height: 8, Tiles X: 4, Tiles Y: 1 (Total 32x8)
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  8, 8, 4, 1, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_PROGRESSIVE +
  NEO_TILE_TOP       + NEO_TILE_LEFT +
  NEO_TILE_ROWS      + NEO_TILE_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

// --- DHT11 Setup ---
#define DHTPIN 5          // DHT11 Signal wire connected to GPIO5 (D1 on ESP8266)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Colors (RGB565) ---
uint16_t lightBlue;
uint16_t amberColor;
uint16_t greenColor;
uint16_t backgroundBarColor; // Dim color for empty bar slots

// --- NTP Time Setup ---
WiFiUDP ntpUDP;
const long timeOffsetInSeconds = 19800; // Adjust for your timezone
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffsetInSeconds, 60000);

String timeText = "1200"; 
unsigned long lastTimeUpdate = 0;
unsigned long lastSensorUpdate = 0;

int temperature = 0;
int humidity = 0;

void setup() {
  Serial.begin(115200);
  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(2); 
  
  // Setup color profiles
  lightBlue          = matrix.Color(140, 240, 240); // Time text
  amberColor         = matrix.Color(255, 100, 0);   // Temp Bar
  greenColor         = matrix.Color(0, 255, 80);    // Humidity Bar
  backgroundBarColor = matrix.Color(15, 15, 15);    // Dim background for the bar tracks

  // Initialize DHT Sensor
  dht.begin();

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
  timeClient.update();
  unsigned long currentMillis = millis();

  // --- Fetch NTP Time (Every 1 Second) ---
  if (currentMillis - lastTimeUpdate > 1000) {
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    
    hours = hours % 12;
    if (hours == 0) hours = 12; 
    
    char buffer[5];
    sprintf(buffer, "%02d%02d", hours, minutes);
    timeText = String(buffer);
    
    lastTimeUpdate = currentMillis;
  }

  // --- Fetch Sensor Data (Every 2 Seconds) ---
  if (currentMillis - lastSensorUpdate > 2000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature(); 

    if (!isnan(h) && !isnan(t)) {
      humidity = (int)h;
      temperature = (int)t;
    }
    lastSensorUpdate = currentMillis;
  }

  // Clear the display buffer
  matrix.fillScreen(0);

  // --- 1. Draw Static Time (Top 7 rows) ---
  matrix.setTextColor(lightBlue);
  for (int i = 0; i < 4; i++) {
    matrix.setCursor((i * 8) + 1, 0); 
    matrix.print(timeText[i]);         
  }

// --- 2. Map and Draw Temperature Bar (Left 16 Pixels) ---
  // Scale adjusted to your specific range: 20C to 36C mapped to 0-16 pixels
  int tempBarLength = map(temperature, 20, 36, 0, 16); 
  tempBarLength = constrain(tempBarLength, 0, 16); // Safety boundary

  for (int x = 0; x < 16; x++) {
    if (x < tempBarLength) {
      matrix.drawPixel(x, 7, greenColor); // Active temperature pixels
    } else {
      matrix.drawPixel(x, 7, backgroundBarColor); // Empty track background
    }
  }

  // --- 3. Map and Draw Humidity Bar (Right 16 Pixels) ---
  // Humidity goes from 0% to 100%
  int humBarLength = map(humidity, 0, 100, 0, 16);
  humBarLength = constrain(humBarLength, 0, 16); // Safety boundary

  for (int x = 16; x < 32; x++) {
    // Check relative to the right-side allocation (x - 16)
    if ((x - 16) < humBarLength) {
      matrix.drawPixel(x, 7, greenColor); // Active humidity pixels
    } else {
      matrix.drawPixel(x, 7, backgroundBarColor); // Empty track background
    }
  }

  // Render buffer to the physical panel
  matrix.show();
  delay(100); 
}
