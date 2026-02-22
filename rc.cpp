#include <WiFi.h>
#include <esp_now.h>

#define ENA 25
#define IN1 26
#define IN2 18
#define ENB 14
#define IN3 32
#define IN4 13
#define MIN_ANGLE 10.0
#define MAX_ANGLE 35.0
#define MIN_SPEED 550     
#define MAX_SPEED 1023

typedef struct {
  float x;   
  float y;   
} ControlPacket;

ControlPacket data;

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}
void loop(){}

void onReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(ControlPacket)) return;
  memcpy(&data, incomingData, sizeof(data));
  Serial.print("X: ");
  Serial.print(data.x);
  Serial.print("  Y: ");
  Serial.println(data.y);
  drive(data.x, data.y);
}

int angleToSpeed(float angle) {
  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
  return map(angle * 10, MIN_ANGLE * 10, MAX_ANGLE * 10, MIN_SPEED, MAX_SPEED);
}

void drive(float x, float y) {
  float ax = abs(x);
  float ay = abs(y);
  if (ax < 5.0 && ay < 5.0) {   
    stopMotors();
    return;
  }
  int speed = angleToSpeed(max(ax, ay));
  if (ay > ax) {
    if (y > 0) forward(speed);
    else       backward(speed);
  } 
  else {
    if (x > 0) right(speed);
    else       left(speed);
  }
}

void backward(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void forward(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left(int spd) {
  analogWrite(ENA, spd / 3);
  analogWrite(ENB, spd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void right(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd / 3);
  digitalWrite(IN1, LOW);   
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);  
  digitalWrite(IN4, LOW);
}


void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
