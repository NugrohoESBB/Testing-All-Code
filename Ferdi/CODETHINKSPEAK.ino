#include <WiFi.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Wire.h>
#include <DHT.h>

#define DHTPIN            23
#define DS_PIN            4
#define RELAY_PELTIER_PIN 25
#define RELAY_FAN1_PIN    26
#define RELAY_FAN2_PIN    27
#define RELAY_FAN3_PIN    32
#define RELAY_FAN4_PIN    18
#define RELAY_FAN5_PIN    19
#define RELAY_LAMP1_PIN   0
#define RELAY_LAMP2_PIN   2

const char* ssid              = "UGMURO-INET";
const char* password          = "Gepuk15000";
const char* writeAPIKey       = "45FCKYW6MTSAT7JJ";
const unsigned long channelID = 2741978;

#define DHTTYPE DHT21
OneWire oneWire(DS_PIN);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DallasTemperature ds18b20(&oneWire);

float humidity, currentTempDHT, currentTempDS, CtFahreinheit, heatIndex;
String formattedTempDHT, formattedTempDS, formattedHumidity, formattedFahreinheit, formattedHeatIndex;

unsigned long sensorInterval      = 500;
unsigned long displayInterval     = 1000;
unsigned long thingSpeakInterval  = 10000;

WiFiClient client;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
elapsedMillis thingSpeakMillis;

float calculateHeatIndex(float CtFahreinheit, float humidity) {
  // Rumus empiris untuk perhitungan Heat Index dalam Fahrenheit
  heatIndex = -42.379 + 2.04901523 * CtFahreinheit + 10.14333127 * humidity 
                    - 0.22475541 * CtFahreinheit * humidity 
                    - 0.00683783 * CtFahreinheit * CtFahreinheit 
                    - 0.05481717 * humidity * humidity 
                    + 0.00122874 * CtFahreinheit * CtFahreinheit * humidity 
                    + 0.00085282 * CtFahreinheit * humidity * humidity 
                    - 0.00000199 * CtFahreinheit * CtFahreinheit * humidity * humidity;
  
  // Penyesuaian untuk kelembapan rendah atau tinggi
  if (humidity < 13 && (CtFahreinheit >= 80.0 && CtFahreinheit <= 112.0)) {
    heatIndex -= ((13 - humidity) / 4) * sqrt((17 - abs(CtFahreinheit - 95.0)) / 17);
  } else if (humidity > 85 && (CtFahreinheit >= 80.0 && CtFahreinheit <= 87.0)) {
    heatIndex += ((humidity - 85) / 10) * ((87 - CtFahreinheit) / 5);
  }

  return heatIndex;
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
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0,0);
    lcd.print("Connecting..");
  }
  Serial.print("Connected to WiFi ");
  Serial.println(ssid);
  ThingSpeak.begin(client);

  lcd.setCursor(0, 0);
  lcd.print("Baby Incubator");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);  // Show the message for 2 seconds
  lcd.clear();

  deactivateCooling();
  deactivateHeating();
}

void loop() {
  if (sensorMillis >= sensorInterval) {
    currentTempDHT = dht.readTemperature();
    humidity = dht.readHumidity();
    ds18b20.requestTemperatures();
    currentTempDS = ds18b20.getTempCByIndex(0);
    
    CtFahreinheit = (currentTempDHT * 9.0/5.0) + 32.0;
    delay(10);
    heatIndex = calculateHeatIndex(CtFahreinheit, humidity);

    formattedTempDHT = String(currentTempDHT, 2);
    formattedTempDS = String(currentTempDS, 2);
    formattedHumidity = String(humidity, 2);
    formattedFahreinheit = String(CtFahreinheit, 2);
    formattedHeatIndex = String(heatIndex, 2);

    Serial.println("Suhu DHT    : " + String(currentTempDHT) + "°C");
    Serial.println("Suhu DS     : " + String(currentTempDS) + "°C");
    Serial.println("Humidity    : " + String(humidity) + "%");

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
    ThingSpeak.setField(1, formattedTempDHT);
    ThingSpeak.setField(2, formattedTempDS);
    ThingSpeak.setField(3, formattedFahreinheit);
    ThingSpeak.setField(4, formattedHeatIndex);
    ThingSpeak.setField(5, formattedHumidity);

    int APIwait = ThingSpeak.writeFields(channelID, writeAPIKey);
    
    if (APIwait == 200) {
      Serial.println("Update successful");
    } else {
     Serial.println("Error code: " + String(APIwait));
    }
    thingSpeakMillis = 0;
  }
}

