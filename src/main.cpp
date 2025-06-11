#include <Arduino.h> // Required for PlatformIO (C++)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define REED_PIN 4 // GPIO connected to reed switch
#define LED_PIN 2  // GPIO connected to LED (e.g., onboard LED)

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
  String deviceName = "Device_" + GetEspUniqueId();
  print(deviceName);
  BLEDevice::init(deviceName.c_str()); // Convert String to const char*
  // BLEDevice::init("Device_" + GetEspUniqueId());
  BLEServer *pServer = BLEDevice::createServer();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  print("Ble Advertising Started...");
}

void setup()
{
  // pinMode(REED_PIN, INPUT_PULLUP); // Reed switch between GND and pin
  // pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  // Serial.println("Reed Switch Door Sensor Started");

  // BLEDevice::init("Myesp32");
  // BLEServer *pServer = BLEDevice::createServer();

  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  // pAdvertising->start();

  // print("Ble Advertising Started...");

  AdvertiseBLE();
}

void loop()
{
  // int reedState = digitalRead(REED_PIN);

  // if (reedState == HIGH) {
  //   // Door is OPEN
  //   digitalWrite(LED_PIN, HIGH);
  //   Serial.println("Door OPEN");
  // } else {
  //   // Door is CLOSED
  //   digitalWrite(LED_PIN, LOW);
  //   Serial.println("Door CLOSED");
  // }

  // Serial.println("MicroController Working...");
  // delay(200); // Small delay for stability
}
