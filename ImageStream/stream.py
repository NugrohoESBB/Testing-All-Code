#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT21
#define soilPin 34
#define RELAY1_PIN 26 // Kipas
#define RELAY2_PIN 25
#define RELAY3_PIN 17 // Pompa
#define RELAY4_PIN 16

const char* ssid              = "UGMURO-INET";
const char* password          = "Gepuk15000";
const char* writeAPIKey       = "X0076DPBF0F1X2OE";
const unsigned long channelID = 2847979;

int soilValue, APIhandler;
float soilPercentage, temperature, humidity;

unsigned long thingSpeakInterval = 15000, sensorInterval = 500, lcdInterval = 2000, serialInterval = 2000;

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
elapsedMillis thingSpeakMillis, sensorMillis, lcdMillis, serialMillis;
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(115200);

  pinMode(soilPin, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("   INITIATION....   ");

  delay (1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("   Connecting....   ");
  }

  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("  WiFi: " + String (ssid));

  delay(1000);

  dht.begin();
  ThingSpeak.begin(client);

  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("     ALL CLEAR!     ");
  delay(2000);
  lcd.clear();
}

void loop() {
  if (serialMillis >= serialInterval) {
    serialOutput();
    serialMillis = 0;
  }

  if (lcdMillis >= lcdInterval) {
    lcdOutput()
    lcdMillis = 0;
  }

  if (sensorMillis >= sensorInterval) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    delay(10);
    soilValue = analogRead(soilPin);
    soilPercentage = map(soilValue, 4095, 0, 0, 100);
    soilPercentage = constrain(soilPercentage, 0, 100);
    conditionRelay();

    sensorMillis = 0;
  }

  if (thingSpeakMillis >= thingSpeakInterval) {
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, soilPercentage);

    APIhandler = ThingSpeak.writeFields(channelID, writeAPIKey);
    if (APIhandler == 200) {
      Serial.println("Update successful.");
    } else {
      Serial.println("Update failed. HTTP error code: " + String(APIhandler));
    }
    thingSpeakMillis = 0;
  }
}

void lcdOutput() {
  lcd.setCursor(0, 0);
  lcd.print("Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Suhu: " + String(temperature) + "°C");
  lcd.setCursor(0, 2);
  lcd.print("Hum: " + String(humidity) + "%");
  lcd.setCursor(0, 3);
  lcd.print("Soil: " + String(soilPercentage) + "%");
}

void serialOutput() {
  Serial.println("Temperature : " + String(temperature) + "*C");
  Serial.println("Humidity    : " + String(humidity) + "%");
  Serial.println("Soil        : " + String(soilPercentage) + "%");
}

void conditionRelay() {
  if (temperature > 20.0) {
    digitalWrite(RELAY1_PIN, LOW);
  } else {
    digitalWrite(RELAY1_PIN, HIGH);
  }

  if (soilPercentage > 50.0) {
    digitalWrite(RELAY3_PIN, LOW);
  } else {
    digitalWrite(RELAY3_PIN, HIGH);
  }
}
