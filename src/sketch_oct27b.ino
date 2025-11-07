// Reconfirming the setup for ESP32-C6 and MAX98357A
// NOTE: Ensure the 'ESP8266Audio', 'Adafruit GFX', and 'Adafruit SSD1306' libraries are installed.
#include "Arduino.h" // Standard for setup/loop functions
#include "LittleFS.h" // ESP32 core uses LittleFS for the data directory

/*
// Audio Libraries
#include "AudioFileSourceID3.h"
#include "AudioFileSourceLittleFS.h" 
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
*/

// Robot Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// --- 1. PIN CONFIGURATION ---

/*
// 1A. I2S Audio Pins (MAX98357A)
#define I2S_BCLK_PIN    (GPIO_NUM_20) // Bit Clock (MAX98357A BCLK)
#define I2S_LRCK_PIN    (GPIO_NUM_21) // Left/Right Clock (MAX8357A LRC)
#define I2S_DOUT_PIN    (GPIO_NUM_22) // Digital Data Output (MAX98357A DIN/DOUT)
*/

// 1B. Robot Pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET      -1
#define I2C_SDA 6
#define I2C_SCL 7

#define LDR_LEFT 5
#define LDR_RIGHT 4
#define MOTOR_SLEEP 8

// Motor A (Left Motor)
#define MOTOR_A_IN1 10 
#define MOTOR_A_IN2 11 

// Motor B (Right Motor)
#define MOTOR_B_IN1 2 
#define MOTOR_B_IN2 3 

#define BLACK_THRESHOLD_Left 1400 // LDR value below which is considered 'black'
#define BLACK_THRESHOLD_Right 800 // LDR value below which is considered 'black'

// --- SPEED DEFINITIONS ---
#define JUMP_SPEED 180      // (0 - 255) High-power burst for jump-starting
#define RUN_SPEED 140      // (0 - 255) Slower continuous speed for forward and pivot
#define JUMP_DURATION 10    // (ms) How long to apply the jumpstart burst


// --- 2. GLOBAL AUDIO OBJECTS ---
/*
AudioGeneratorMP3 *mp3 = NULL; 
AudioFileSourceLittleFS *file = NULL;
AudioOutputI2S *out = NULL;
AudioFileSourceID3 *id3 = NULL;
*/

// --- 2B. GLOBAL ROBOT OBJECTS ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* phrases[] = {"I am speed", "Kachow!", "Lightning McQueen"};
const int numPhrases = 3;
int phraseIndex = 0;

// --- 2C. GLOBAL ROBOT STATE ---
// These variables track the motor state to enable jump-starting
bool motorA_isRunning = false;
bool motorB_isRunning = false;


// --- 3. FILESYSTEM & FILE PATH ---
// const char *mp3FilePath = "/tune.mp3.mp3"; 

// --- 4. AUDIO LIBRARY CALLBACK FUNCTIONS (Essential for debugging) ---
/*
void audio_info(const char *info) { Serial.printf("INFO: %s\n", info); }
void audio_id3(const char *type, const char *str) { Serial.printf("ID3 TAG: %s = %s\n", type, str); }
void audio_eof_mp3(const char *info) { Serial.printf("INFO: EOF reached: %s\n", info); }
*/


// --- 5. MOTOR HELPER FUNCTIONS ---
// These functions replace analogWrite() and add the jumpstart logic

/**
 * @brief Sets the speed for Motor A (Left) with jumpstart logic.
 * @param speed The desired continuous speed (0-255). 0 means stop.
 */
void setMotorA(int speed) {
    if (speed > 0 && !motorA_isRunning) {
        // Motor is starting from a stop. Jumpstart it!
        // We use JUMP_SPEED (180) as the high-power burst
        analogWrite(MOTOR_A_IN1, JUMP_SPEED); 
        analogWrite(MOTOR_A_IN2, 0);
        delay(JUMP_DURATION); // Short blocking delay for the burst
    }

    // Now set the desired continuous speed
    if (speed > 0) {
        analogWrite(MOTOR_A_IN1, speed);
        analogWrite(MOTOR_A_IN2, 0);
        motorA_isRunning = true;
    } else {
        // Speed is 0, so stop the motor
        analogWrite(MOTOR_A_IN1, 0);
        analogWrite(MOTOR_A_IN2, 0);
        motorA_isRunning = false;
    }
}

/**
 * @brief Sets the speed for Motor B (Right) with jumpstart logic.
 * @param speed The desired continuous speed (0-255). 0 means stop.
 */
void setMotorB(int speed) {
    if (speed > 0 && !motorB_isRunning) {
        // Motor is starting from a stop. Jumpstart it!
        // We use JUMP_SPEED (180) as the high-power burst
        analogWrite(MOTOR_B_IN1, 0); 
        analogWrite(MOTOR_B_IN2, JUMP_SPEED); // Note: IN2 is forward for Motor B
        delay(JUMP_DURATION); // Short blocking delay for the burst
    }

    // Now set the desired continuous speed
    if (speed > 0) {
        analogWrite(MOTOR_B_IN1, 0);
        analogWrite(MOTOR_B_IN2, speed);
        motorB_isRunning = true;
    } else {
        // Speed is 0, so stop the motor
        analogWrite(MOTOR_B_IN1, 0);
        analogWrite(MOTOR_B_IN2, 0);
        motorB_isRunning = false;
    }
}


void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ESP32-C6 Combined Robot & MP3 Player ---");
    // Serial.printf("I2S Pinout: BCLK=%d, LRCK=%d, DOUT=%d\n", I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN);

    // --- ROBOT HARDWARE INITIALIZATION ---
    Serial.println("Initializing Robot Hardware...");
    pinMode(MOTOR_SLEEP, OUTPUT);
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_IN1, OUTPUT);
    pinMode(MOTOR_B_IN2, OUTPUT);
    pinMode(LDR_LEFT, INPUT);
    pinMode(LDR_RIGHT, INPUT);
    digitalWrite(MOTOR_SLEEP, HIGH); // Enable motor driver

    // I2C/OLED Setup
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("FATAL: SSD1306 allocation failed"));
        return; 
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("C6 Robot Initialized");
    // display.println("LFS & Audio Starting...");
    display.display();

    /*
    // --- AUDIO INITIALIZATION ---
    // 5. Initialize the Filesystem
    if (!LittleFS.begin()) {
      Serial.println("FATAL: Failed to mount LittleFS. Did you run 'Upload Sketch Data'?");
      return;
    }
    Serial.println("LittleFS mounted successfully.");

    // Check if the MP3 file exists
    if (!LittleFS.exists(mp3FilePath)) {
      Serial.printf("FATAL: File not found at path: %s\n", mp3FilePath);
      Serial.println("Ensure your file is named 'tune.mp3.mp3' and the data was uploaded successfully.");
      return;
    }
    Serial.printf("Found MP3 file: %s\n", mp3FilePath);


    // 6. Initialize I2S Output
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN);
    out->SetChannels(1); 
    out->SetGain(0.5); // Set volume (0.0 to 1.0)
    
    // 7. Initialize Audio Generator and Connect Pipeline
    file = new AudioFileSourceLittleFS(mp3FilePath);
    id3 = new AudioFileSourceID3(file);
    mp3 = new AudioGeneratorMP3();

    // Connect the pipeline: File -> ID3 -> MP3 Decoder -> I2S Output
    if (mp3->begin(id3, out)) {
        Serial.println("Audio playback successfully initiated.");
    } else {
        Serial.println("FATAL: Failed to start MP3 generator. Check pinout and library version.");
    }
    */
    
    Serial.println("Initialization Complete. Starting Loop.");
}

void loop() {
    unsigned long currentMillis = millis();

    // --- 8A. ROBOT LOGIC (NON-BLOCKING) ---
    int leftValue = analogRead(LDR_LEFT);
    int rightValue = analogRead(LDR_RIGHT);

    bool leftBlack = leftValue < BLACK_THRESHOLD_Left;
    bool rightBlack = rightValue < BLACK_THRESHOLD_Right;
    
    // --- NEW "STRADDLE" LOGIC (Corrected) ---
    
    if (leftBlack && !rightBlack) {
        // Drifting right, LEFT sensor hit the line.
        // Correct by turning RIGHT.
        setMotorA(0); 
        setMotorB(RUN_SPEED); 
    }
    else if (!leftBlack && rightBlack) {
        // Drifting left, RIGHT sensor hit the line.
        // Correct by turning LEFT.
        setMotorA(RUN_SPEED);
        setMotorB(0); 
    }
    else if (!leftBlack && !rightBlack) {
        // Both sensors see white (straddling the line).
        // This is the ideal state: Go Forward.
        setMotorA(RUN_SPEED);
        setMotorB(RUN_SPEED);
    }
    else {
        // Both sensors see black (leftBlack && rightBlack)
        // This means we've probably overshot a sharp turn.
        // STOP to prevent running off.
        setMotorA(0);
        setMotorB(0);
    }


    // --- 8B. OLED DISPLAY UPDATE (NON-BLOCKING) ---
    static unsigned long lastDisplay = 0;
    if (currentMillis - lastDisplay > 500) {
        lastDisplay = currentMillis;
        
        // Debug data to serial
        Serial.printf("LDR: L=%4d (%s), R=%4d (%s)\n", // | MP3: %s\n", 
                      leftValue, leftBlack ? "Black" : "White", 
                      rightValue, rightBlack ? "Black" : "White"
                      //, mp3 && mp3->isRunning() ? "Playing" : "Stopped"
                      );

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.printf("L:%4d / R:%4d\n", leftValue, rightValue);
        display.setCursor(0, 10);
        
        // Update status text for new logic
        if (!leftBlack && !rightBlack) {
            display.println("Status: FOLLOWING");
        } else if (leftBlack && rightBlack) {
            display.println("Status: LOST (STOP)");
        } else {
            display.println("Status: CORRECTING");
        }

        // display.setCursor(0, 20);
        // display.printf("Audio: %s\n", mp3 && mp3->isRunning() ? "Playing" : "Stopped");
        display.setCursor(0, 40);
        display.setTextSize(2);
        display.println(phrases[phraseIndex]);
        display.display();
        
        phraseIndex = (phraseIndex + 1) % numPhrases;
    }


    /*
    // --- 8C. AUDIO LOGIC (CRITICAL FOR PLAYBACK) ---
    if (mp3 && mp3->isRunning()) {
      // Keep running the audio stream
      if (!mp3->loop()) {
        // --- AGGRESSIVE GUARANTEED LOOPING LOGIC ---
        Serial.println("INFO: MP3 finished playing. Resetting entire audio pipeline for robust looping.");
        
        // 1. Safely stop and delete ALL components of the file source chain and generator
        mp3->stop();
        delete mp3;
        delete id3; // Important: Deleting the ID3 wrapper
        delete file; // Critical: Deleting the file source to close the handle
        
        mp3 = NULL;
        id3 = NULL;
        file = NULL;

        // 2. Re-create the entire file source chain
        file = new AudioFileSourceLittleFS(mp3FilePath);
        id3 = new AudioFileSourceID3(file);
        
        // 3. Re-create the MP3 generator and restart playback
        mp3 = new AudioGeneratorMP3();
        if (mp3->begin(id3, out)) {
             Serial.println("INFO: Loop restart successful.");
        } else {
             Serial.println("FATAL: Loop restart failed.");
        }
        // ------------------------------------------
      }
    } else {
      // If the audio is not running, delay briefly to allow other tasks to run.
      delay(1); 
    }
    */
}