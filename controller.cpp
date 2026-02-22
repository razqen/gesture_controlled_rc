#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);

typedef struct {
  float x;
  float y;
} ControlPacket;

ControlPacket data;

uint8_t receiverMAC[] = {

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

  float angleX = mpu6050.getAngleX();
  float angleY = mpu6050.getAngleY();

  data.x = angleX;
  data.y = angleY;

  esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));

  Serial.print("X: ");
  Serial.print(data.x);
  Serial.print("  Y: ");
  Serial.println(data.y);

  delay(20);
}
