//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  SpaPack v1.0
//|
//|  copyright Ralph Ferrara Jr.
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  #include <Time.h> 
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Declare Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const int sensorTemp            = A1;       // Temperature Sensor
  const int sensorPressure        = A2;       // Pressure Sensor
  const int relayLight            = 12;       // Digital Pinout for Lighting   
  const int relayBlower           = 11;       // Digital Pinout for Blower
  const int relayAux              = 10;       // Digital Pinout for Aux/Ozone  (110V 10AMP SSR)
  const int relayHigh             = 9;        // Digital Pinout for Motor High (110V 10AMP SSR)
  const int relayLow              = 8;        // Digital Pinout for Motor Low  (110V 10AMP SSR)
  const int relayHeater           = 7;        // Digital Pinout for Heater     (220V 30AMP SSR)

  const int inputMode             = 6;        // Digital Input for Mode Button
  const int inputUp               = 5;        // Digital Input for Temperature Up
  const int inputDown             = 4;        // Digital Input for Temperature Down
  const int inputLight            = 3;        // Digital Input for Light
  
  int tempCurrent           = 40;       // Current Temperature
  int tempSet               = 72;       // Temperature Currently Set To
  int tempMax               = 105;      // Maximum Temperature Setting
  int tempMin               = 45;       // Hard Minimum Temperature Setting  
  int pressure              = 0;       // Pressure Setting
  int pressureMin           = 1;       // Minimum Pressure Setting
  int pressureMax           = 2;       // Maximum Pressure Setting
  
  int statusLight           = 0;        // 0 = Off / 1 = On
  int statusMotor           = 0;        // 0 = Off / 1 = Low / 2 = High
  int statusHeater          = 0;        // 0 = Off / 1 = On
  int statusBlower          = 0;        // 0 = Off / 1 = On
  int statusAux             = 0;        // 0 = Off / 1 = On    

  int currentMode           = 0;        // Mode (0 = Off, 1 = Low, 2 = High, 3 = High, 4 = Low/Light 5 = High/Light 6 = High/Blower/Light)
  
  int debounceTemp          = 0;        // Debounce button presses for Up/Down
  int debounceMode          = 0;        // Debounce button presses for Mode
  int debounceLight         = 0;        // Debounce button presses for Light  
    
  int cleanCycle            = -1;       // Are we currently Cleaning the Spa?    
  int cleanCycleLength      = 60;       // Clean Cycle Minutes 
  int cleanHourStart        = 23;       // What time should we run the clean cycle

  int runCycle              = -1;       // If we're in a cycle, keep track of how long we're running
  int runCycleLength        = 20;       // How long should we run for when we're in the spa
  
  int holdCycle             = -1;       // If we're in a hold temperature cycle, keep track of how long we're running
  int holdCycleLength       = 240;      // How many minutes should we hold the Temperature
  int holdCycleDay[]        = {0,6,7};  // Which days should we have a hold cycle on? ( 0 = Sunday / 7 = Saturday )
  int holdCycleStart        = 17;       // What hour should our Hold Period Start
  
  int heaterWaitLength      = 5;        // How long to wait before re-igniting the heater MUST BE POSITIVE NUMBER
  int heaterCycle           = 0;        // Keep track of how long it's been 
  int heaterMaxLength       = 240;      // Maximum Length the heater should EVER run
  
  int currentMin    = 0;        // How we determine the current minute        

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Setup Loop
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void setup() {
    Serial.begin(9600);
    Serial.println("Starting Up"); 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setting the Default Date/Time
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    setTime(12,0,0,1,1,11);    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setup Digital Pins
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    pinMode(sensorPressure,   INPUT);
    pinMode(relayLight,       OUTPUT);
    pinMode(relayHigh,        OUTPUT);
    pinMode(relayLow,         OUTPUT);  
    pinMode(relayHeater,      OUTPUT);
    pinMode(relayBlower,      OUTPUT);    
    pinMode(relayAux,         OUTPUT);        
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Setup Buttons
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    pinMode(inputUp,          INPUT);
    pinMode(inputDown,        INPUT);    
    pinMode(inputMode,        INPUT);
    pinMode(inputLight,       INPUT);
    pinMode(2,                INPUT); //Interrupt Pin
    attachInterrupt(0, buttonPress, CHANGE);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Disable Everything at First
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    digitalWrite(relayLight,   LOW);   
    digitalWrite(relayHigh,    LOW);   
    digitalWrite(relayLow,     LOW);   
    digitalWrite(relayHeater,  LOW);   
    digitalWrite(relayAux,     LOW);
    digitalWrite(relayBlower,  LOW);  
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Disable Everything at First
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    heaterWaitLength  = heaterWaitLength * -1;
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Main Loop
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void loop() {
    byte incomingByte;
    if (Serial.available() > 0) {    
        incomingByte = Serial.read();
        if (incomingByte == 119) debounceTemp = 1;
        if (incomingByte == 115) debounceTemp = -1;
        if (incomingByte == 97) debounceMode = 1;
        if (incomingByte == 100) debounceLight = 1;
      }    
    cycleTime();
    modeCheck();
    handleHeat();
    modePress();
    tempPress();
    lightPress();
    process();
    delay(250);
  }
 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Mode Intetrupt
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void buttonPress() { 
    Serial.println("Button Pressed!");
    if (digitalRead(inputMode)  == HIGH) debounceMode  =  1; 
    if (digitalRead(inputUp)    == HIGH) debounceTemp  =  1; 
    if (digitalRead(inputDown)  == HIGH) debounceTemp  = -1; 
    if (digitalRead(inputLight) == HIGH) debounceLight =  1; 
  }
 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  int setTemp(float temperature) { tempSet = temperature; }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Get Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  int getTemp() { 
    return(72);
    return(tempSet); 
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Increment Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void incTemp() { 
    tempSet = (tempSet == tempMax) ? tempMax : tempSet + 1;
    Serial.println("Temperature Increased to...");
    Serial.println(tempSet);
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Decrease Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void decTemp() { 
    tempSet = (tempSet == tempMin) ? tempMin : tempSet -1; 
    Serial.println("Temperature Decreased to...");
    Serial.println(tempSet);    
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Set Motor Speed
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void setMotorSpeed() { 
    if (statusMotor == 0) stopCycles(); else startRunCycle();
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Toggle Motor Speed
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void incrementMotorSpeed() {     
    statusMotor         = statusMotor++; 
    if (statusMotor == 3) statusMotor = 0;
    setMotorSpeed();
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Begin a Cycle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void startRunCycle() {
    runCycle            = 0;
    cleanCycle          = -1;
    holdCycle           = 0;    
  }  
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Begin a Clean Cycle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void startCleanCycle() {
    cleanCycle            = 0;
    runCycle              = -1;
    currentMode           = -1;    
  }   
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Begin a Clean Cycle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void startHoldCycle() {
    holdCycle            = 0;
    currentMode          = -1;
  }   
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Stop Hold Cycle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void stopHoldCycle() {
    holdCycle            = -1;
  }   
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Begin a Run Cycle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void stopCycles() {
    cleanCycle          = -1;
    runCycle            = -1;
    statusMotor         = 0;
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Check Timer :: Increments the Cycle Timer
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean cycleTime() {
    if (currentMin == minute()) return false;
    currentMin = minute();    
    if (cleanCycle >= 0)    cleanCycle++;  
    if (runCycle   >= 0)    runCycle++; 
    if (holdCycle  >= 0)    holdCycle++;
    heaterCycle++;
    if (cleanCycle == cleanCycleLength || runCycle == runCycleLength || holdCycle == holdCycleLength) stopCycles();
    return true;
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Check Pressure
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean pressureOK() {
    pressure = analogRead(sensorPressure);
    return true;
    return (pressure >= pressureMin && pressure <= pressureMax);
  } 
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Check Timer :: Increments the Cycle Timer
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  boolean handleHeat() {
    if (runCycle == -1 && holdCycle == -1) stopHeater();
    if (heaterCycle >= heaterMaxLength) stopHeater();
    if (heaterCycle > 0 && heaterCycle <= heaterMaxLength && statusHeater == 0) startHeater(); 
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Start Heater
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void startHeater() { 
    if (pressureOK) stopHeater();
    digitalWrite(relayHeater, HIGH); 
    statusHeater = 1;
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Stop 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void stopHeater() { 
    digitalWrite(relayHeater, LOW);  
    statusHeater = 0;
    heaterCycle  = heaterWaitLength;
  } 

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Mode Button Press
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void modePress() { 
    if (debounceMode > 0) currentMode++;
    if (currentMode > 3)  currentMode = 0;
    debounceMode = 0;
  }  
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Temperature Press
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void tempPress() { 
    if (debounceTemp ==  1) incTemp();
    if (debounceTemp == -1) decTemp();
    debounceTemp = 0;
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Temperature Press
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void lightPress() { 
    if (debounceLight ==  1) statusLight = (statusLight == 0) ? 1 : 0;
    debounceLight = 0;
  }  
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Mode Set
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void modeSet(int motorSpeed, int blowerOn) { 
    if (motorSpeed >  0 && runCycle == -1) startRunCycle();
    if (motorSpeed == 0 && runCycle >= 0)  runCycle = 0 ;    
    statusMotor  = motorSpeed;
    statusBlower = blowerOn;   
  }
   
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Mode Toggle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  void modeCheck() {
    switch(currentMode) {
      case -1 : break;                  // Handling a Cycle | No Controls 
      case 0  : modeSet(0,0); break;  // Off
      case 1  : modeSet(1,0); break;  // Motor Low  | No Blower
      case 2  : modeSet(2,0); break;  // Motor High | No Blower
      case 3  : modeSet(2,1); break;  // Motor High | Blower
    }
  }      

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Process :: Handle all Variables 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  void process() { 
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Motor
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      switch(statusMotor) { 
        case 0 : 
          stopHeater();
          digitalWrite(relayLow,  LOW);
          digitalWrite(relayHigh, LOW);  
          digitalWrite(relayAux,  LOW);                    
          break;
        case 1 : 
          digitalWrite(relayLow,  HIGH);
          digitalWrite(relayHigh, LOW);
          digitalWrite(relayAux,  HIGH);          
          break;          
        case 2 : 
          digitalWrite(relayLow,  HIGH);
          digitalWrite(relayHigh, HIGH);
          digitalWrite(relayAux,  HIGH);                    
          break;   
      }
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Blower
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
      switch(statusBlower) { 
        case 0 : digitalWrite(relayBlower,  LOW);  break;
        case 1 : digitalWrite(relayBlower,  HIGH); break;
      }
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
      //| Handle the Light
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      switch(statusLight) { 
        case 0 : digitalWrite(relayLight,  LOW);  break;
        case 1 : digitalWrite(relayLight,  HIGH); break;
      }
      
  }
  
