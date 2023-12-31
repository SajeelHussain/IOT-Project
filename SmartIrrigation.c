#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(32, 16, 2); // Adjust the I2C address if needed

#define moisturepin A0
#define temppin A1
#define pump 7

int moisturevalue = 0;
float tempvalue = 0;

// WiFi settings
String ssid     = "Simulator Wifi";  // SSID to connect to
String password = ""; // Our virtual wifi has no password (so dont do your banking stuff on this network)
String host     = "api.thingspeak.com"; // Open Weather Map API
const int httpPort   = 80

// ThingSpeak API keys for moisture and temperature fields
String moistureApiKey = "TGBLJ2RJ8D28AQTD";
String tempApiKey = "CQ2NZS9QKKO9ZKMN";
String voltageApiKey = "0Y7PX0LJPPBPZ4L2";
String pumpApiKey = "J882IERKRMQ3X6K1";

String moistureUri = "/update?api_key=" + moistureApiKey + "&field1=";
String tempUri = "/update?api_key=" + tempApiKey + "&field2=";
String voltageUri = "/update?api_key=" + voltageApiKey + "&field3=";
String pumpUri = "/update?api_key=" + pumpApiKey + "&field4=";

void setupESP8266(void) {
  Serial.begin(115200);
  Serial.println("AT");
  delay(10);
  if (!Serial.find("OK")) return;

  Serial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
  delay(10);
  if (!Serial.find("OK")) return;

  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(50);
  if (!Serial.find("OK")) return;
}

void sendData(int data, String uri) {
  String httpPacket = "GET " + uri + String(data) + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
  int length = httpPacket.length();

  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10);

  Serial.print(httpPacket);
  delay(10);
  if (!Serial.find("SEND OK\r\n")) return;
}

void setup()
{
  Serial.begin(9600);
  pinMode(pump, OUTPUT);
  pinMode(moisturepin, INPUT);
  pinMode(temppin, INPUT);

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Soil Moisture");
  lcd.clear();

  // Connect to WiFi and ThingSpeak
  setupESP8266();
}

void loop()
{
  tempvalue = analogRead(temppin);
  int temp = (tempvalue - 20) * 3.04;
  int celsius = map(temp, 0, 1023, -40, 125);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(celsius));

  // Send temperature data to ThingSpeak
  sendData(celsius, tempUri);

  delay(2000);

  moisturevalue = analogRead(moisturepin);
  moisturevalue = (100 - ((moisturevalue / 1023.00) * 100));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisturevalue);

  // Send moisture data to ThingSpeak
  sendData(moisturevalue, moistureUri);

  delay(2000);

  // Read voltage and pump state
  int voltage = analogRead(A2); // Assuming the voltage is connected to A2
  int pumpState = digitalRead(pump);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voltage: ");
  lcd.print(voltage);

  // Send voltage data to ThingSpeak
  sendData(voltage, voltageUri);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump State: ");
  lcd.print(pumpState);

  // Send pump state data to ThingSpeak
  sendData(pumpState, pumpUri);

  delay(2000);

  // Your existing moisture control logic
  if (moisturevalue <= 60)
  {
    digitalWrite(pump, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(moisturevalue);
    lcd.setCursor(0, 1);
    lcd.print("Pump: ON");
    Serial.println("Soil Humidity = " + String(moisturevalue) + "% - Pump ON");
  }
  else
  {
    digitalWrite(pump, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(moisturevalue);
    lcd.setCursor(0, 1);
    lcd.print("Pump: OFF");
    Serial.println("Soil Humidity = " + String(moisturevalue) + "% - Pump OFF");
  }

  delay(2000);
}
