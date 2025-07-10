const int x = 1;
const int y = 2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Joystick Starting...");
  pinMode(x,INPUT);
  pinMode(y,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int readingX = analogRead(x);
  int readingY = analogRead(y);
  Serial.println("X: ");
  Serial.println(readingX);
  Serial.println("Y: ");
  Serial.println(readingY);
  delay(2000);
}
