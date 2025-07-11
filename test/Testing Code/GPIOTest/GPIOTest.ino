#define indx 1
void setup() {
  // put your setup code here, to run once:

  for(int i = 0; i < 4; i++)
  {
    pinMode(i+indx, OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i = 0; i < 4; i++)
  {
    digitalWrite(indx+i, 1);
    delay(250);
  }
  for(int i = 0; i < 4; i++)
  {
    digitalWrite(indx+i, 0);
    delay(250);
  }
   for(int i = 0; i < 4; i++)
  {
    digitalWrite(indx+i, 1);
    
  }
  delay(250);
   for(int i = 0; i < 4; i++)
  {
    digitalWrite(indx+i, 0);
    
  }
  delay(250);
}
