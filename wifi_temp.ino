/* Hello WeMos
 * Displays a few WeMos bitmaps
 *
 * Connections:
 * WeMos D1 Mini   Nokia 5110    Description
 * (ESP8266)       PCD8544 LCD
 *
 * D2 (GPIO4)      0 RST         Output from ESP to reset display
 * D1 (GPIO5)      1 CE          Output from ESP to chip select/enable display
 * D6 (GPIO12)     2 DC          Output from display data/command to ESP
 * D7 (GPIO13)     3 Din         Output from ESP SPI MOSI to display data input
 * D5 (GPIO14)     4 Clk         Output from ESP SPI clock
 * 3V3             5 Vcc         3.3V from ESP to display
 * D0 (GPIO16)     6 BL          3.3V to turn backlight on, or PWM
 * G               7 Gnd         Ground
 * 
 * D4 DHT22 sensor
 * D3 sensor pullup
 *
 * Dependencies:
 * https://github.com/adafruit/Adafruit-GFX-Library
 * https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library
 * - This pull request adds ESP8266 support:
 * - https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library/pull/27
 */

#include <Arduino.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include <DHT.h>

#include <ESP8266WiFi.h>
const char WiFiAPPSK[] = "sparkfun";
WiFiServer server(80);

// Pins
const int8_t RST_PIN = D2;
const int8_t CE_PIN = D1;
const int8_t DC_PIN = D6;
//const int8_t DIN_PIN = D7;  // Uncomment for Software SPI
//const int8_t CLK_PIN = D5;  // Uncomment for Software SPI
const int8_t BL_PIN = D0;

Adafruit_PCD8544 display = Adafruit_PCD8544(DC_PIN, CE_PIN, RST_PIN);

DHT dht(D4, DHT21);

void setup() {
  Serial.begin(9600);
  Serial.println("\n\nWeMos D1 Mini + Nokia 5110 PCD8544 84x48 Monochrome LCD\nUsing Adafruit_PCD8544 and Adafruit_GFX libraries\n");

  // Turn LCD backlight on
  pinMode(BL_PIN, OUTPUT);
  digitalWrite(BL_PIN, HIGH);

  display.begin();
  display.setContrast(60);  // Adjust for your display
  Serial.println("Show Adafruit logo bitmap");

  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);
  dht.begin();

  // clear display to hide Adafruit logo preloaded by library
  display.clearDisplay();
  display.display();

  setupWiFi();
  server.begin();
}

void displayTemp(float t, float h) {
  char tstr[8];
  dtostrf(t, 0, 1, tstr);
  snprintf(&tstr[strlen(tstr)], 8, "%cC", char(247));
  int tpadx = (84 - strlen(tstr)*6*2) / 2;
  int tpady = (48 - 2*8*2) / 3;

  char hstr[8];
  dtostrf(h, 0, 0, hstr);
  snprintf(&hstr[strlen(hstr)], 8, "%");
  int hpadx = (84 - strlen(hstr)*6*2) / 2;
  int hpady = 48 - (8*2 + tpady);
  
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(tpadx,tpady);
  display.print(tstr);
  display.setCursor(hpadx,hpady);
  display.println(hstr);
  display.display();
}

void displayWifi() {
  char str[16];
  if (WiFi.status() == WL_CONNECTED)
    sprintf(str, "Connected");
  else
    sprintf(str, "Not Connected");
  int padx = (84 - strlen(str)*6) / 2;
  int pady = (48 - 3*8) / 2;

  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(padx,pady);
  display.print(str);

  pady += (2*8);
  displayShrinkIP(pady);
  display.display();
}

void loop() {
  uint32_t now = millis();

  static uint32_t disp_t=now, read_t=now-30000;
  static float t;
  static float h;
  static int show = 0;
  static int timeout = 0;
  
  if ((now - read_t) >= 30000) {
    read_t += 30000;
    t = dht.readTemperature();
    h = dht.readHumidity();
  }

  if ((now - disp_t) >= timeout) {
    disp_t += timeout;
    
    switch (show) {
    case 0:
      displayTemp(t, h);
      timeout = 5000;
      break;
    case 1:
      displayWifi();
      timeout = 2000;
      break;
    }
    
    show ++;
    show %= 2;
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    client.flush();
  
    // Prepare the response. Start with the common header:
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
    s += "Temperature = " + String(t, 1) + "&degC<br>\r\n";
    s += "Humidity = " + String(h, 0) + "%<br>\r\n";
    s += "</html>\r\n";
  
    // Send the response to the client
    client.print(s);
  }
  
  delay(1);
}

void displayShrinkIP(int pady) {
  char str[16];
  display.setTextSize(1);
  display.setTextColor(BLACK);
  sprintf(str, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  int padx = (84 - (strlen(str)*6 - 9)) / 2;
  for (int i=0; i<strlen(str); i++) {
    display.setCursor(padx, pady);
    if (str[i]=='.')
      padx += 5;
    else if (str[i+1]=='.')
      padx += 4;
    else
      padx += 6;
    display.print(str[i]);
  }  
}

void setupWiFi()
{
  WiFi.begin("rejnok", "r3jn0Knet");

  /*
  // INTERACTIVE CONNECTION DIALOG
  char str[16] = "Connecting";
  int padx = (84 - strlen(str)*6) / 2;
  int pady = (48 - 3*8) / 2;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(padx, pady);
  display.print(str);
  display.display();

  sprintf(str, ".....");
  padx = (84 - strlen(str)*6) / 2;
  pady += (8*2);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    for (int i = 0; i < 5; i++)
      str[i] = (i==cnt) ? '.' : ' ';
    cnt ++;
    cnt %= 5;
    display.setTextColor(BLACK);
    display.setCursor(padx, pady);
    display.print(str);
    display.display();
    delay(200);
    display.setCursor(padx, pady);
    display.setTextColor(WHITE);
    display.print(".....");
  }

  displayShrinkIP(pady);
  display.display();
  delay(5000); */


  /*
  // AP MODE SECTION
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);*/
}
