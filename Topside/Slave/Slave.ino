#include <Wire.h>

// SCL - analog pin 5
// SDA - analog pin 4
int led = 13;


void setup() {
    Serial.begin(9600);           // start serial for input and output
    Serial.println("Slave Ready..");
    pinMode(led, OUTPUT);     
    Wire.begin(4);                // join i2c bus with address #4
    Wire.onReceive(receiveEvent); // register event
    digitalWrite(led, LOW);
}

void loop() {
  delay(100);
}

void blink(boolean fast) { 
  if (fast) Serial.println("Blinking Fast"); else Serial.println("Blinking Slow");
  int i;
  int del = (fast) ? 50 : 500;
  int mix = (fast) ? 20 : 5;
  for(i=0;i<mix;i++) { 
    digitalWrite(led, HIGH);
    delay(del);
    digitalWrite(led, LOW);
    delay(del);  
  }
}

void receiveEvent(int howMany) {
    String fullText = "";
    while(Wire.available()) {
        char c = Wire.read();
        fullText = fullText + c;
    }
    Serial.println("Received : " + fullText);
    if (fullText == "fast") blink(true);
    if (fullText == "slow") blink(false);
}

