#include <WiFi.h>
#include <ThingSpeak.h>
#include <elapsedMillis.h>
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
const char* APIKey            = "L1JFXC6ULNYU577E";
const unsigned long channelID = 2732118;

#define DHTTYPE DHT21
OneWire ds1(DS_PIN);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float temperature, currentTempDHT, currentTempDS;

unsigned long sensorInterval      = 500;
unsigned long displayInterval     = 1000;
unsigned long thingSpeakInterval  = 10000;

WiFiClient client;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
elapsedMillis thingSpeakMillis;

void dsValue(void) {
  byte i1;
  byte present1 = 0;
  byte type_s1;
  byte data1[12];
  byte addr1[8];
  
  if (!ds1.search(addr1)) {
    ds1.reset_search();
    delay(250);
    return;
  }

  if (OneWire::crc8(addr1, 7) != addr1[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr1[0]) {
    case 0x10:
      type_s1 = 1;
      break;
    case 0x28:
      type_s1 = 0;
      break;
    case 0x22:
      type_s1 = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds1.reset();
  ds1.select(addr1);
  // start conversion, with parasite power on at the end  
  ds1.write(0x44, 1);
  delay(1000);
  present1 = ds1.reset();
  ds1.select(addr1);    
  // Read Scratchpad
  ds1.write(0xBE);

  for (i1 = 0; i1 < 9; i1++) {
    data1[i1] = ds1.read();
  }

  // Convert the data to actual temperature
  int16_t raw1 = (data1[1] << 8) | data1[0];
  if (type_s1) {
    // 9 bit resolution default
    raw1 = raw1 << 3;
    if (data1[7] == 0x10) {
      raw1 = (raw1 & 0xFFF0) + 12 - data1[6];
    }
  } else {
    byte cfg1 = (data1[4] & 0x60);
    // 9 bit resolution, 93.75 ms
    if (cfg1 == 0x00) raw1 = raw1 & ~7;
    // 10 bit res, 187.5 ms
    else if (cfg1 == 0x20) raw1 = raw1 & ~3;
    // 11 bit res, 375 ms
    else if (cfg1 == 0x40) raw1 = raw1 & ~1;
  }
  
  currentTempDS = (float)raw1 / 16.0;
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
  lcd.begin();
  //lcd.init();
  lcd.backlight();
  
  pinMode(RELAY_PELTIER_PIN, OUTPUT);
  pinMode(RELAY_FAN1_PIN, OUTPUT);
  pinMode(RELAY_FAN2_PIN, OUTPUT);
  pinMode(RELAY_FAN3_PIN, OUTPUT);
  pinMode(RELAY_FAN4_PIN, OUTPUT);
  pinMode(RELAY_FAN5_PIN, OUTPUT);
  pinMode(RELAY_LAMP1_PIN, OUTPUT);
  pinMode(RELAY_LAMP2_PIN, OUTPUT);

  digitalWrite(RELAY_PELTIER_PIN, LOW);
  digitalWrite(RELAY_FAN1_PIN, LOW);
  digitalWrite(RELAY_FAN2_PIN, LOW);
  digitalWrite(RELAY_FAN3_PIN, LOW);
  digitalWrite(RELAY_FAN4_PIN, LOW);
  digitalWrite(RELAY_FAN5_PIN, LOW);
  digitalWrite(RELAY_LAMP1_PIN, LOW);
  digitalWrite(RELAY_LAMP2_PIN, LOW);
  
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi " + String(ssid));
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);

  deactivateCooling();
  deactivateHeating();
}

void loop() {
  if (sensorMillis >= sensorInterval) {
    temperature = dht.readTemperature();
    dsValue();
    delay(10);

    Serial.println("Suhu DHT : " + String(currentTempDHT) + "°C");
    Serial.println("Suhu DS  : " + String(currentTempDS) + "°C");

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
    lcd.print("TempDHT : " + String(currentTempDHT) + "°C");

    lcd.setCursor(0, 1);
    lcd.print("TempDS  : " + String(currentTempDS) + "°C");

    displayMillis = 0;
  }

  if (thingSpeakMillis >= thingSpeakInterval) {
    ThingSpeak.setField(1, currentTempDHT);
    ThingSpeak.setField(2, currentTempDS);

    int APIwait = ThingSpeak.writeFields(channelID, APIKey);
    
    if (APIwait == 200) {
      Serial.println("Update successful");
    } else {
     Serial.println("Error code: " + String(APIwait));
    }

    thingSpeakMillis = 0;
  }
}

