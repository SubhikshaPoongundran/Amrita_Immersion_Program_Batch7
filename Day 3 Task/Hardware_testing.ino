#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <math.h>

// OLED display config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
#define IR_SENSOR_PIN 5
#define LED_IR_PIN 2
#define LED_MPU_PIN 4
#define LED_VIBRATION_PIN 15

// WiFi credentials
const char* ssid = "Vivo_T1";
const char* password = "1234567890";

// ThingSpeak
String thingSpeakApiKey = "SGZRZMAO84TK4UTV";
const char* server = "http://api.thingspeak.com/update";

// Sensor
Adafruit_MPU6050 mpu;

// Constants
const float gravity = 9.8;
const float accelThreshold = 3.0;
const float vibrationThreshold = 1.5;

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(LED_IR_PIN, OUTPUT);
  pinMode(LED_MPU_PIN, OUTPUT);
  pinMode(LED_VIBRATION_PIN, OUTPUT);

  Wire.begin(21, 22); // I2C pins for ESP32

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.display();

  // MPU6050 init
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found!");
    display.setCursor(0, 10);
    display.println("MPU6050 Error!");
    display.display();
    while (true);
  }

  display.setCursor(0, 10);
  display.println("MPU6050 Ready");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMagnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  float vibration = fabs(accelMagnitude - gravity);

  // IR sensor
  bool objectDetected = digitalRead(IR_SENSOR_PIN) == LOW;

  // OLED Output
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("IR: ");
  display.println(objectDetected ? "Detected" : "None");

  display.setCursor(0, 10);
  display.print("Accel Mag: ");
  display.println(accelMagnitude, 2);

  display.setCursor(0, 20);
  display.print("Vibration: ");
  display.println(vibration, 2);

  display.setCursor(0, 30);
  display.print("Ax: ");
  display.print(a.acceleration.x, 2);
  display.print(" Ay: ");
  display.println(a.acceleration.y, 2);

  display.setCursor(0, 40);
  display.print("Az: ");
  display.println(a.acceleration.z, 2);

  display.display();

  // Debug Serial
  Serial.print("IR: ");
  Serial.print(objectDetected);
  Serial.print(" | Vibration: ");
  Serial.println(vibration, 2);

  // LEDs
  digitalWrite(LED_IR_PIN, objectDetected ? HIGH : LOW);
  digitalWrite(LED_MPU_PIN,
    fabs(a.acceleration.x) > accelThreshold ||
    fabs(a.acceleration.y) > accelThreshold ||
    fabs(a.acceleration.z) > accelThreshold ? HIGH : LOW
  );
  digitalWrite(LED_VIBRATION_PIN, vibration > vibrationThreshold ? HIGH : LOW);

  // Send to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + thingSpeakApiKey;
    url += "&field1=" + String(objectDetected);
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

  delay(5000);  // ThingSpeak limit
}
