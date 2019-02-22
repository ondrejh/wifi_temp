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

// Bitmaps
#include "wemos-logo-84x48.h"
#include "wemos-logo-84x28.h"
#include "wemos-w-53x48.h"
#include "wemos-w-84x48.h"

// Pins
const int8_t RST_PIN = D2;
const int8_t CE_PIN = D1;
const int8_t DC_PIN = D6;
//const int8_t DIN_PIN = D7;  // Uncomment for Software SPI
//const int8_t CLK_PIN = D5;  // Uncomment for Software SPI
const int8_t BL_PIN = D0;


// Software SPI with explicit CE pin.
// Adafruit_PCD8544 display = Adafruit_PCD8544(CLK_PIN, DIN_PIN, DC_PIN, CE_PIN, RST_PIN);

// Software SPI with CE tied to ground. Saves a pin but other pins can't be shared with other hardware.
// Adafruit_PCD8544(int8_t CLK_PIN, int8_t DIN_PIN, int8_t DC_PIN, int8_t RST_PIN);

// Hardware SPI based on hardware controlled SCK (SCLK) and MOSI (DIN) pins. CE is still controlled by any IO pin.
// NOTE: MISO and SS will be set as an input and output respectively, so be careful sharing those pins!
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
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(tpadx,tpady);
  display.print(tstr);
  display.setCursor(hpadx,hpady);
  display.println(hstr);
  display.display();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  displayTemp(t, h);
  delay(5000);
}
