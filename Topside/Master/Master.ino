#include <Wire.h>
// SCL - analog pin 5
// SDA - analog pin 4

int led = 13;

void setup() {
  pinMode(led, OUTPUT);     
  Wire.begin(); //join I2C as master
  Serial.begin(9600); //setup serial for input and output
  digitalWrite(led, LOW);
  Serial.println("Master Ready..");
  delay(5000); // the master should be ready after the slave
  Serial.println("Ready");
  randomSeed(analogRead(0));  
}

void loop() {
    boolean speed;
    delay(1000);    
    int randNumber = random(300);
    speed = (randNumber > 100);
    String text = (speed) ? "fast" : "slow";
    Serial.println(text);
    blink(speed);
    delay(2000);     
    Wire.beginTransmission(4);
    char charBuf[5];    
    text.toCharArray(charBuf, 5 );
    Wire.write(charBuf);
    Wire.endTransmission();
  }
  
 void blink(boolean fast) { 
  int i;
  int del = (fast) ? 50 : 500;
  int mix = (fast) ? 20 : 5;
  for(i=0;i<mix;i++) { 
    digitalWrite(13, HIGH);
    delay(del);
    digitalWrite(13, LOW);
    delay(del);  
  }
}
