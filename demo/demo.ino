
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
const int sd_cs = 26;
const int miso = 29;
const int mosi = 31;
const int spi_clk = 32;
const int display_cs = 33;
const int display_dc = 37;
const int RED_PIN = 38;
const int GREEN_PIN = 39;
const int BLUE_PIN = 40;
const int led = 41;
const int buzzerPin = 42;
const int tx = 43;
const int rx = 44;
const int sda = 47;
const int display_rst = 48;

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

  int buttonState1 = digitalRead(buttonPin1);
  int buttonState2 = digitalRead(buttonPin2);
  int buttonState3 = digitalRead(buttonPin3);
  int buttonState4 = digitalRead(button4Pin);
  int buttonState5 = digitalRead(button5Pin);
  int buttonState6 = digitalRead(button6Pin);

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
  delay(500);
}
