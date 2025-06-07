#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_DISTANCE_PIN 2
#define LED_MPU_PIN 4
#define LED_VIBRATION_PIN 15

// Sensors
Adafruit_MPU6050 mpu;

// Constants
const float gravity = 9.8;
const float distanceThreshold = 150.0;
const float accelThreshold = 3.0;
const float vibrationThreshold = 1.5;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_DISTANCE_PIN, OUTPUT);
  pinMode(LED_MPU_PIN, OUTPUT);
  pinMode(LED_VIBRATION_PIN, OUTPUT);

  Wire.begin(21, 22);  // ESP32 I2C pins

  // Initialize MPU6050
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found!");
    while (true);
  }

  Serial.println("MPU6050 Ready");
}

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return duration * 0.034 / 2;
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float distance = readDistanceCM();

  float accelMagnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z);

  float vibration = fabs(accelMagnitude - gravity);

  // Serial output
  Serial.print("Distance: "); Serial.print(distance);
  Serial.print(" cm | Vibration: "); Serial.print(vibration);
  Serial.print(" m/s^2 | Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(" Y: "); Serial.print(a.acceleration.y);
  Serial.print(" Z: "); Serial.println(a.acceleration.z);

  // LED Logic
  digitalWrite(LED_DISTANCE_PIN, distance > distanceThreshold ? HIGH : LOW);
  digitalWrite(LED_MPU_PIN, 
    fabs(a.acceleration.x) > accelThreshold ||
    fabs(a.acceleration.y) > accelThreshold ||
    fabs(a.acceleration.z) > accelThreshold ? HIGH : LOW);
  digitalWrite(LED_VIBRATION_PIN, vibration > vibrationThreshold ? HIGH : LOW);

  delay(15000);  // 15 seconds delay
}
