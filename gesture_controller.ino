#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);

typedef struct {
  char cmd;      
  float angle;   
} ControlPacket;

ControlPacket data;

uint8_t receiverMAC[] = {};  //need to fill this

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

  data.cmd = 'S';
  data.angle = 0;

 
  if (angleY > 5) {
    data.cmd = 'F';
    data.angle = angleY;
  }
  else if (angleY < -5) {
    data.cmd = 'B';
    data.angle = -angleY;
  }

  else if (angleX > 5) {
    data.cmd = 'R';
    data.angle = angleX;
  }
  else if (angleX < -5) {
    data.cmd = 'L';
    data.angle = -angleX;
  }

  data.angle = constrain(data.angle, 0, 25);

  esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));

  Serial.print("Cmd: ");
  Serial.print(data.cmd);
  Serial.print("  Angle: ");
  Serial.println(data.angle);

  delay(20);
}
