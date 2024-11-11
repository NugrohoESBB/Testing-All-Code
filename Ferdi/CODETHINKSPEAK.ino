#include <WiFi.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Wire.h>
#include <DHT.h>

#define DHTPIN            32
#define DS_PIN            34
#define RELAY_PELTIER_PIN 25
#define RELAY_FAN1_PIN    26
#define RELAY_FAN2_PIN    27
#define RELAY_FAN3_PIN    32
#define RELAY_FAN4_PIN    33
#define RELAY_FAN5_PIN    18
#define RELAY_LAMP1_PIN   0
#define RELAY_LAMP2_PIN   2

const char* ssid              = "UGMURO-INET";
const char* password          = "Gepuk15000";
const char* writeAPIKey       = "L1JFXC6ULNYU577E";
const unsigned long channelID = 2732118;

#define DHTTYPE DHT21
OneWire oneWire(DS_PIN);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DallasTemperature ds18b20(&oneWire);

float temperature, currentTempDHT, currentTempDS;

unsigned long sensorInterval      = 500;
unsigned long displayInterval     = 1000;
unsigned long thingSpeakInterval  = 10000;

WiFiClient client;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
elapsedMillis thingSpeakMillis;

void setup() {
  Serial.begin(115200);

  dht.begin();
  ds18b20.begin();
  lcd.begin();
  lcd.backlight();
  
  pinMode(RELAY_PELTIER_PIN, OUTPUT);
  pinMode(RELAY_FAN1_PIN, OUTPUT);
  pinMode(RELAY_FAN2_PIN, OUTPUT);
  pinMode(RELAY_FAN3_PIN, OUTPUT);
  pinMode(RELAY_FAN4_PIN, OUTPUT);
  pinMode(RELAY_FAN5_PIN, OUTPUT);
  pinMode(RELAY_LAMP1_PIN, OUTPUT);
  pinMode(RELAY_LAMP2_PIN, OUTPUT);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi ");
  Serial.println(ssid);
  ThingSpeak.begin(client);

  deactivateCooling();
  deactivateHeating();
}

void loop() {
  if (sensorMillis >= sensorInterval) {
    temperature = dht.readTemperature();
    ds18b20.requestTemperatures();
    float currentTempDS = ds18b20.getTempCByIndex(0);
    delay(10);

    Serial.println("Suhu DHT: " + String(currentTempDHT) + "°C");
    Serial.println("Suhu DS: " + String(currentTempDS) + "°C");

    if (currentTempDHT < 33.0 || currentTempDS < 33.0) {
      activateHeating();
      deactivateCooling();
    } else if (currentTempDHT > 35.0 || currentTempDS > 35.0) {
      deactivateHeating();
      activateCooling();
    }

    sensorMillis = 0;
  }

  if (displayMillis >= displayInterval) {
    lcd.setCursor(0, 0);
    lcd.print("TempDHT : " + String(currentTempDHT) + " C");

    lcd.setCursor(0, 1);
    lcd.print("TempDS  : " + String(currentTempDS) + " C");

    displayMillis = 0;
  }

  if (thingSpeakMillis >= thingSpeakInterval) {
    ThingSpeak.setField(1, currentTempDHT);
    ThingSpeak.setField(2, currentTempDS);

    int APIwait = ThingSpeak.writeFields(channelID, writeAPIKey);
    
    if (APIwait == 200) {
      Serial.println("Update successful");
    } else {
     Serial.println("Error code: " + String(APIwait));
    }
    thingSpeakMillis = 0;
  }
}

void activateCooling() {
  digitalWrite(RELAY_PELTIER_PIN, HIGH);
  digitalWrite(RELAY_FAN1_PIN, HIGH);
  digitalWrite(RELAY_FAN2_PIN, HIGH);
  digitalWrite(RELAY_FAN3_PIN, HIGH);
  digitalWrite(RELAY_FAN4_PIN, HIGH);
  digitalWrite(RELAY_FAN5_PIN, HIGH);
}

// Function to deactivate cooling (fans and Peltier)
void deactivateCooling() {
  digitalWrite(RELAY_PELTIER_PIN, LOW);
  digitalWrite(RELAY_FAN1_PIN, LOW);
  digitalWrite(RELAY_FAN2_PIN, LOW);
  digitalWrite(RELAY_FAN3_PIN, LOW);
  digitalWrite(RELAY_FAN4_PIN, LOW);
  digitalWrite(RELAY_FAN5_PIN, LOW);
}

// Function to activate heating (lamps)
void activateHeating() {
  digitalWrite(RELAY_LAMP1_PIN, HIGH);
  digitalWrite(RELAY_LAMP2_PIN, HIGH);
}

// Function to deactivate heating (lamps)
void deactivateHeating() {
  digitalWrite(RELAY_LAMP1_PIN, LOW);
  digitalWrite(RELAY_LAMP2_PIN, LOW);
}
