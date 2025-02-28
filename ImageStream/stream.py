#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>

#define DHTPIN 4
#define DHTTYPE DHT21
#define soilPin 34
#define RELAY1_PIN 26
#define RELAY2_PIN 25
#define RELAY3_PIN 17
#define RELAY4_PIN 16

const char* ssid              = "UGMURO-INET";
const char* password          = "Gepuk15000";
const char* writeAPIKey       = "X0076DPBF0F1X2OE";
const unsigned long channelID = 2847979;

unsigned long thingSpeakInterval  = 15000;
unsigned long sensorInterval      = 500;
unsigned long displayInterval     = 1000;

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);
elapsedMillis thingSpeakMillis;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
LiquidCrystal_I2C lcd(0x27, 20, 4);

int soilValue;
int soilPercentage;
float temperature;
float humidity;

void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Selamat Datang!");
  lcd.setCursor(0, 1);
  lcd.print("-");
  lcd.setCursor(3, 3);
  lcd.print("-- UG MURO --");
  delay(5000);
  lcd.clear();
  delay(2000);

  pinMode(soilPin, INPUT);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Connected to WiFi");
  ThingSpeak.begin(client);

  lcd.setCursor(5, 0);
  lcd.print("Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Suhu   : ");
  lcd.setCursor(0, 2);
  lcd.print("K.Udara: ");
  lcd.setCursor(0, 3);
  lcd.print("K.Tanah: ");
}

void loop() {
  if (sensorMillis >= sensorInterval) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    delay(10);
    soilValue = analogRead(soilPin);
    soilPercentage = map(soilValue, 4095, 0, 0, 100);
    soilPercentage = constrain(soilPercentage, 0, 100);

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("% \t");
    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.print("*C \t");
    Serial.print("Soil: ");
    Serial.print(soilPercentage);
    Serial.println("%");

    sensorMillis = 0;
  }

  if (displayMillis >= displayInterval) {
    lcd.setCursor(0, 0);
    lcd.print("Monitoring");

    lcd.setCursor(0, 1);
    lcd.print("Suhu: ", temperature, "C");
    lcd.setCursor(0, 2);
    lcd.print("Hum: ", humidity, "%");
    lcd.setCursor(0, 3);
    lcd.print("Soil: ", soilPercentage, "%");

    displayMillis = 0;
  }

  if (thingSpeakMillis >= thingSpeakInterval) {
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, soilPercentage);

    int x = ThingSpeak.writeFields(channelID, writeAPIKey);
    if (x == 200) {
      Serial.println("Update successful.");
    } else {
      Serial.println("Update failed. HTTP error code: " + String(x));
    }
    thingSpeakMillis = 0;
  }
}
