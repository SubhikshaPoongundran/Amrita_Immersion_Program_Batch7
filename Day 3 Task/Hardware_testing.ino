#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED I2C address is usually 0x3C
#define OLED_ADDR 0x3C

// Define I2C pins for ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // Start serial for debug
  Serial.begin(115200);

  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  
  // Display messages
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("I2C OLED Display");
  display.setCursor(0, 30);
  display.println("SCK = SCL");
  display.setCursor(0, 45);
  display.println("ESP32 + OLED :)");
  
  display.display();
}

void loop() {
  // Nothing to loop
}
