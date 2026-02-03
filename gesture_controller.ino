#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);

uint8_t receiverMAC[] = {};  //need to fill this

char currentState = 'S';
char lastState = 'S';

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
}

void loop() {
  mpu6050.update();

  if (mpu6050.getAngleY() > 20) currentState = 'F';
  else if (mpu6050.getAngleY() < -20) currentState = 'B';
  else if (mpu6050.getAngleX() > 20) currentState = 'R';
  else if (mpu6050.getAngleX() < -20) currentState = 'L';
  else currentState = 'S';

  if (currentState != lastState) {
    esp_now_send(receiverMAC, (uint8_t*)&currentState, sizeof(currentState));
    Serial.println(currentState);
    lastState = currentState;
  }

  delay(20);
}
