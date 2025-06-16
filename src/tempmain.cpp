#include <Arduino.h>

#define REED_PIN 27 // Reed switch (LOW when door closed)
#define PIR_PIN 33  // PIR sensor (HIGH when motion detected)
#define LED_PIN 2

// Status LED

// Timing constants
const unsigned long MOTION_TIMEOUT = 30000;     // 30 sec no motion = energy save
const unsigned long BLINK_INTERVAL = 500;       // 500ms blink speed
const unsigned long DOOR_ALERT_DURATION = 5000; // 5 sec LED on for door alerts

// System state
unsigned long lastMotionTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long doorAlertTime = 0;
bool ledState = LOW;
bool energySaveActive = false;

void setup()
{
    pinMode(REED_PIN, INPUT_PULLUP);
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println("\n🚪 SMART LAB MONITORING SYSTEM");
    Serial.println("----------------------------------");
    Serial.println("DOOR STATUS  | OCCUPANCY  | ENERGY MODE");
    Serial.println("----------------------------------");
}

void loop()
{
    bool isDoorClosed = (digitalRead(REED_PIN) == LOW);
    bool isMotionDetected = (digitalRead(PIR_PIN) == HIGH);
    unsigned long currentMillis = millis();

    // Update motion detection
    if (isMotionDetected)
    {
        lastMotionTime = currentMillis;
        if (energySaveActive)
        {
            energySaveActive = false;
            Serial.println("\n✅ Motion detected - Energy mode OFF");
            Serial.println("----------------------------------");
        }
    }

    // Check energy save condition
    bool shouldEnergySave = (currentMillis - lastMotionTime > MOTION_TIMEOUT);

    // Energy save activation
    if (shouldEnergySave && !energySaveActive)
    {
        energySaveActive = true;
        Serial.println("\n🚨 ENERGY SAVING MODE ACTIVATED!");
        Serial.println("--> ACTION REQUIRED: Switch OFF lights and AC");
        Serial.println("----------------------------------");
    }

    // Door state changes
    static bool lastDoorState = !isDoorClosed;
    if (isDoorClosed != lastDoorState)
    {
        lastDoorState = isDoorClosed;
        Serial.println(isDoorClosed ? "✅ Door CLOSED" : "🚪 Door OPENED");

        // Trigger door alert LED
        if (!isDoorClosed)
        {
            doorAlertTime = currentMillis;
        }
    }

    // LED control
    if (energySaveActive)
    {
        // Blink LED during energy save
        if (currentMillis - lastBlinkTime > BLINK_INTERVAL)
        {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
            lastBlinkTime = currentMillis;
        }
    }
    else if (!isDoorClosed && (currentMillis - doorAlertTime < DOOR_ALERT_DURATION))
    {
        // Solid LED for door alerts
        digitalWrite(LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
    }

    // Periodic status update
    static unsigned long lastStatusUpdate = 0;
    if (currentMillis - lastStatusUpdate > 5000)
    {
        lastStatusUpdate = currentMillis;
        Serial.print(isDoorClosed ? "🚪 CLOSED    | " : "🚪 OPEN      | ");
        Serial.print(isMotionDetected ? "👥 OCCUPIED | " : "👤 EMPTY    | ");
        Serial.println(energySaveActive ? "🔴 ENERGY SAVE" : "🟢 NORMAL");
    }

    delay(100);
}