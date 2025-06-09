#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_DISTANCE_PIN 2
#define LED_MPU_PIN 4
#define LED_VIBRATION_PIN 15

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak
String thingSpeakApiKey = "SGZRZMAO84TK4UTV";
const char* server = "http://api.thingspeak.com/update";

// Sensors
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD address 0x27

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

  Wire.begin(21, 22); // I2C for ESP32

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // WiFi Init
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected   ");

  // MPU6050 Init
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found!");
    lcd.setCursor(0, 1);
    lcd.print("MPU6050 Error    ");
    while (true);
  }

  lcd.setCursor(0, 1);
  lcd.print("MPU6050 Ready    ");
  delay(1000);
  lcd.clear();
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

  // LCD Output
  lcd.setCursor(0, 0);
  lcd.print("Dist:");
  lcd.print(distance, 1);
  lcd.print("cm   ");

  lcd.setCursor(0, 1);
  lcd.print("Vib:");
  lcd.print(vibration, 2);
  lcd.print(" m/s2 ");

  // Debug Serial
  Serial.print("Distance: "); Serial.print(distance);
  Serial.print(" | Vibration: "); Serial.println(vibration);

  // LED Logic
  digitalWrite(LED_DISTANCE_PIN, distance > distanceThreshold ? HIGH : LOW);
  digitalWrite(LED_MPU_PIN, 
    fabs(a.acceleration.x) > accelThreshold ||
    fabs(a.acceleration.y) > accelThreshold ||
    fabs(a.acceleration.z) > accelThreshold ? HIGH : LOW);
  digitalWrite(LED_VIBRATION_PIN, vibration > vibrationThreshold ? HIGH : LOW);

  // Send to ThingSpeak
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
    Serial.print("ThingSpeak Code: ");
    Serial.println(httpResponseCode);
    http.end();
  }

  delay(15000);  // 15 seconds
} 