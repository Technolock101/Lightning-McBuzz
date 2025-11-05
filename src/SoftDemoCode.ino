#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define I2C_SDA 6
#define I2C_SCL 7

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

// Threshold for detecting black
#define BLACK_THRESHOLD 2000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
const char* phrases[] = {"I am speed", "Kachow!", "Lightning McQueen"};
const int numPhrases = 3;
int phraseIndex = 0;

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_SLEEP, OUTPUT);

  // Motor A
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);

  // Motor B
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  // LDRs
  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);

  // Keep motor driver awake
  digitalWrite(MOTOR_SLEEP, HIGH);

  Wire.begin(I2C_SDA, I2C_SCL);  // AVR boards use fixed hardware I2C pins
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void loop() {
  int leftValue = analogRead(LDR_LEFT);
  int rightValue = analogRead(LDR_RIGHT);

  Serial.print("LDR LEFT: ");
  Serial.print(leftValue);
  Serial.print(" | LDR RIGHT: ");
  Serial.println(rightValue);

  bool leftBlack = leftValue < BLACK_THRESHOLD;
  bool rightBlack = rightValue < BLACK_THRESHOLD;

  if (leftBlack && rightBlack) {
    // Both on black → move forward both motors
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);
  } else if (leftBlack && !rightBlack) {
    // Left black, right light → Left backward, Right forward
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, HIGH);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);
  } else if (!leftBlack && rightBlack) {
    // Left light, right black → Left forward, Right backward
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, HIGH);
    digitalWrite(MOTOR_B_IN2, LOW);
  } else {
    // Both light → stop both motors
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
  }

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println(phrases[phraseIndex]);
  display.display();
 
  phraseIndex = (phraseIndex + 1) % numPhrases;

  delay(100); // small loop delay
}
