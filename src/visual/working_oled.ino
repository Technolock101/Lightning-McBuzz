#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define I2C_SDA 6
#define I2C_SCL 7
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
const char* phrases[] = {"I am speed", "Kachow!", "Lightning McQueen"};
const int numPhrases = 3;
int phraseIndex = 0;
 
void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);  // AVR boards use fixed hardware I2C pins
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}
 
void loop() {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println(phrases[phraseIndex]);
  display.display();
 
  phraseIndex = (phraseIndex + 1) % numPhrases;
  delay(1000);
}