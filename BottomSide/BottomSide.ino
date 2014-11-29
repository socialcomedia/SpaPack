//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  SpaPack v1.0
//|
//|  copyright MIT License 2014 :: Ralph C. Ferrara Jr.
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  #include <Time.h> 
  #include <math.h>
  #include <ST7735.h>  

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Version
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const float thisVer      = 1.03;
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Sensor Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const int sensorPressure  = 6;        // Pressure Sensor
  const int sensorTemp	    = A2;       // Temperature Sensor
  const int sensorHiTemp    = A3;       // High Limit Sensor
  const int inputButton     = A4;       // Analog Input for Button
  
  const int relayLight      = 12;       // Digital Pinout for Lighting   
  const int relayBlower     = 11;       // Digital Pinout for Blower
  const int relayAux        = 10;       // Digital Pinout for Aux/Ozone  (110V 10AMP SSR)
  const int relayHigh       = 9;        // Digital Pinout for Motor High (110V 10AMP SSR)
  const int relayLow        = 8;        // Digital Pinout for Motor Low  (110V 10AMP SSR)
  const int relayHeater     = 7;        // Digital Pinout for Heater     (220V 30AMP SSR)

  const int btnLightLow     = 101;      // Analog Low for Light Button
  const int btnLightHigh    = 200;      // Analog High for Light Button

  const int btnModeLow      = 201;      // Analog Low for Mode Button
  const int btnModeHigh     = 300;      // Analog High for Mode Button

  const int btnUpLow        = 301;      // Analog Low for Temp Up Button
  const int btnUpHigh       = 400;      // Analog High for Temp Up Button

  const int btnDownLow      = 401;      // Analog Low for Temp Down Button
  const int btnDownHigh     = 500;      // Analog High for Temp Down Button
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  High Lows
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int tempFail              = 108;      // Maximum Temperature
  int tempMax               = 105;      // Maximum Configurable Temperature Setting
  int tempMin               = 45;       // Hard Minimum Temperature Setting  
  int pressureMin           = 1;        // Minimum Pressure Setting
  int pressureMax           = 1024;     // Maximum Pressure Setting
    
  int calibrateOffset       = 0;
  
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Analog Readings
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int temperature1          = 70;      // Current Temperature Probe #1
  int temperature2          = 70;      // Current Temperature Probe #1  
  int tempCount             = 0;       // How many times we've sampled temperature
  int tempTotal1            = 0;       // Total Temperature Count
  int tempTotal2            = 0;       // Total Temperature Count  

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Current Statuses
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  int statusLight           = 0;        // 0 = Off / 1 = On
  int statusMotor           = 0;        // 0 = Off / 1 = Low / 2 = High
  int statusHeater          = 0;        // 0 = Off / 1 = On
  int statusBlower          = 0;        // 0 = Off / 1 = On
  int statusAux             = 0;        // 0 = Off / 1 = On    

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
    //| Get Start Up Temperatures
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    checkTemp();    
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
    cycleTime();
    checkTemp();
    processAll();
    checkPressure();
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Data Retrieval
//| Buttons, Sensors Etc
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Check Pressure :: Ensure we're within our limits
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean checkPressure() {
    if (digitalRead(sensorPressure) == HIGH) Serial.println("OK"); else Serial.println("FAIL");
    return (digitalRead(sensorPressure) == HIGH);
  } 
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void checkTemp() { 
     temperature1 = convertTemperature(analogRead(sensorTemp));
     temperature2 = convertTemperature(analogRead(sensorHiTemp));
     testInteger("Current Temperature #1", temperature1);
     testInteger("Current Temperature #2", temperature2);      
     delay(1000);
  }
 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Button Interrupt
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void buttonPress(int testButton) { 
    int button     = 0;
    int buttonRead = analogRead(inputButton);
    if (testButton > -1) buttonRead = testButton; // Handle Test Mode Serial
    if (buttonRead >= btnLightLow && buttonRead <= btnLightHigh) button = 1;
    if (buttonRead >= btnModeLow && buttonRead <= btnModeHigh)   button = 2;
    if (buttonRead >= btnUpLow && buttonRead <= btnUpHigh)       button = 3;
    if (buttonRead >= btnDownLow && buttonRead <= btnDownHigh)   button = 4;    
    testInteger("Button Press", button);
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
    if (button != 0) delay(250); // Prevent Bounce
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
  
  void processAll() { 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Check if we're below the minimum Temperature or need to hold On/Off
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    int overRideMinimum = (temperature1 < tempMin) ? 1 : 0;
    if (overRideMinimum == 1) testInteger("OverRide Minimum", temperature1);    
    int holdStatus = (temperature1 <= tempSet) ? 1 : 0;    
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
      //| Handle the Heater
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      switch(statusHeater) { 
        case 0 : digitalWrite(relayHeater,  LOW);  break;
        case 1 : 
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| Make sure we have water pressure
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (checkPressure() == false) {
            digitalWrite(relayHeater,  LOW); 
            testString("Pressure Low", "Error");
            break;
          }
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| Make sure our thermometers are ok and somewhat synced
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (temperature1 != temperature2 && temperature1 != (temperature2 + 1) && temperature1 != (temperature2 - 1)) {
            digitalWrite(relayHeater,  LOW);
            testString("Thermometer Temperatures Inconclusive", "Error");
            break;
          }
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| See if we're above Maximum Temperature
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (temperature1 > tempFail || temperature2 > tempFail) { 
            digitalWrite(relayHeater,  LOW); 
            testString("Maximum Temperature Overage", "Error");            
            break;
          }
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| If the Motor isn't getting power turn off heater
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (digitalRead(relayLow) == LOW) { 
            digitalWrite(relayHeater,  LOW);
            testString("Can not run heater without motor running", "Error");            
            break;
          }          
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| If We're over set temperature :: Normal 
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (temperature1 > tempSet) { 
            digitalWrite(relayHeater,  LOW); 
            testString("Over Set Temperature", "Normal");            
            break;
          }
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
          //| Hey! It's cold, turn the heater on damnit
          //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
          if (temperature1 <= tempSet) { 
            digitalWrite(relayHeater,  HIGH);
          }
          break;
      }      
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Private Functions 
//| 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

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
    if (incomingByte == 119) buttonPress(btnUpLow);      // KeyPress W
    if (incomingByte == 115) buttonPress(btnDownLow);    // KeyPress S
    if (incomingByte == 97)  buttonPress(btnLightLow);   // KeyPress A
    if (incomingByte == 100) buttonPress(btnModeLow);    // KeyPress D 
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
     Serial.println(Temp);
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
    //| Test Mode
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    if (testMode == true) {actualMinute = currentMin + 1; delay(500);} // Fake a minute in 500ms
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
  
