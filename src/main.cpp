#include "secrets.h"

#include <Arduino.h> // Required for PlatformIO (C++)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <WiFi.h>
#include <HTTPClient.h>
// #include <ArduinoJson.h>

#define REED_PIN 27   // Reed switch (LOW when door closed)
#define PIR_PIN 33    // PIR sensor (HIGH when motion detected)
#define LED_PIN 23    // Status LED
#define LED_LOCK 2    // Status LOCKED
#define LED_UNLOCK 16 // Status UNLOCKED

// Status LED

// Timing constants
const unsigned long MOTION_TIMEOUT = 15000;     // 30 sec no motion = energy save
const unsigned long BLINK_INTERVAL = 500;       // 500ms blink speed
const unsigned long DOOR_ALERT_DURATION = 5000; // 5 sec LED on for door alerts

// System state
unsigned long lastMotionTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long doorAlertTime = 0;
bool ledState = LOW;
bool energySaveActive = false;

// home
uint8_t activeAction = 0;

void print(String message)
{
  Serial.println(message);
}

String GetEspUniqueId()
{
  uint64_t chipid = ESP.getEfuseMac();  // Returns the MAC address (48 bits)
  char uniqueID[13];                    // 12 characters + null terminator
  sprintf(uniqueID, "%012llX", chipid); // Convert to hex string

  return String(uniqueID); // Return as String
}

void AdvertiseBLE()
{
  // Get unique ID
  // String deviceName = "Device_" + GetEspUniqueId();
  // print(deviceName);
  // BLEDevice::init(deviceName.c_str()); // Convert String to const char*

  // BLEDevice::init(DEVICE_ID);
  BLEDevice::init("Device_F073AF6CDDA0");

  // BLEServer *pServer = BLEDevice::createServer();
  BLEDevice::createServer();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  print("Ble Advertising Started...");
}

void ConnectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

void MakeRequest(const String &url)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(url); // Specify the URL

    int httpResponseCode = http.GET(); // Make GET request

    if (httpResponseCode > 0)
    {
      // Serial.println("Active Action ...");
      String payload = http.getString();
      // Serial.println("Response code: " + String(httpResponseCode));
      // Serial.println("Response payload: " + payload);
      activeAction = payload.toInt();
    }
    else
    {
      Serial.println("Error on HTTP request");
    }

    http.end(); // Free resources
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void SmartLockPostman(const String &url, int number)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Create JSON with number value
    String requestBody = String("{\"command\":") + String(number) + "}";

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println("POST success:");
      // Serial.println("Code: " + String(httpResponseCode));
      // Serial.println("Response: " + response);
    }
    else
    {
      Serial.println("POST failed. Error code: " + String(httpResponseCode));
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void setup()
{
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_LOCK, OUTPUT);
  pinMode(LED_UNLOCK, OUTPUT);

  Serial.begin(115200);
  Serial.println("\n🚪 SMART LAB MONITORING SYSTEM");
  Serial.println("----------------------------------");
  Serial.println("DOOR STATUS  | OCCUPANCY  | ENERGY MODE");
  Serial.println("----------------------------------");
  // get esp uniquie id

  // Serial.println(GetEspUniqueId());

  // calling api over wifi

  ConnectWifi();

  // bluetooth code
  AdvertiseBLE();
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
  // if (shouldEnergySave && !energySaveActive)
  if (shouldEnergySave)
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
      print("calling api");
      SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 3);
      doorAlertTime = currentMillis;
    }
    else
    {
      SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 4);
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
    // Serial.print(isDoorClosed ? "🚪 CLOSED    | " : "🚪 OPEN      | ");
    // Serial.println(energySaveActive ? "🔴 ENERGY SAVE" : "🟢 NORMAL");

    // print("empty check api called.");
    if (isMotionDetected)
    {
      Serial.print(isMotionDetected ? "👥 OCCUPIED | " : "👤 EMPTY    | ");

      SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 5);
    }
    else
    {
      SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 6);
    }
  }

  // Check Active Action

  MakeRequest(String(BASE_URL) + "/actions/active-action");

  // Active Action

  switch (activeAction)
  {
  case 1:
    Serial.println("Lock the door.");
    SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 1);
    digitalWrite(LED_UNLOCK, LOW);
    digitalWrite(LED_LOCK, HIGH);

    break;
  case 2:
    Serial.println("Unlock the door..");

    SmartLockPostman(String(BASE_URL) + "/actions/smartlock-postman", 2);
    digitalWrite(LED_LOCK, LOW);
    digitalWrite(LED_UNLOCK, HIGH);
    break;
  }
  delay(2000);
}
