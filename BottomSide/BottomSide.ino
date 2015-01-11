//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  SpaPack v1.0
//|
//|  copyright MIT License 2014 :: Ralph C. Ferrara Jr.
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  #include <Time.h> 
  #include <math.h>
  #include <SPI.h> 
  #include <Wire.h>   

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Version
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const float thisVer      = 1.17;
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Sensor Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const int sensorPressure  = 2;        // Pressure Sensor
  const int sensorTemp	    = A1;       // Temperature Sensor
  const int sensorHiTemp    = A2;       // High Limit Sensor
  const int inputButton     = A3;       // Analog Input for Button
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Relays
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=0-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  const int relayLow        = 2;        // Digital Pinout for Motor Low  (110V 10AMP SSR)
  const int relayHigh       = 3;        // Digital Pinout for Motor High (110V 10AMP SSR)
  const int relayBlower     = 4;        // Digital Pinout for Blower
  const int relayAux        = 5;        // Digital Pinout for Aux/Ozone  (110V 10AMP SSR)
  const int relayHeater     = 6;        // Digital Pinout for Heater     (220V 30AMP SSR)
  const int relayLight      = 7;        // Digital Pinout for Lighting 
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Button Resistances
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const int btnLightLow     = 585;      // Analog Low for Light Button
  const int btnLightHigh    = 625;      // Analog High for Light Button

  const int btnModeLow      = 900;      // Analog Low for Mode Button
  const int btnModeHigh     = 950;      // Analog High for Mode Button

  const int btnUpLow        = 275;      // Analog Low for Temp Up Button
  const int btnUpHigh       = 325;      // Analog High for Temp Up Button

  const int btnDownLow      = 750;      // Analog Low for Temp Down Button
  const int btnDownHigh     = 800;      // Analog High for Temp Down Button
  
  int lastButton            = 0;
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  High Lows
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int tempFail              = 108;      // Maximum Temperature before failing
  int tempMax               = 105;      // Maximum Configurable Temperature Setting
  int tempMin               = 45;       // Hard Minimum Temperature Setting  
  int tempDiff              = 3;        // Maximum Temperature Differential (T1/T2)
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Analog Readings
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int temperature1          = -1;      // Current Temperature Probe #1
  int temperature2          = -1;      // Current Temperature Probe #1  
  int tempCount             = 1000;   // How many cycles since we sampled temperature

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Current Statuses
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  int statusLight           = 0;        // 0 = Off / 1 = On
  int statusMotor           = 0;        // 0 = Off / 1 = Low / 2 = High
  int statusHeater          = 0;        // 0 = Off / 1 = On
  int statusBlower          = 0;        // 0 = Off / 1 = On
  int statusAux             = 0;        // 0 = Off / 1 = On    
  int statusError           = 0;

  int currentMode           = 0;        // Mode  (0 = Off, 1 = Low, 2 = High, 3 = High, 4 = Low/Light 5 = High/Light 6 = High/Blower/Light)
  int tempSet               = 72;      // Temperature Currently Set To    
  int currentMin            = 0;        // How we determine the current minute        

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Handle our Cycles
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  char cycleType            = 'X';      // Cycle (X = Off, C = Clean, R = Run, H = Hold)
  int cycleRemaining        = 0;        // Cycle Remaining Type
    
  int runCycleLength        = 20;       // How long should we run for when we're in the spa 
  int cleanCycleLength      = 60;       // Clean Cycle Minutes 
  int cleanHourStart        = 23;       // What time should we run the clean cycle
  int holdCycleLength       = 240;      // How many minutes should we hold the Temperature
  int holdCycleDay[]        = {7,8};    // Which days should we have a hold cycle on? ( 1 = Sunday / 8 = Saturday ) 
  int holdCycleStart        = 18;       // What hour should our Hold Period Start
  
  int errorCycle            = 5;        // Turn off after 5 minutes with an error
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Heater Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  int heaterTimeout         = 5;        // How long to wait before re-igniting the heater MUST BE POSITIVE NUMBER
  int heaterRemaining       = 0;        // Keep track of how long it's been 
  int heaterMaximum         = 240;      // Maximum Length the heater should EVER run

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Light Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  int lightRemaining        = 0;        // Keep track of how long it's been 
  int lightMaximum          = 240;      // Maximum Length the light should EVER run

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Test Mode
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean testMode          = false;
  String  testText          = "";

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Setup Loop
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void setup() {
    Serial.begin(9600);
    Serial.println("Setting up the system..");
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setting the Default Date/Time
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    setTime(12,0,0,1,1,11);    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setup Inputs
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    pinMode(sensorPressure,    INPUT);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setup Inputs
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    pinMode(relayLight,        OUTPUT);
    pinMode(relayHigh,         OUTPUT);
    pinMode(relayLow,          OUTPUT);  
    pinMode(relayHeater,       OUTPUT);
    pinMode(relayBlower,       OUTPUT);    
    pinMode(relayAux,          OUTPUT);        
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setup Buttons
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    pinMode(inputButton,       INPUT);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Disable Everything at First
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    digitalWrite(relayLow,     LOW);
    digitalWrite(relayHigh,    LOW);   
    digitalWrite(relayBlower,  LOW);
    digitalWrite(relayHeater,  LOW);
    digitalWrite(relayAux,     LOW);
    digitalWrite(relayLight,   LOW);      
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Shorten Times for Test Mode
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (testMode) { 
      runCycleLength        = 5;
      cleanCycleLength      = 10;
      holdCycleLength       = 10;
      heaterMaximum         = 6;
      lightMaximum          = 10;
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Shorten Times for Test Mode
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    Wire.begin(); //join I2C as master    
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Main Loop
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void loop() {
    byte incomingByte;
    if (Serial.available() > 0) { 
      incomingByte = Serial.read();
      handleSerial(incomingByte);
    }
    checkTemp();
    cycleTime();
    processAll();   
    buttonPress(-1, true);
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Data Retrieval
//| Buttons, Sensors Etc
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Check Pressure :: Ensure we're within our limits
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean checkPressure() {
    return (digitalRead(sensorPressure) == HIGH);
  } 

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  int getTemp() {
     return int((temperature1 + temperature2) / 2);
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean checkTemp() { 
    if (tempCount > 1000) { 
      temperature1 = convertTemperature(analogRead(sensorTemp));
      temperature2 = convertTemperature(analogRead(sensorHiTemp));  
      tempCount    = 0;
    }
    tempCount++;
}
 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Button Interrupt
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void buttonPress(int testButton, boolean debounce) { 
    return;
    int button     = 0;
    int buttonRead = analogRead(inputButton);
    if (testButton > -1) buttonRead = testButton; // Handle Test Mode Serial
    if (buttonRead >= btnLightLow && buttonRead <= btnLightHigh) button = 1;
    if (buttonRead >= btnModeLow && buttonRead <= btnModeHigh)   button = 2;
    if (buttonRead >= btnUpLow && buttonRead <= btnUpHigh)       button = 3;
    if (buttonRead >= btnDownLow && buttonRead <= btnDownHigh)   button = 4;    
    if (debounce == true || button != lastButton) lastButton = button; else return;
    switch(button) { 
       case 1 : 
         statusLight = (statusLight == 0) ? 1 : 0; 
         if (statusLight == 0)   lightRemaining  = -2;
         if (statusLight == 1)   lightRemaining  = lightMaximum;         
         testInteger("Light Mode", statusLight);
         break;
       case 2 : 
         currentMode = (currentMode == 3) ? 0 : currentMode + 1; 
         if (currentMode >  0) startCycle('R');
         if (currentMode == 0) { 
           cycleRemaining        = -1;
           cycleType             = 'X';           
         }
         break;
       case 3 : 
         tempSet = (tempSet == tempMax) ? tempMax : tempSet + 1; 
         testInteger("Temperature Up", tempSet);
         break;
       case 4 : 
         tempSet = (tempSet == tempMin) ? tempMin : tempSet - 1; 
         testInteger("Temperature Down", tempSet);         
         break;
    }
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Data Processing
//| 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||      

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Begin a Cycle
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean startCycle(char setCycle) {
    testString("Setting Cycle", String(setCycle));
    if (setCycle != 'X' && setCycle != 'C' && setCycle != 'H' && setCycle != 'R') return false;
    if (setCycle == 'X') cycleRemaining = 0;
    if (setCycle == 'C') cycleRemaining = cleanCycleLength;
    if (setCycle == 'H') cycleRemaining = holdCycleLength;
    if (setCycle == 'R') cycleRemaining = runCycleLength;
    cycleType         = setCycle;
    heaterRemaining   = heaterMaximum;
  }  
  
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Stop All Cycles
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void stopCycle() {
      cycleRemaining        = -2;
      cycleType             = 'X';
      statusLight           = 0;     // Don't let it run forever if we forget
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Assign Temperature
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  int assignTemp(float temperature) { tempSet = temperature; }
   
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Process :: Handle all Variables 
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  boolean processAll() {    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check if we're below the minimum Temperature or need to hold On/Off
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (errorCycle  <= 0) { 
        digitalWrite(relayLow, LOW);     
        digitalWrite(relayHigh, LOW);
        digitalWrite(relayHeater, LOW);
        digitalWrite(relayBlower, LOW);
        digitalWrite(relayAux, LOW);
        digitalWrite(relayLight, LOW);
        return false;
    }    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check if we're below the minimum Temperature or need to hold On/Off
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    int overRideMinimum = (temperature1 < tempMin) ? 1 : 0;
    int holdStatus = (temperature1 <= tempSet) ? 1 : 0;    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check for Bad Thermistor / Bad Readings
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (temperature1 < 31 || temperature1 > 112 || temperature2 < 31 || temperature2 > 112) {
        statusError     = 105; // Frozen / Overheated / Bad Thermistor        
        cycleType       = 'X';
        overRideMinimum = 0;
        holdStatus      = 0;
        cycleRemaining  = -2;
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Handle Cycles setStatuses(setMotor,setBlower,setAux,setHeater,setLight)
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    switch (cycleType) { 
      case 'X'  : setStatuses(overRideMinimum, 0, 0, overRideMinimum, statusLight); break; // No Cycle
      case 'C'  : setStatuses(1, 0, 1, (temperature1 < tempMin), 0); break; // Clean Cycle
      case 'H'  : setStatuses(holdStatus, 0, holdStatus, holdStatus, statusLight); break; // Hold Cycle
      case 'R'  : // Run Cycle
        switch (currentMode){
          case 0 : setStatuses(0, 0, 0, 0, statusLight); break;
          case 1 : setStatuses(1, 0, 0, 1, statusLight); break;
          case 2 : setStatuses(2, 0, 0, 1, statusLight); break;
          case 3 : setStatuses(2, 1, 0, 1, statusLight); break;
          default: setStatuses(0, 0, 0, 0, 0);
        }
        break; // Handle Cycles
    }

    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Handle the Motor
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    switch(statusMotor) {
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Off
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      case 0 : 
        digitalWrite(relayLow,  LOW);
        digitalWrite(relayHigh, LOW);  
        break;
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Low (110V)
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      case 1 : 
        digitalWrite(relayLow,  HIGH);
        digitalWrite(relayHigh, LOW);
        break;      
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| High (220V)
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      case 2 : 
        digitalWrite(relayLow,  HIGH);
        digitalWrite(relayHigh, HIGH);
        break;   
      }
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Aux
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      digitalWrite(relayAux,      (statusAux == 1)     ? HIGH : LOW);
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Blower
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      digitalWrite(relayBlower,   (statusBlower == 1)  ? HIGH : LOW);
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Light
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      digitalWrite(relayLight,    (statusLight == 1)   ? HIGH : LOW);
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Errors
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      if (digitalRead(relayLow) == HIGH && checkPressure() == false) statusError = 101; // Water Pressure
      if (temperature1 < (temperature2 - tempDiff) && temperature1 > (temperature2 + tempDiff) ) statusError = 102; // Thermistors Match Readings
      if (temperature1 > tempFail || temperature2 > tempFail) statusError = 103; // Overheating over failure temperature   
      if (statusError > 0 ) digitalWrite(relayHeater,  LOW);
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Heater
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      switch(statusHeater) { 
        case 0 : digitalWrite(relayHeater,  LOW);  break;
        case 1 : if (digitalRead(relayLow) == LOW) statusError = 104; // No power to motor
                 if (temperature1 > tempSet) digitalWrite(relayHeater,  LOW); 
                 if (temperature1 <= tempSet && statusError == 0) digitalWrite(relayHeater,  HIGH);
                 break;
      }     
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Error Cycle
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      if (statusError == 0) errorCycle = 5;
      updateScreen();
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Check Pressure :: Ensure we're within our limits
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean updateScreen() {
//  UP(2)|TEMP(3)|SETTEMP(3)|MOTOR(1)-0,1,2|BLOW(1)|AUX(1)|HEAT(1)|LIGHT(1)|ERROR(3)|CYCLE(1)|REMAIN(4)|TIME(5);
//  UP|070|072|0|0|0|0|0|105|X|-002|12:00
    String fullText = "";
    fullText += "UP|";
    fullText += strPad(String(getTemp()), 3, '0') + '|';
    fullText += strPad(String(tempSet), 3, '0') + '|';
    fullText += String(statusMotor) + '|';
    fullText += String(statusBlower) + '|';
    fullText += String(statusAux) + '|';    
    fullText += String(statusHeater) + '|';        
    fullText += String(statusLight) + '|';
    fullText += strPad(String(statusError),3,'0') + '|';
    fullText += String(cycleType) + '|';    
    fullText += strPad(String(cycleRemaining),4,'0') + '|';
    fullText += String(hourFormat12()) + ':' + strPad(String(minute()),2,'0');
    Serial.println(fullText);
    char charBuf[38];    
    fullText.toCharArray(charBuf, 38);

    Wire.beginTransmission(4);
    Wire.write(charBuf);
    Wire.endTransmission();
    
    delay(500);   
  }     

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Private Functions 
//| 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  String strPad(String strToPad, int padLength, char padChar) { 
    boolean neg = (strToPad.substring(0,1) == "-");
    if (neg) strToPad = strToPad.substring(1);
    String resp = "";
    int toPad = (padLength - strToPad.length());
    if (neg) toPad = toPad - 1;
    int i = 0;
    for(i=0;i<toPad;i++) resp += padChar;
    resp += strToPad;
    return (neg) ? '-' + resp : resp;
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Set Statuses
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void setStatuses(int setMotor, int setBlower, int setAux, int setHeater, int setLight) { 
    statusMotor   = setMotor;
    statusHeater  = setHeater;
    statusLight   = setLight;
    statusBlower  = setBlower;
    statusAux     = setAux;
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Handle Serial Commands
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void handleSerial(byte incomingByte) { 
    if (incomingByte == 119) buttonPress(btnUpLow,    false);      // KeyPress W
    if (incomingByte == 115) buttonPress(btnDownLow,  false);    // KeyPress S
    if (incomingByte == 97)  buttonPress(btnLightLow, false);   // KeyPress A
    if (incomingByte == 100) buttonPress(btnModeLow,  false);    // KeyPress D 
    if (incomingByte == 99)  setTime(22,59,58,21,11,14); // KeyPress C Test Cleaning Cycle
    if (incomingByte == 104) setTime(17,59,58,22,11,14); // KeyPress H Test Hold Cycle
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Set Temperature
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  int convertTemperature(int sensorTemp) { 
     double Temp;
     Temp = log(((10240000/sensorTemp) - 10000));
     Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
     Temp = Temp - 273.15;              // Convert Kelvin to Celsius
     Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
     return int(Temp);
  }
 
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Check Timer :: Increments the Cycle Timer
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean cycleTime() {
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Fetch the Actual Minute
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    int actualMinute = minute();
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Exit if it's still the same minute
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (currentMin == actualMinute) return false;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Handle Current Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (cycleRemaining > -1) cycleRemaining--;
    if (cycleRemaining == 0 && cycleType == 'R') startCycle('H');
    if (cycleRemaining == -1) stopCycle();    
    testInteger("Cycles Remaining", cycleRemaining);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Error Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (statusError > 0 && errorCycle > 0) errorCycle--;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check for Hold Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (cycleType == 'X' && holdCycleStart == hour() && minute() == 0) { 
      int i;
      for (i = 0; i < sizeof(holdCycleDay) - 1; i++){
         if (holdCycleDay[i] == weekday()) startCycle('H');
      }
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check for Clean Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (cycleType == 'X' && cleanHourStart == hour() && minute() == 0) { 
      startCycle('C');
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Heater Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (heaterRemaining >= (heaterTimeout * -1)) heaterRemaining--;
    if (heaterRemaining <= 0) statusHeater = 0;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Handle Light Remaining :: Don't want to burn out that bulb
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (lightRemaining > -1) lightRemaining--;
    if (lightRemaining == -1) statusLight = 0;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Increment Current Minute
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    currentMin = minute();
  }

  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Test Log
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void testInteger(String message, int data) { 
    String strSep = " => ";
    Serial.println(message + strSep + data);
  }
  
  void testString(String message, String data) { 
    String strSep = " => ";
    Serial.println(message + strSep + data);
  }
  
