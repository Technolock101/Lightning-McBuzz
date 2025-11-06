#define LDR_LEFT 4
#define LDR_RIGHT 5

void setup() {
  Serial.begin(115200);

  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);

  Serial.println("Reading LDR values...");
}

void loop() {
  int leftValue = analogRead(LDR_LEFT);
  int rightValue = analogRead(LDR_RIGHT);

  Serial.print("Left LDR: ");
  Serial.print(leftValue);
  Serial.print("   |   Right LDR: ");
  Serial.println(rightValue);

  delay(300);  // Slow down printing
}
