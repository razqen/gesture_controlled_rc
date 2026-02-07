#include <WiFi.h>
#include <esp_now.h>

#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 14
#define IN3 12
#define IN4 13
#define MIN_ANGLE 5.0
#define MAX_ANGLE 25.0
#define MIN_SPEED 80     
#define MAX_SPEED 255

char command = 'S';

typedef struct {
  char cmd;     
  float angle;  
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

void onReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(ControlPacket)) return;
  memcpy(&data, incomingData, sizeof(data));
  Serial.print("Cmd: ");
  Serial.print(data.cmd);
  Serial.print("  Angle: ");
  Serial.println(data.angle);
  drive(data.cmd, data.angle);
}

int angleToSpeed(float angle) {
  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
  return map(angle * 10, MIN_ANGLE * 10, MAX_ANGLE * 10, MIN_SPEED, MAX_SPEED);
}

void drive(char cmd, float angle) {
  int speed = angleToSpeed(angle);
  switch (cmd) {
    case 'F': forward(speed); break;
    case 'B': backward(speed); break;
    case 'L': left(speed); break;
    case 'R': right(speed); break;
    default: stopMotors(); break;
  }
}

void forward(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left(int spd) {
  analogWrite(ENA, spd / 3);   //increase spd/3 for faster turn
  analogWrite(ENB, spd);      
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right(int spd) {
  analogWrite(ENA, spd);       
  analogWrite(ENB, spd / 3);   //increase spd/3 for faster turn
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
