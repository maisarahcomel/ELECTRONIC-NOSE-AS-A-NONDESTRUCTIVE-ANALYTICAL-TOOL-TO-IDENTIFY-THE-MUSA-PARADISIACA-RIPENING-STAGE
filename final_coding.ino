#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 28800;
const int daylightOffset_sec = 0;

// WiFi credentials
const char* ssid = "Maisarah";      // change SSID
const char* password = "stevecomel";  // change password

// Set the LCD address (refer to the datasheet for your specific module)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int mq2 = 34; // Change to the appropriate pin number for MQ2 sensor
const int mq4 = 35; // Change to the appropriate pin number for MQ4 sensor

// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycby_nUYjHuSJgVp0c7iRNcYsbyxPyvgER1H6OILHzvpEtB2IHwF4iafUbyu9I546EKtw";  // change Gscript ID
//https://script.google.com/macros/s/AKfycby_nUYjHuSJgVp0c7iRNcYsbyxPyvgER1H6OILHzvpEtB2IHwF4iafUbyu9I546EKtw/exec

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.print("E-Nose");
  lcd.setCursor(0, 1);
  lcd.print("Fruit Analyser");
  delay(3000);
  lcd.clear();

  lcd.print("By");
  lcd.setCursor(0, 1);
  lcd.print("Maisarah");
  delay(3000);
  lcd.clear();

  pinMode(mq4, INPUT);
  pinMode(mq2, INPUT);

  Serial.begin(9600);

  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  int mq2_value = analogRead(mq2);
  int mq4_value = analogRead(mq4);

  Serial.println("");
  Serial.print("MQ-2 Value: ");
  Serial.print(mq2_value);
  Serial.println(" PPM");
  Serial.print("MQ-4 Value: ");
  Serial.print(mq4_value);
  Serial.println(" PPM");
  Serial.println("\n\n");

  sendSheet(mq2_value, mq4_value);

  lcd.setCursor(0, 0);
  lcd.print("ETHYLENE VALUE:");
  lcd.setCursor(0, 1);
  lcd.print(mq2_value);
  delay(3000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("METHANE VALUE:");
  lcd.setCursor(0, 1);
  lcd.print(mq4_value);
  delay(3000);
  lcd.clear();
}

void sendSheet(int mq2, int mq4) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStringBuff[50];  // 50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%B-%d-%Y-%H:%M:%S", &timeinfo);

  String asString(timeStringBuff);

  int firstHyphenIndex = asString.indexOf("-");
  int secondHyphenIndex = asString.indexOf("-", firstHyphenIndex + 1);
  int thirdHyphenIndex = asString.indexOf("-", secondHyphenIndex + 1);

  String month = asString.substring(0, firstHyphenIndex);
  String dayYear = asString.substring(firstHyphenIndex + 1, secondHyphenIndex);
  String Year = asString.substring(secondHyphenIndex + 1, thirdHyphenIndex);
  String time = asString.substring(thirdHyphenIndex + 1);

  Serial.print("Date: ");
  Serial.println(month + "-" + dayYear + "-" + Year);  // Print Date
  Serial.print("Time: ");
  Serial.println(time);  // Print Time
  String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "&date=" + month + "-" + dayYear + "-" + Year + "&time=" + time + "&mq2=" + mq2 + "&mq4=" + mq4;
  Serial.print("POST data to spreadsheet:");
  Serial.println(urlFinal);
  HTTPClient http;
  http.begin(urlFinal.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  //---------------------------------------------------------------------
  http.end();
}
