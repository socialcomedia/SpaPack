#include <Wire.h>
// SCL - analog pin 5
// SDA - analog pin 4

int led = 13;
int debug = 1;

void setup() {
  pinMode(led, OUTPUT);     
  Wire.begin(); //join I2C as master
  Serial.begin(9600); //setup serial for input and output
  digitalWrite(led, HIGH);
  Serial.println("I2C MASTER 3.3V");
  delay(5000); // the master should be ready after the slave
  Serial.println("Ready");
}

void loop() {
  if (Serial.available()) {
    char inChar = (char)Serial.read(); 
    if (debug) {
      Serial.print("serial"+      Serial.println(inChar));
    }
    Wire.beginTransmission(4);
    Wire.write(inChar);
    Wire.endTransmission();
  } 
  else {
    if (debug) Serial.println("!serial");
  }

  Wire.requestFrom(4, 1); // request 1 byte from slave device #4
  if (debug) Serial.println("waitOnReq");
  while(!Wire.available());
  char d = Wire.read();
  if (debug) {
    Serial.print(" reqData:");
    Serial.println(d);
    delay(50);
  } 
  else { // ignore special character ?
    if (d != '?') Serial.print(d);
  }

  if (debug) digitalWrite(led, HIGH);
  if (debug) delay(200);
}
