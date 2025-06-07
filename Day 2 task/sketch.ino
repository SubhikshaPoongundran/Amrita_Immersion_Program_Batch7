#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_DISTANCE_PIN 2  // LED based on ultrasonic sensor
#define LED_MPU_PIN 4       // LED based on MPU6050 motion
#define LED_VIBRATION_PIN 15 // New LED for vibration indicator

// WiFi Credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak
String thingSpeakApiKey = "SGZRZMAO84TK4UTV";  // Replace with your Write API Key
const char* server = "http://api.thingspeak.com/update";

// MPU6050
Adafruit_MPU6050 mpu;

// Gravity constant approx
const float gravity = 9.8;

// Vibration threshold (adjust experimentally)
const float vibrationThreshold = 1.5;  // m/s² deviation from gravity

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_DISTANCE_PIN, OUTPUT);
  pinMode(LED_MPU_PIN, OUTPUT);
  pinMode(LED_VIBRATION_PIN, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not connected!");
    while (1);
  }
}

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  // Read sensor values
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float distance = readDistanceCM();

  // Calculate acceleration magnitude
  float accelMagnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z);

  // Calculate vibration value as deviation from gravity
  float vibration = abs(accelMagnitude - gravity);

  // Print to Serial
  Serial.print("Distance: "); Serial.print(distance); Serial.print(" cm");
  Serial.print(" | Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(" | Y: "); Serial.print(a.acceleration.y);
  Serial.print(" | Z: "); Serial.print(a.acceleration.z);
  Serial.print(" | Magnitude: "); Serial.print(accelMagnitude);
  Serial.print(" | Vibration: "); Serial.println(vibration);

  // Thresholds
  const float distanceThreshold = 150.0;    // cm
  const float accelThreshold = 3.0;         // m/s² for motion
  const float vibrationThreshold = 1.5;     // m/s² for vibration deviation

  // LED logic based on distance
  if (distance > distanceThreshold) {
    digitalWrite(LED_DISTANCE_PIN, HIGH);  // Turn ON if object is far
  } else {
    digitalWrite(LED_DISTANCE_PIN, LOW);   // Turn OFF if object is near
  }

  // LED logic based on MPU6050 motion (acceleration on any axis)
  if (abs(a.acceleration.x) > accelThreshold ||
      abs(a.acceleration.y) > accelThreshold ||
      abs(a.acceleration.z) > accelThreshold) {
    digitalWrite(LED_MPU_PIN, HIGH);  // Turn ON if motion detected
  } else {
    digitalWrite(LED_MPU_PIN, LOW);   // Turn OFF if no significant motion
  }

  // LED logic based on vibration threshold
  if (vibration > vibrationThreshold) {
    digitalWrite(LED_VIBRATION_PIN, HIGH);  // Turn ON if vibration detected
  } else {
    digitalWrite(LED_VIBRATION_PIN, LOW);   // Turn OFF if no significant vibration
  }

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + thingSpeakApiKey;
    url += "&field1=" + String(distance);
    url += "&field2=" + String(a.acceleration.x);
    url += "&field3=" + String(a.acceleration.y);
    url += "&field4=" + String(a.acceleration.z);
    url += "&field5=" + String(vibration);

    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("Data sent. Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data. Code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(15000);  // ThingSpeak allows one update every 15 seconds
}