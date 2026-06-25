#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ================= DHT22 =================
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ================= MQ2 =================
#define MQ2_PIN 34

// ================= RGB LED =================
#define RED_LED 25
#define GREEN_LED 26
#define BLUE_LED 27

// ================= BUZZER =================
#define BUZZER 15

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= THINGSPEAK =================
String apiKey = "U29G6AFEHL112FTO";

// ================= VARIABLES =================
String statusText;

void setup() {

  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(BUZZER, LOW);

  dht.begin();

  // OLED
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("Smart Gas Monitor");
  display.display();

  delay(2000);

  // WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
}

void loop() {

  // ================= SENSOR READINGS =================

  int gasValue = analogRead(MQ2_PIN);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // ================= HAZARD CLASSIFICATION =================

  if (gasValue < 1200) {

    statusText = "NORMAL";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, LOW);

    digitalWrite(BUZZER, LOW);
  }

  else if (gasValue < 2500) {

    statusText = "WARNING";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    digitalWrite(BUZZER, LOW);
  }

  else {

    statusText = "CRITICAL";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    digitalWrite(BUZZER, HIGH);
  }

  // ================= SERIAL MONITOR =================

  Serial.println("--------------------------------");

  Serial.print("Gas Level: ");
  Serial.println(gasValue);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Status: ");
  Serial.println(statusText);

  // ================= OLED DISPLAY =================

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SMART GAS MONITOR");

  display.setCursor(0, 15);
  display.print("Gas: ");
  display.println(gasValue);

  display.setCursor(0, 28);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");

  display.setCursor(0, 41);
  display.print("Hum : ");
  display.print(humidity);
  display.println(" %");

  display.setCursor(0, 54);
  display.print("Status:");
  display.print(statusText);

  display.display();

  // ================= THINGSPEAK =================

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url =
      "http://api.thingspeak.com/update?api_key=" +
      apiKey +
      "&field1=" + String(gasValue) +
      "&field2=" + String(temperature, 2) +
      "&field3=" + String(humidity, 2);

    http.begin(url);

    int responseCode = http.GET();

    Serial.print("ThingSpeak Response: ");
    Serial.println(responseCode);

    http.end();
  }

  delay(15000);
}