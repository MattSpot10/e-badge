#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h> 
#include <WiFi.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
// Display SPI Pins
#define TFT_CS   9
#define TFT_RST  48
#define TFT_DC   13
#define TFT_MOSI 11
#define TFT_SCLK 12

// Button Pins 
const int PIN_A = 38;
const int PIN_B = 18;
const int PIN_UP = 17;
const int PIN_DOWN = 16;
const int PIN_LEFT = 14;
const int PIN_RIGHT = 15;

// Analog Inputs
const int PIN_JOYSTICK_X = 1; 
const int PIN_JOYSTICK_Y = 2;
const int BUTTON_THRESHOLD = 4095 / 3; 

const int PIN_ADDR_LEDS = 7; 
const int LED_COUNT = 23; 
const int PIN_BUZZER = 42;
const int PIN_RED = 6;
const int PIN_GREEN = 5;
const int PIN_BLUE = 4;

// ============================================================================
// GLOBALS & STATE
// ============================================================================
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

Adafruit_NeoPixel pixels(LED_COUNT, PIN_ADDR_LEDS, NEO_GRB + NEO_KHZ800);

// Test State Enum
enum TestState {
  TEST_WELCOME,
  TEST_DISPLAY,
  TEST_BUTTONS,
  TEST_JOYSTICK,
  TEST_RGB_LED,
  TEST_ADDR_LEDS,
  TEST_BUZZER,
  TEST_WIFI,
  TEST_COMPLETE
};

TestState current_state = TEST_WELCOME;
unsigned long last_button_press = 0;

const unsigned long DEBOUNCE_DELAY = 250; 


bool isButtonPressed(int pin) {

  if (pin == PIN_A || pin == PIN_B) {
    return digitalRead(pin) == HIGH;
  }

  return analogRead(pin) >= BUTTON_THRESHOLD;
}

// ============================================================================
// NEXT STATE LOGIC & SCREEN UTILS
// ============================================================================

void nextState() {
  if (current_state == TEST_RGB_LED) {
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
  }
  if (current_state == TEST_ADDR_LEDS) {
    pixels.clear();
    pixels.show();
  }
  
  if (current_state < TEST_COMPLETE) {
    current_state = (TestState)(current_state + 1);
  }
}

void clearScreen(const char* title) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.println(title);
  tft.drawFastHLine(0, 30, 320, ST77XX_WHITE);
  tft.setCursor(10, 40);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
}

// ============================================================================
// TEST FUNCTIONS (INITIALIZATION & STATIC DRAW)
// ============================================================================

void runWelcomeTest() {
  clearScreen("WELCOME TO E-BADGE TEST");
  tft.setTextSize(2);
  tft.setCursor(30, 80);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("Press 'A' to Start");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 150);
  tft.println("Use 'A' to advance to the next test.");
}

void runDisplayTest() {
  clearScreen("TEST 1: DISPLAY");
  tft.setTextColor(ST77XX_GREEN);
  tft.println("If you can read this, the display works!");
  tft.setTextColor(ST77XX_RED);
  tft.println("");
  tft.println("Display Color Test:");
  tft.fillRect(10, 100, 50, 50, ST77XX_RED);
  tft.fillRect(70, 100, 50, 50, ST77XX_GREEN);
  tft.fillRect(130, 100, 50, 50, ST77XX_BLUE);
  tft.fillRect(190, 100, 50, 50, ST77XX_YELLOW);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 170);
  tft.println("Press 'A' to continue.");
}

void initButtonsTest() {
  // Only draw the static elements once
  clearScreen("TEST 2: BUTTONS"); // Title corrected
  tft.println("Press each directional and A/B button.");
  tft.println("Button status will appear below:");

  tft.setCursor(10, 80); tft.print("UP: ");
  tft.setCursor(10, 95); tft.print("DOWN: ");
  tft.setCursor(10, 110); tft.print("LEFT: ");
  tft.setCursor(10, 125); tft.print("RIGHT: ");
  tft.setCursor(10, 140); tft.print("A: ");
  tft.setCursor(10, 155); tft.print("B: ");
  
  tft.setCursor(10, 180);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to confirm and continue.");
}

void initJoystickTest() {
  // Only draw the static elements once
  clearScreen("TEST 3: JOYSTICK"); // Title corrected
  tft.println("Move the joystick and check the values.");

  tft.setCursor(10, 60);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("X-Axis: ");

  tft.setCursor(10, 80);
  tft.print("Y-Axis: ");

  tft.setCursor(10, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to confirm and continue.");
}

void initRgbLedTest() {
  clearScreen("TEST 4: RGB LED");
  tft.println("The onboard RGB LED should cycle more colors.");
  tft.println("Color Check: Red, Yellow, Green, Cyan, Blue, Magenta.");

  tft.setCursor(10, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to stop the cycle and continue.");
}

void initAddrLedsTest() {
  clearScreen("TEST 5: ADDRESSABLE LEDs");
  tft.println("All 23 Addressable LEDs should chase colors.");
  
  pixels.clear(); 
  pixels.show(); 
  
  tft.setCursor(10, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to stop and continue.");
}

// ============================================================================
// TEST FUNCTIONS (CONTINUOUS UPDATE)
// ============================================================================

void updateButtonsTest() {
  // Check and display button status (only updating the changing text)
  auto displayButtonStatus = [&](int pin, int y, const char* name) {
    int x_pos = 60;
    tft.setCursor(x_pos, y);
    tft.fillRect(x_pos, y, 100, 10, ST77XX_BLACK); 

    bool pressed = isButtonPressed(pin);
    tft.setTextColor(pressed ? ST77XX_GREEN : ST77XX_RED);
    tft.print(pressed ? "PRESSED" : "RELEASED");
  };

  displayButtonStatus(PIN_UP, 80, "UP");
  displayButtonStatus(PIN_DOWN, 95, "DOWN");
  displayButtonStatus(PIN_LEFT, 110, "LEFT");
  displayButtonStatus(PIN_RIGHT, 125, "RIGHT");
  displayButtonStatus(PIN_A, 140, "A");
  displayButtonStatus(PIN_B, 155, "B");
}

void updateJoystickTest() {
  int x_val = analogRead(PIN_JOYSTICK_X);
  int y_val = analogRead(PIN_JOYSTICK_Y);

  // Update X value
  int x_pos = 140;
  tft.setCursor(x_pos, 60);
  tft.fillRect(x_pos, 60, 150, 10, ST77XX_BLACK); // Clear value area
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(x_val);

  // Update Y value
  tft.setCursor(x_pos, 80);
  tft.fillRect(x_pos, 80, 150, 10, ST77XX_BLACK); // Clear value area
  tft.print(y_val);
}


void updateRgbLedTest() {
  unsigned long now = millis();
  int cycle = (now / 1000) % 6; // Cycle every 1 second through 6 colors

  uint16_t color_tft = ST77XX_BLACK;
  const char* color_name = "";

  if (cycle == 0) { // Red
    analogWrite(PIN_RED, 255); analogWrite(PIN_GREEN, 0); analogWrite(PIN_BLUE, 0);
    color_tft = ST77XX_RED; color_name = "-> RED <-";
  } else if (cycle == 1) { // Yellow (Red + Green)
    analogWrite(PIN_RED, 255); analogWrite(PIN_GREEN, 255); analogWrite(PIN_BLUE, 0);
    color_tft = ST77XX_YELLOW; color_name = "-> YELLOW <-";
  } else if (cycle == 2) { // Green
    analogWrite(PIN_RED, 0); analogWrite(PIN_GREEN, 255); analogWrite(PIN_BLUE, 0);
    color_tft = ST77XX_GREEN; color_name = "-> GREEN <-";
  } else if (cycle == 3) { // Cyan (Green + Blue)
    analogWrite(PIN_RED, 0); analogWrite(PIN_GREEN, 255); analogWrite(PIN_BLUE, 255);
    color_tft = ST77XX_CYAN; color_name = "-> CYAN <-";
  } else if (cycle == 4) { // Blue
    analogWrite(PIN_RED, 0); analogWrite(PIN_GREEN, 0); analogWrite(PIN_BLUE, 255);
    color_tft = ST77XX_BLUE; color_name = "-> BLUE <-";
  } else { // Magenta (Red + Blue)
    analogWrite(PIN_RED, 255); analogWrite(PIN_GREEN, 0); analogWrite(PIN_BLUE, 255);
    color_tft = ST77XX_MAGENTA; color_name = "-> MAGENTA <-";
  }

  // Update display status
  int x_pos = 10;
  int y_pos = 80;
  tft.setCursor(x_pos, y_pos);
  tft.fillRect(x_pos, y_pos, 200, 10, ST77XX_BLACK); 
  tft.setTextColor(color_tft); 
  tft.println(color_name);
}

void updateAddrLedsTest() {
  unsigned long now = millis();
  int led_index = (now / 100) % LED_COUNT; // Chase speed: 100ms per LED
  int color_cycle = (now / 3000) % 3; // Change base color every 3 seconds

  uint32_t color = pixels.Color(0, 0, 0);
  if (color_cycle == 0) color = pixels.Color(255, 0, 0); // Red
  else if (color_cycle == 1) color = pixels.Color(0, 255, 0); // Green
  else color = pixels.Color(0, 0, 255); // Blue

  pixels.clear();

  pixels.setPixelColor(led_index, color); 
  pixels.show();

  int x_pos = 10;
  int y_pos = 100;
  tft.setCursor(x_pos, y_pos);
  tft.fillRect(x_pos + 130, y_pos, 40, 10, ST77XX_BLACK);
  tft.print(led_index);
}


void runBuzzerTest() {
  clearScreen("TEST 6: BUZZER");
  tft.println("The Buzzer should make a tone.");

  tone(PIN_BUZZER, 500, 200); // 500 Hz for 200ms
  delay(200);
  tone(PIN_BUZZER, 700, 200); // 700 Hz for 200ms
  delay(200);
  tone(PIN_BUZZER, 900, 200); // 900 Hz for 200ms
  delay(200);
  noTone(PIN_BUZZER);

  tft.setCursor(10, 80);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("Buzzer Tone Played.");

  tft.setCursor(10, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to confirm and continue.");
}

void runWifiTest() {
  clearScreen("TEST 7: WIFI SCAN");
  tft.println("Testing ESP32's Wi-Fi module...");

  // --- WiFi Test ---
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("\n-> WiFi Scan:");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    tft.setTextColor(ST77XX_RED);
    tft.println("No WiFi networks found.");
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.print(n);
    tft.println(" networks found. WiFi OK!");
  }
  WiFi.scanDelete();
  
  tft.setCursor(10, 160);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press 'A' to confirm and complete.");
}

void runCompleteTest() {
  clearScreen("TEST COMPLETE!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(30, 80);
  tft.println("ALL TESTS PASSED!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 120);
  tft.println("Your e-Badge hardware is functional.");
  tft.setCursor(10, 140);
  tft.println("Restart the badge to run the test again.");
  
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
  pixels.clear();
  pixels.show();
  noTone(PIN_BUZZER);
}

// ============================================================================
// ARDUINO SETUP & LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI); 
  tft.init(240, 320);
  tft.setRotation(1); 
  tft.fillScreen(ST77XX_BLACK);

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  
  pixels.begin();
  pixels.clear();
  pixels.show();

  pinMode(PIN_A, INPUT); 
  pinMode(PIN_B, INPUT); 
  
  pinMode(PIN_BUZZER, OUTPUT);

  runWelcomeTest();
}

void loop() {
  unsigned long currentTime = millis();

  if (digitalRead(PIN_A) == HIGH && (currentTime - last_button_press > DEBOUNCE_DELAY)) {
    nextState();
    last_button_press = currentTime;
    delay(10);
    
    switch (current_state) {
      case TEST_DISPLAY: runDisplayTest(); break;
      case TEST_BUTTONS: initButtonsTest(); break; 
      case TEST_JOYSTICK: initJoystickTest(); break; 
      case TEST_RGB_LED: initRgbLedTest(); break; 
      case TEST_ADDR_LEDS: initAddrLedsTest(); break;
      case TEST_BUZZER: runBuzzerTest(); break;
      case TEST_WIFI: runWifiTest(); break;
      case TEST_COMPLETE: runCompleteTest(); break;
      default: break; 
    }
  }

  // --- Continuous / Looping Tests (updates only the changing data) ---
  
  if (current_state == TEST_BUTTONS) {
    updateButtonsTest();
  }
  
  if (current_state == TEST_JOYSTICK) {
    updateJoystickTest();
  }

  if (current_state == TEST_RGB_LED) {
    updateRgbLedTest();
  }

  if (current_state == TEST_ADDR_LEDS) {
    updateAddrLedsTest();
  }
  
  // --- Speed Control ---
  // Apply a long delay only when waiting on a static test screen for user input
  if (current_state == TEST_WELCOME || current_state == TEST_DISPLAY || 
      current_state == TEST_BUZZER || current_state == TEST_WIFI ||
      current_state == TEST_COMPLETE) {
    delay(500);
  } else {
    delay(10);
  }
}