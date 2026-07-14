#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Wire.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
#define LED D6
#define BUZZER D5
#define GAS_SENSOR A0
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
char ssid[] = "purrple";    
char pass[] = "1234567890";
WiFiClient client;

// ThingSpeak channel details
unsigned long myChannelNumber = 3016488;
const char *myWriteAPIKey = "N8WYDG66PDPDQ5LU";

void setup() {
  // Serial and peripherals
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  ThingSpeak.begin(client);
}

void loop() {
  // Read sensors
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  int gasValue = analogRead(GAS_SENSOR);


  // Debug prints
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C, Humidity: ");
  Serial.print(humi);
  Serial.print(" %, Gas: ");
  Serial.println(gasValue);

  // LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("G:");
  lcd.print(gasValue);
  lcd.setCursor(7, 0);
  lcd.print("T:");
  lcd.print(temp);
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(humi);
  lcd.print(" %");

  // Alert logic
  if (gasValue > 720) {
    digitalWrite(LED,HIGH);
    Serial.println("Gas leak detected!");
  } else {
    digitalWrite(LED,HIGH);
    Serial.println("No gas leak.");
  }
    if (temp > 32) {
    digitalWrite(BUZZER, HIGH);
    Serial.println("temperature high");
  } else {
    digitalWrite(BUZZER, LOW);
    Serial.println("temperature high");
  }

  // Reconnect WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi.");
  }

  // Upload data to ThingSpeak
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humi);
  ThingSpeak.setField(3, gasValue);
  ThingSpeak.setStatus("Sensor update");

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("ThingSpeak update successful.");
  } else {
    Serial.println("ThingSpeak update failed. HTTP error code: " + String(x));
  }

  delay(20000); // Wait 20 seconds before sending next update
}
