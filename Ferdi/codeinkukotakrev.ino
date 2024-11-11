#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "UGMURO-INET";
const char* password = "Gepuk15000";

// Define pins 
#define PIR_PIN 14       
#define DHT_PIN 2         
#define SOUND_PIN 34       
#define BUZZER_PIN 12      
#define RELAY_IN1_PIN 5   
#define RELAY_IN2_PIN 4    
// #define SCL_PIN 22
// #define SDA_PIN 21

// DHT sensor type 
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// LCD setup: Address 0x27, 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define thresholds
float minTemp = 33.0;  
float maxTemp = 35.0; 
int soundThreshold = 500;  

// Variables
float currentTemp;
float humidity;
int soundLevel;
bool fanOn = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0,0);
    lcd.print("Connecting..");
  }

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

  // Initialize the LCD and turn on the backlight
  lcd.begin();
  lcd.backlight();

  // Display startup message
  lcd.setCursor(0, 0);
  lcd.print("Baby Incubator");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);  // Show the message for 2 seconds
  lcd.clear();
}

void loop() {
  // Read temperature and humidity from DHT sensor
  currentTemp = dht.readTemperature();
  humidity = dht.readHumidity();

  // Check if readings are valid
  if (isnan(currentTemp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity to serial monitor
  Serial.print("Temp: ");
  Serial.print(currentTemp);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Display temperature and humidity on the LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(currentTemp);
  lcd.print(" C  ");
  
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(humidity);
  lcd.print(" %  ");

  // Read sound sensor value (analog reading)
  soundLevel = analogRead(SOUND_PIN);
  Serial.print("Sound Level: ");
  Serial.println(soundLevel);

  // Read PIR sensor (motion detection)
  bool motionDetected = digitalRead(PIR_PIN);
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

  // Short delay before next loop
  delay(1000);
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
