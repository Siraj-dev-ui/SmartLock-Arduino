#include <Arduino.h> // Required for PlatformIO (C++)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <WiFi.h>
#include <HTTPClient.h>

#define REED_PIN 4 // GPIO connected to reed switch
#define LED_PIN 2  // GPIO connected to LED (e.g., onboard LED)

const char *ssid = "haider";
const char *password = "123456789";
const char *apiUrl = "https://smartlock-nestapi.onrender.com/doors/test";

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

  // for testing
  BLEDevice::init("sirajesp");

  BLEServer *pServer = BLEDevice::createServer();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  print("Ble Advertising Started...");
}

void WifiCall()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

void MakeRequest()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(apiUrl); // Specify the URL

    int httpResponseCode = http.GET(); // Make GET request

    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Response payload: " + payload);
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

  delay(10000); // Wait 10 seconds before next request
}

void setup()
{
  // pinMode(REED_PIN, INPUT_PULLUP); // Reed switch between GND and pin
  // pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  // Serial.println("Reed Switch Door Sensor Started");

  // bluetooth code
  // AdvertiseBLE();

  // calling api over wifi

  WifiCall();
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

  MakeRequest();
}
