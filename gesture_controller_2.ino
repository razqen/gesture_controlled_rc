#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

uint8_t receiverMAC[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC}; // Change this with the address that is on our MPU6050

bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

Quaternion q;                                                                                       //starting and initialising everything
VectorFloat gravity;
float ypr[3];   

char currentState = 'S';
char lastState = 'S';

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  devStatus = mpu.dmpInitialize();                                                                   //

  mpu.setXGyroOffset(220);     //change these values after tuning to our mpu6050 accordingly          //Configuring offsets that will cause issue in reading your hand movements by the dmp
  mpu.setYGyroOffset(76);      //change these values after tuning to our mpu6050 accordingly             
  mpu.setZGyroOffset(-85);     //change these values after tuning to our mpu6050 accordingly
  mpu.setZAccelOffset(1788);    //change these values after tuning to our mpu6050 accordingly         //

  if (devStatus == 0) {                                                                               //Checking if dmp is configured and ready to use
    mpu.setDMPEnabled(true);
    packetSize = mpu.dmpGetFIFOPacketSize();
    dmpReady = true;                                                                                  
  } else {
    Serial.println("DMP Init Failed");
    while (1);
  }                                                                                                   //

  WiFi.mode(WIFI_STA);                                                                                //
  esp_now_init();

  esp_now_peer_info_t peerInfo = {};                                                                  //here we initialise the whereabouts of the car i.e what we want to communicate with
  memcpy(peerInfo.peer_addr, receiverMAC, 6);                                                         //this is the who
  peerInfo.channel = 0;                                                                               //where
  peerInfo.encrypt = false;                                                                           //how
  esp_now_add_peer(&peerInfo);                                                                        //
}

void loop() {
  if (!dmpReady) return;

  fifoCount = mpu.getFIFOCount();                                                                       
  if (fifoCount < packetSize) return;                                                                  //if fifo does not contain a packet then exit the loop

  mpu.getFIFOBytes(fifoBuffer, packetSize);

  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

  float pitch = ypr[1] * 180 / M_PI;
  float roll  = ypr[2] * 180 / M_PI;

  if (pitch > 20) currentState = 'F';
  else if (pitch < -20) currentState = 'B';
  else if (roll > 20) currentState = 'R';
  else if (roll < -20) currentState = 'L';
  else currentState = 'S';

  if (currentState != lastState) {
    esp_now_send(receiverMAC, (uint8_t*)&currentState, sizeof(currentState));
    Serial.println(currentState);
    lastState = currentState;
  }

  delay(20);
}

