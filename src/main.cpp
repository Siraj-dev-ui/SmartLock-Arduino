#include <Arduino.h>  // Required for PlatformIO (C++)

#define REED_PIN 4   // GPIO connected to reed switch
#define LED_PIN 2    // GPIO connected to LED (e.g., onboard LED)

void setup() {
  // pinMode(REED_PIN, INPUT_PULLUP); // Not needed for now
  // pinMode(LED_PIN, OUTPUT);        // Not needed for now
  Serial.begin(115200);
  Serial.println("Reed Switch Door Sensor Started");
}

void loop() {
  Serial.println("esp32 working");
  delay(200); // Small delay for stability
}
