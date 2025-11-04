// --- (Your defines and global channel variables are all correct) ---
#define LED_PIN 22
#define LDR_LEFT 4
#define LDR_RIGHT 5

#define MOTOR_SLEEP 8
#define MOTOR_A_IN1 10
#define MOTOR_A_IN2 11
#define MOTOR_B_IN1 2
#define MOTOR_B_IN2 3

// --- PWM Setup for ESP32 ---
#define PWM_FREQ 5000     // 5kHz frequency
#define PWM_RESOLUTION 8  // 8-bit resolution (0-255)

const int KICK_START_MS = 25; 

// --- Global variables to store the channel numbers ---
uint8_t g_a_in1_channel;
uint8_t g_a_in2_channel;
uint8_t g_b_in1_channel;
uint8_t g_b_in2_channel;


void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_SLEEP, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  // --- Set up PWM and SAVE the returned channel ---
  g_a_in1_channel = ledcAttach(MOTOR_A_IN1, PWM_FREQ, PWM_RESOLUTION);
  g_a_in2_channel = ledcAttach(MOTOR_A_IN2, PWM_FREQ, PWM_RESOLUTION);
  g_b_in1_channel = ledcAttach(MOTOR_B_IN1, PWM_FREQ, PWM_RESOLUTION);
  g_b_in2_channel = ledcAttach(MOTOR_B_IN2, PWM_FREQ, PWM_RESOLUTION);

  // Start with motor off (Brake) and driver asleep
  digitalWrite(MOTOR_SLEEP, LOW);
  ledcWrite(g_a_in1_channel, 0);
  ledcWrite(g_a_in2_channel, 0);
  ledcWrite(g_b_in1_channel, 0);
  ledcWrite(g_b_in2_channel, 0);
}

void loop() {
  int value = analogRead(LDR_LEFT);
  Serial.print("LDR LEFT Value: ");
  Serial.println(value);

  if (value > 2500) {
    // --- Dark State: Stop the motor ---
    
    // Brake first (commands are ignored if driver is asleep, but good practice)
    ledcWrite(g_a_in1_channel, 0);
    ledcWrite(g_a_in2_channel, 0);
    ledcWrite(g_b_in1_channel, 0);
    ledcWrite(g_b_in2_channel, 0);
    
    // Put driver to sleep
    digitalWrite(MOTOR_SLEEP, LOW);

  } else {
    // --- Light State: Spin the motor ---
    
    // 1. Wake up the driver
    digitalWrite(MOTOR_SLEEP, HIGH);
    
    // 💡 THE FIX: Give the driver a moment to wake up!
    delay(5); // 5ms is more than enough
    
    // 2. KICK-START: (Motor A forward, Motor B backward)
    ledcWrite(g_a_in1_channel, 255); 
    ledcWrite(g_a_in2_channel, 0);
    ledcWrite(g_b_in1_channel, 0); 
    ledcWrite(g_b_in2_channel, 255);
    delay(KICK_START_MS); // Wait for the kick

    // 3. RUNNING SPEED: (Motor A forward 50%, Motor B backward 50%)
    ledcWrite(g_a_in1_channel, 127);
    ledcWrite(g_a_in2_channel, 0);
    ledcWrite(g_b_in1_channel, 0);
    ledcWrite(g_b_in2_channel, 127);
  }

  // General loop delay
  delay(250);
}