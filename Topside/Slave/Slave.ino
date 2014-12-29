#include <Wire.h>

// SCL - analog pin 5
// SDA - analog pin 4
int led = 13;
int debug = 1;
void setup() {
    pinMode(led, OUTPUT);     
    Wire.begin(4);                // join i2c bus with address #4
    Wire.onReceive(receiveEvent); // register event
    Wire.onRequest(requestEvent); // register event
    Serial.begin(9600);           // start serial for input and output
    digitalWrite(led, HIGH);
    Serial.println("I2C SLAVE 5V");
    delay(2000); // the slave should become ready first
    Serial.println("Ready");
}

void loop() {
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
    if (debug) {
        Serial.print("rxEvent "+       Serial.println(howMany));
    }
    while(Wire.available())
    {
        char c = Wire.read();
        if (debug) {
            Serial.print("i2c_rx    "+        Serial.println(c));
        } 
        else {
            Serial.print(c);
        }
    }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
    if (debug) Serial.println("reqEvent");
    if (debug) digitalWrite(led, LOW);
    if (Serial.available()) {
        char inChar = '0'; 
        inChar = (char)Serial.read(); 
        Wire.write(inChar);
        if (debug) {
            Serial.print("sent  "+          Serial.println(inChar));
        }
    } else { //send some dummy data if no data is available from serial
        Wire.write("?");
        if (debug) Serial.println("!sent:");
    }
}
