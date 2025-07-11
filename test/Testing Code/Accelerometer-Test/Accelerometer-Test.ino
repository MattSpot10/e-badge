#include <Wire.h>
#include <MMA8453_n0m1.h>

#define sda_pin 47
#define scl_pin 21

MMA8453_n0m1 accel;

float p = 3.1415926;

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! Accelerometer Test"));

  accel.setI2CAddr(0x1C);
  Wire.setPins(sda_pin, scl_pin);
  accel.dataMode(true, 2);
  Serial.println(F("Initialized"));
  delay(500);
}

void loop() {
  delay(1000);
  accel.update();
  Serial.print("x: ");
  Serial.print(accel.x());
  Serial.print(" y: ");
  Serial.print(accel.y());
  Serial.print(" z: ");
  Serial.println(accel.z());
}

