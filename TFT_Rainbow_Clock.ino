#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

const char *ssid     = "Sayantan";
const char *password = "sayantan";

const long utcOffsetInSeconds = 19800;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
String date = "";

void setup(void) {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  targetTime = millis() + 1000;

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = ("http://customprojects.000webhostapp.com/date.php"); //your website link in this format
    Serial.println(url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      // start of fetching get process
      date = http.getString();
      Serial.println(date);

    }
    else
      Serial.println(httpCode);
    http.end();
    delay(500);
  }

  timeClient.begin();
}

void loop() {
  if (targetTime < millis()) {
    targetTime = millis() + 30000;

    // Colour changing state machine
    for (int i = 0; i < 160; i++) {
      tft.drawFastVLine(i, 0, tft.height(), colour);
      switch (state) {
        case 0:
          green += 2;
          if (green == 64) {
            green = 63;
            state = 1;
          }
          break;
        case 1:
          red--;
          if (red == 255) {
            red = 0;
            state = 2;
          }
          break;
        case 2:
          blue ++;
          if (blue == 32) {
            blue = 31;
            state = 3;
          }
          break;
        case 3:
          green -= 2;
          if (green == 255) {
            green = 0;
            state = 4;
          }
          break;
        case 4:
          red ++;
          if (red == 32) {
            red = 31;
            state = 5;
          }
          break;
        case 5:
          blue --;
          if (blue == 255) {
            blue = 0;
            state = 0;
          }
          break;
      }
      colour = red << 11 | green << 5 | blue;
    }

    timeClient.update();

    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.println(timeClient.getFormattedTime());

    // The new larger fonts do not use the .setCursor call, coords are embedded
    tft.setTextColor(TFT_BLACK, TFT_BLACK); // Do not plot the background colour

    // Overlay the black text on top of the rainbow plot (the advantage of not drawing the backgorund colour!)
    tft.drawCentreString("ESP32 NTP Clock", 80, 4, 2); // Draw text centre at position 80, 12 using font 2
    tft.drawCentreString(timeClient.getFormattedTime(), 80, 25, 6); // Draw text centre at position 80, 24 using font 4
    tft.drawCentreString(date, 80, 68, 4); // Draw text centre at position 80, 24 using font 6
    tft.drawCentreString(daysOfTheWeek[timeClient.getDay()], 80, 94, 4); // Draw text centre at position 80, 90 using font 2

    delay(1000);
  }
}
