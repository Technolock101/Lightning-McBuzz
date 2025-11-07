// Reconfirming the setup for ESP32-C6 and MAX98357A
// NOTE: Ensure the 'ESP8266Audio' library is installed via the Library Manager.
#include "Arduino.h" // Standard for setup/loop functions
#include "LittleFS.h" // ESP32 core uses LittleFS for the data directory
#include "AudioFileSourceID3.h"
#include "AudioFileSourceLittleFS.h" 
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// --- 1. I2S PIN CONFIGURATION ---
// These are the GPIO pins connected to the MAX98357A.
// (Using the user-specified pins 20, 21, 22)
#define I2S_BCLK_PIN    (GPIO_NUM_20) // Bit Clock (MAX98357A BCLK)
#define I2S_LRCK_PIN    (GPIO_NUM_21) // Left/Right Clock (MAX98357A LRC)
#define I2S_DOUT_PIN    (GPIO_NUM_22) // Digital Data Output (MAX98357A DIN/DOUT)

// --- 2. GLOBAL AUDIO OBJECTS ---
AudioGeneratorMP3 *mp3 = NULL; // Initialize pointers to NULL
AudioFileSourceLittleFS *file = NULL;
AudioOutputI2S *out = NULL;
AudioFileSourceID3 *id3 = NULL;

// --- 3. FILESYSTEM & FILE PATH ---
const char *mp3FilePath = "/tune.mp3.mp3"; 

// --- 4. AUDIO LIBRARY CALLBACK FUNCTIONS (Essential for debugging) ---
void audio_info(const char *info) { Serial.printf("INFO: %s\n", info); }
void audio_id3(const char *type, const char *str) { Serial.printf("ID3 TAG: %s = %s\n", type, str); }
void audio_eof_mp3(const char *info) { Serial.printf("INFO: EOF reached: %s\n", info); }

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ESP32-C6 MAX98357A MP3 Player ---");

    // 5. Initialize the Filesystem (Using simple LittleFS.begin() which is preferred)
    if (!LittleFS.begin()) {
      // The default LittleFS partition will automatically be used.
      Serial.println("FATAL: Failed to mount LittleFS. Did you run 'Upload Sketch Data'?");
      return;
    }
    Serial.println("LittleFS mounted successfully.");

    // Check if the MP3 file exists
    if (!LittleFS.exists(mp3FilePath)) {
      Serial.printf("FATAL: File not found at path: %s\n", mp3FilePath);
      // Corrected file name in the error message
      Serial.println("Ensure your file is named 'tune.mp3.mp3' and the data was uploaded successfully.");
      return;
    }
    Serial.printf("Found MP3 file: %s\n", mp3FilePath);


    // 6. Initialize I2S Output
    out = new AudioOutputI2S();
    
    // Set the specific GPIO pins for the I2S output
    out->SetPinout(I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN);

    // MAX98357A is a mono amplifier.
    out->SetChannels(1); 
    out->SetGain(0.5); // Set volume (0.0 to 1.0)
    
    // 7. Initialize Audio Generator and Connect Pipeline
    file = new AudioFileSourceLittleFS(mp3FilePath);
    // id3 object wraps the file object, which is needed by the mp3 generator
    id3 = new AudioFileSourceID3(file);
    mp3 = new AudioGeneratorMP3();

    // Connect the pipeline: File -> ID3 -> MP3 Decoder -> I2S Output
    if (mp3->begin(id3, out)) {
        Serial.println("Audio playback successfully initiated.");
    } else {
        Serial.println("FATAL: Failed to start MP3 generator. Check pinout and library version.");
    }
}

void loop() {
    // 8. The essential loop function
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
        // This ensures the file is reopened and the stream pointer is at byte 0.
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
      // If the audio is not running, delay briefly.
      delay(100); 
    }
}