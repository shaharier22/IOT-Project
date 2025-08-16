#define BLYNK_TEMPLATE_ID "TMPL6k5z5Bj4r"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "LR9rUbC39AEfmJllVPDpH3GfRRN1M5RU"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
char ssid[] = "NO NAME";
char pass[] = "sakib0197";

// IR sensors
const int irPin1 = 14; // Entrance
const int irPin2 = 27; // Exit

int in_count = 0;
int out_count = 0;
int current_count = 0;
bool crashed = false; 

const unsigned long timeout = 1000; // ms
const int buttonPin = 15; // Crash alert

// Blynk virtual pins
#define VPIN_IN      V4
#define VPIN_OUT     V1
#define VPIN_CURRENT V2
#define VPIN_CRASH   V6

// LCD setup (I2C address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Passenger capacity
const int max_capacity = 32;

void setup() {
  Serial.begin(115200);

  pinMode(irPin1, INPUT_PULLUP);
  pinMode(irPin2, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // LCD init
  Wire.begin(21, 22); // SDA = 21, SCL = 22
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bus Status:");
  lcd.setCursor(0, 1);
  lcd.print("Empty");

  Serial.println("Human Counter Initialized");
  Serial.println("--------------------------------");
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Passenger: ");
  lcd.print(current_count);

  lcd.setCursor(0, 1);
  if (current_count == 0) {
    lcd.print("Capacity: Empty");
  } else if (current_count >= max_capacity) {
    lcd.print("Capacity: Full");
  } else if (current_count >= (max_capacity / 2)) {
    lcd.print("Capacity: Half");
  } else {
    lcd.print("Capacity: Low");
  }
}

void loop() {
  Blynk.run();

  // Crash button handling
  if (digitalRead(buttonPin) == LOW) {
    if (!crashed) {
      crashed = true;
      Serial.println("⚠️ Vehicle Crashed!");
      Blynk.virtualWrite(VPIN_CRASH, "Vehicle Crashed!");
      delay(5000);
      Serial.println("⚠️ Vehicle is safe");
      Blynk.virtualWrite(VPIN_CRASH, "Vehicle is safe");
    }
  } else {
    crashed = false;
  }

  // ENTRY detection
  if (digitalRead(irPin1) == LOW) {
    unsigned long startTime = millis();
    while ((millis() - startTime) < timeout) {
      if (digitalRead(irPin2) == LOW) {
        in_count++;
        current_count = in_count - out_count;
        Serial.println("🚶‍♂️ Person Entered");
        Serial.print("🔢 Total In: "); Serial.println(in_count);
        Serial.print("🧮 Currently Inside: "); Serial.println(current_count);
        Serial.println("--------------------------------");
        Blynk.virtualWrite(VPIN_IN, in_count);
        Blynk.virtualWrite(VPIN_CURRENT, current_count);
        updateLCD();
        delay(1000);
        break;
      }
    }
    while (!digitalRead(irPin1) || !digitalRead(irPin2));
  }

  // EXIT detection
  else if (digitalRead(irPin2) == LOW) {
    unsigned long startTime = millis();
    while ((millis() - startTime) < timeout) {
      if (digitalRead(irPin1) == LOW) {
        if (out_count < in_count) {
          out_count++;
          current_count = in_count - out_count;
          Serial.println("🏃‍♂️ Person Exited");
          Serial.print("🔢 Total Out: "); Serial.println(out_count);
          Serial.print("🧮 Currently Inside: "); Serial.println(current_count);
          Serial.println("--------------------------------");
          Blynk.virtualWrite(VPIN_OUT, out_count);
          Blynk.virtualWrite(VPIN_CURRENT, current_count);
          updateLCD();
          delay(1000);
          break;
        }
      }
    }
    while (!digitalRead(irPin1) || !digitalRead(irPin2));
  }
}
