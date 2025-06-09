#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define I2C_SDA       21
#define I2C_SCL       22

MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing MPU...");
  display.display();

  mpu.initialize();

  if (!mpu.testConnection()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MPU6050 Error!");
    display.display();
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("MPU6050 Ready :)");
  display.display();
  delay(1000);
}

void loop() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Convert raw to m/s^2
  float ax_ms2 = (ax / 16384.0) * 9.81;
  float ay_ms2 = (ay / 16384.0) * 9.81;
  float az_ms2 = (az / 16384.0) * 9.81;

  // Calculate vibration magnitude in m/s^2
  float vibration = sqrt(ax_ms2 * ax_ms2 + ay_ms2 * ay_ms2 + az_ms2 * az_ms2);

  // Print to Serial
  Serial.printf("AX: %.2f AY: %.2f AZ: %.2f | Vib: %.2f m/s^2\n", ax_ms2, ay_ms2, az_ms2, vibration);

  // Print to OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("AX: "); display.println(ax_ms2, 2);
  display.print("AY: "); display.println(ay_ms2, 2);
  display.print("AZ: "); display.println(az_ms2, 2);
  display.print("Vib: "); display.print(vibration, 2); display.println(" m/s^2");
  display.display();

  delay(500);
}
