#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);  // Station mode is required to read the MAC address
  delay(500);           // Give the WiFi driver time to start

  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);

  Serial.println();
  Serial.print("MAC address: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");  // Keep two digits per byte
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

void loop() {
  // Nothing to do here
}