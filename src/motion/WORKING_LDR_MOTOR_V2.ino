// --- Pins ---
#define LDR_LEFT 4
#define MOTOR_SLEEP 8

// Motor A
#define MOTOR_A_IN1 10
#define MOTOR_A_IN2 11

// Motor B
#define MOTOR_B_IN1 2
#define MOTOR_B_IN2 3

// Threshold for detecting black
#define BLACK_THRESHOLD 2000

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_SLEEP, OUTPUT);

  // Motor A
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);

  // Motor B
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  // LDR
  pinMode(LDR_LEFT, INPUT);

  // Keep motor driver awake
  digitalWrite(MOTOR_SLEEP, HIGH);
}

void loop() {
  int leftValue = analogRead(LDR_LEFT);

  Serial.print("LDR LEFT: ");
  Serial.println(leftValue);

  bool leftBlack = leftValue > BLACK_THRESHOLD;

  if (leftBlack) {
    // Left on black → move forward both motors
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);
  } else {
    // Left not on black → stop both motors
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
  }

  delay(100); // small loop delay
}
