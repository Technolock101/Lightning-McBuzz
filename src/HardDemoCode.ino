#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// Safe I2C pins
#define I2C_SDA 21
#define I2C_SCL 22

// --- Pins ---
#define LDR_LEFT 4
#define LDR_RIGHT 5
#define MOTOR_SLEEP 8

// Motor A
#define MOTOR_A_IN1 10
#define MOTOR_A_IN2 11

// Motor B
#define MOTOR_B_IN1 2
#define MOTOR_B_IN2 3

// Button on safe GPIO
#define BUTTON_PIN 19   

#define BLACK_THRESHOLD 2000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* phrases[] = {"I am speed", "Kachow!", "Lightning McQueen"};
const int numPhrases = 3;
int phraseIndex = 0;

bool motorsEnabled = false;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_SLEEP, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Enable motor driver (if using sleep pin)
  digitalWrite(MOTOR_SLEEP, HIGH);

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Ready");
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();

  // --- BUTTON TOGGLE ---
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState) {
    lastDebounceTime = currentMillis;
  }
  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      motorsEnabled = !motorsEnabled;
      Serial.print("Motors: ");
      Serial.println(motorsEnabled ? "ON" : "OFF");
    }
  }
  lastButtonState = buttonState;

  // --- READ LDRs ---
  int leftValue = analogRead(LDR_LEFT);
  int rightValue = analogRead(LDR_RIGHT);

// --- MOTOR CONTROL ---
if (motorsEnabled) {
  bool leftBlack = leftValue < BLACK_THRESHOLD;
  bool rightBlack = rightValue < BLACK_THRESHOLD;

  if (leftBlack && !rightBlack) {
    // LEFT on black -> TURN LEFT
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);   // Left motor forward
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);   // Right motor stop
  }
  else if (!leftBlack && rightBlack) {
    // RIGHT on black -> TURN RIGHT
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);   // Left motor stop
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);  // Right motor forward
  }
  else {
    // BOTH BLACK or BOTH WHITE → MOVE FORWARD
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);   // Left motor forward
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);  // Right motor forward
  }
} 
else {
  // Motors OFF
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
}

  // --- OLED UPDATE every 500ms ---
  static unsigned long lastDisplay = 0;
  if (currentMillis - lastDisplay > 500) {
    lastDisplay = currentMillis;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(motorsEnabled ? "Motors ON" : "Motors OFF");
    display.setCursor(0, 30);
    display.println(phrases[phraseIndex]);
    display.display();
    phraseIndex = (phraseIndex + 1) % numPhrases;
  }
}
