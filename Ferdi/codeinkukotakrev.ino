#include <WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <DHT.h>

const char* ssid              = "UGMURO-INET";
const char* password          = "Gepuk15000";
const char* writeAPIKey       = "L1JFXC6ULNYU577E";
const unsigned long channelID = 2732118;

// Define pins 
#define PIR_PIN         14
#define DHT_PIN         2
#define SOUND_PIN       34
#define BUZZER_PIN      12
#define RELAY_IN1_PIN   5
#define RELAY_IN2_PIN   4
//#define SCL_PIN 22
//#define SDA_PIN 21

// DHT sensor type 
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// LCD setup: Address 0x27, 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define thresholds
float minTemp = 33.0, maxTemp = 35.0, currentTemp, humidity;
int soundThreshold = 500, soundLevel;
bool fanOn = false, motionDetected;

unsigned long sensorInterval      = 500;
unsigned long displayInterval     = 1000;
unsigned long thingSpeakInterval  = 15000;

WiFiClient client;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
elapsedMillis thingSpeakMillis;

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  lcd.begin();
  //lcd.init();
  lcd.backlight();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0,0);
    lcd.print("Connecting..");
  }
  ThingSpeak.begin(client);

  // Initialize PIR sensor
  pinMode(PIR_PIN, INPUT);

  // Initialize buzzer and relay as outputs
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_IN1_PIN, OUTPUT);
  pinMode(RELAY_IN2_PIN, OUTPUT);

  // Start with fan and buzzer off
  digitalWrite(RELAY_IN1_PIN, LOW);
  digitalWrite(RELAY_IN2_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);


  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("Baby Incubator");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);  // Show the message for 2 seconds
  lcd.clear();
}

void loop() {
  if (sensorMillis >= sensorInterval) {
    currentTemp = dht.readTemperature();
    humidity = dht.readHumidity();
    soundLevel = analogRead(SOUND_PIN);
    motionDetected = digitalRead(PIR_PIN);
    delay(10);

    Serial.println("Suhu DHT    : " + String(currentTemp) + "°C");
    Serial.println("Humidity    : " + String(humidity) + "%");
    Serial.println("Sound Level : " + String(soundLevel));

    workingConditions();

    sensorMillis = 0;
  }

  if (displayMillis >= displayInterval) {
    lcd.setCursor(0, 0);
    lcd.print("TempDHT : " + String(currentTemp) + "°C");

    lcd.setCursor(0, 1);
    lcd.print("Humid   : " + String(humidity) + " %");

    displayMillis = 0;
  }

  if (thingSpeakMillis >= thingSpeakInterval) {
    ThingSpeak.setField(1, currentTemp);
    ThingSpeak.setField(2, humidity);

    int APIwait = ThingSpeak.writeFields(channelID, writeAPIKey);
    
    if (APIwait == 200) {
      Serial.println("Update successful");
    } else {
      Serial.println("Error code: " + String(APIwait));
    }

    thingSpeakMillis = 0;
  }
}

void workingConditions() {
  if (motionDetected) {
    Serial.println("Motion detected! Baby is moving excessively.");
    triggerBuzzer();
  }

  // Check temperature range and control the fan
  if (currentTemp > maxTemp) {
    Serial.println("Temperature too high! Turning on the fan.");
    activateFan();
  } else if (currentTemp < minTemp && fanOn) {
    Serial.println("Temperature back to normal. Turning off the fan.");
    deactivateFan();
  }

  // Check if sound level exceeds threshold (baby crying)
  if (soundLevel > soundThreshold) {
    Serial.println("Baby crying detected! Triggering buzzer.");
    triggerBuzzer();
  }
}

// Function to activate the fan via relay
void activateFan() {
  digitalWrite(RELAY_IN1_PIN, HIGH);  // Turn on fan
  digitalWrite(RELAY_IN2_PIN, LOW);   // Ensure correct polarity
  fanOn = true;
}

// Function to deactivate the fan
void deactivateFan() {
  digitalWrite(RELAY_IN1_PIN, LOW);  // Turn off fan
  fanOn = false;
}

// Function to trigger the buzzer when the baby is crying or moving excessively
void triggerBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);  // Turn on buzzer
  delay(1000);                     // Buzzer sounds for 1 second
  digitalWrite(BUZZER_PIN, LOW);   // Turn off buzzer
}
