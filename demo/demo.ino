
// Defining Pins on board
const int x = 2;
const int y = 1;
const int minibadge_clk = 12;
const int right = 13;
const int left = 14;
const int down = 15;
const int up = 16;
const int b = 17;
const int a = 18;
const int scl = 21;
const int RED_PIN = 38;
const int GREEN_PIN = 39;
const int BLUE_PIN = 40;
const int led = 41;
const int buzzerPin = 42;
const int tx = 43;
const int rx = 44;
const int sda = 47;

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_ImageReader.h> // Image-reading functions

#define TFT_CS   33
#define TFT_RST  48 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC   37

#define TFT_MOSI 35  // Data out
#define TFT_SCLK 36  // Clock out

#define SD_CS 26
#define TFT_MISO 34

#define USE_SD_CARD

//SPIClass hspi = SPIClass(HSPI);
//#define SD_CONFIG SdioConfig(TFT_SCLK, TFT_MOSI, TFT_MISO, HSPI)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
//ImageReturnCode      stat;
//SPIClass SDSPI = SPIClass();
float p = 3.1415926;
int i = 0;




// Define the notes and their frequencies (in Hz)
// These are common musical notes, you can add more or change them.
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_REST 0 // Represents a pause

// PWM resolution. ESP32's analogWrite can be set to 10-bit resolution.
// For common cathode, 0 is off, 1023 is full brightness.
const int PWM_RESOLUTION_BITS = 10;
const int PWM_MAX_DUTY = 30; 

// Note: The PWM frequency is set internally by the ESP32's `ledc` peripheral
// when using `analogWrite`. You can configure it more precisely using the
// `ledcSetup`, `ledcAttachPin`, and `ledcWrite` functions if needed,
// but `analogWrite` is simpler for basic use. The default frequency is often around 1kHz.

// --- Helper function to set RGB color ---
void setRgbColor(int r, int g, int b) {
  /*
   * Args:
   * r (int): Red component brightness (0-255).
   * g (int): Green component brightness (0-255).
   * b (int): Blue component brightness (0-255).
   */

  // Map 0-255 brightness to 0-1023 PWM duty cycle.

  int dutyRed = map(r, 0, 255, 0, PWM_MAX_DUTY);
  int dutyGreen = map(g, 0, 255, 0, PWM_MAX_DUTY);
  int dutyBlue = map(b, 0, 255, 0, PWM_MAX_DUTY);

  analogWrite(RED_PIN, dutyRed);
  analogWrite(GREEN_PIN, dutyGreen);
  analogWrite(BLUE_PIN, dutyBlue);
}

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  Serial.println("Starting RGB LED demonstration...");

  pinMode(SD_CS, OUTPUT);
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  //hspi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  tft.init(240, 320);           // Init ST7789 320x240
  tft.setRotation(3);
  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);


  time = millis() - time;
  Serial.println(time, DEC);
  
  if(!SD.begin(SD_CS)) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
  Serial.print(F("Loading BYUILogo.bmp to screen..."));
  reader.drawBMP("/BYUILogo.bmp", tft, 0, 0);

  Serial.println("done");
  delay(1000);

  // Set pin modes for the LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT); 
  
  analogWriteResolution(RED_PIN,PWM_RESOLUTION_BITS);
  analogWriteResolution(GREEN_PIN,PWM_RESOLUTION_BITS);
  analogWriteResolution(BLUE_PIN,PWM_RESOLUTION_BITS);

  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(b, INPUT_PULLUP);
  pinMode(a, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  Serial.println("Joystick Starting...");
  pinMode(x,INPUT);
  pinMode(y,INPUT);
  pinMode(led,OUTPUT);
  
}




void loop() {
  int readingX = 4200 - analogRead(x);
  int readingY = analogRead(y);
  int x = readingX / 25;
  int y = readingY / 25;
  
  setRgbColor(x,0,y);

  int buttonState1 = digitalRead(right);
  int buttonState2 = digitalRead(left);
  int buttonState3 = digitalRead(down);
  int buttonState4 = digitalRead(up);
  int buttonState5 = digitalRead(b);
  int buttonState6 = digitalRead(a);

  // Variable to keep track if any button is currently pressed
  bool anyButtonPressed = false;

  // Check button 1
  if (buttonState1 != LOW) { // Button is pressed 
    Serial.println("Button 1 pressed - Playing C4");
    tone(buzzerPin, NOTE_C4);
    anyButtonPressed = true;
  }
 
  // Check button 2
  if (buttonState2 != LOW) {
    Serial.println("Button 2 pressed - Playing D4");
    tone(buzzerPin, NOTE_D4);
    anyButtonPressed = true;
  }
 
  // Check button 3
  if (buttonState3 != LOW) {
    Serial.println("Button 3 pressed - Playing E4");
    tone(buzzerPin, NOTE_E4);
    anyButtonPressed = true;
  }
  
  // Check button 4
  if (buttonState4 != LOW) {
    Serial.println("Button 4 pressed - Playing F4");
    tone(buzzerPin, NOTE_F4);
    anyButtonPressed = true;
  }
 
  // Check button 5
  if (buttonState5 != LOW) {
    Serial.println("Button 5 pressed - Playing G4");
    tone(buzzerPin, NOTE_G4);
    anyButtonPressed = true;
    
  }

  // Check button 6
  if (buttonState6 != LOW) {
    
    Serial.println("Button 6 pressed - Playing A4");
    tone(buzzerPin, NOTE_A4);
    anyButtonPressed = true;
  }

  // If no button is currently pressed, stop the tone
  if (!anyButtonPressed) {
    noTone(buzzerPin);
  }

  // A small delay to allow the ESP32 to perform other tasks and for stable readings
  delay(250);
}
