#include <Servo.h>

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  SpaPack v1.0
//|
//|  copyright MIT License 2014 :: Ralph C. Ferrara Jr.
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  #include <Time.h> 
  #include <math.h>
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7735.h> // Hardware-specific library
  #include <SPI.h> 

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Version
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const float thisVer      = 1.11;
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Sensor Variables
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  const int sensorPressure  = 12;        // Pressure Sensor
  const int sensorTemp	    = A2;       // Temperature Sensor
  const int sensorHiTemp    = A3;       // High Limit Sensor
  const int inputButton     = A4;       // Analog Input for Button
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Create the TFT/LCD
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  const int lcdSCLK         = 13;       // Default Pins/Just Reminding
  const int lcdMosi         = 11;       // Default Pins/Just Reminding
  const int lcdCS           = 10;
  const int lcdDC           = 9;
  const int lcdReset        = 0;  
  String lcdStatus          = "";   // Used to determine when to update the screen
  Adafruit_ST7735 tft = Adafruit_ST7735(lcdCS,  lcdDC, lcdReset);    

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Relays
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  const int relayLight      = 7;       // Digital Pinout for Lighting   
  const int relayBlower     = 6;       // Digital Pinout for Blower
  const int relayAux        = 5;       // Digital Pinout for Aux/Ozone  (110V 10AMP SSR)
  const int relayHigh       = 4;        // Digital Pinout for Motor High (110V 10AMP SSR)
  const int relayLow        = 3;        // Digital Pinout for Motor Low  (110V 10AMP SSR)
  const int relayHeater     = 2;        // Digital Pinout for Heater     (220V 30AMP SSR)

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Button Resistances
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

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

  int tempFail              = 108;      // Maximum Temperature before failing
  int tempMax               = 105;      // Maximum Configurable Temperature Setting
  int tempMin               = 45;       // Hard Minimum Temperature Setting  
  int tempDiff              = 3;        // Maximum Temperature Differential (T1/T2)
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Analog Readings
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int temperature1          = -1;      // Current Temperature Probe #1
  int temperature2          = -1;      // Current Temperature Probe #1  
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
    //| Get Start Up Temperatures
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    checkTemp();        
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Start up the TFT
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    tft.initR(INITR_BLACKTAB);
    clearScreen();
    tft.setRotation(1);  
    tft.setTextWrap(true);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.println(" ");        
    tft.println("SpaPack v" + String(thisVer));        
    tft.println("Ralph C Ferrara Jr. 2014.");
    tft.println(" ");
    tft.println("Current Temperature : " + String(getTemp()) + "f");   
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
      tft.println("Test Mode On!..");    
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Here we Go!
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    tft.println("Loading..");
    delay(100);
    clearScreen();
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
    temperature1 = 75;
    temperature2 = 75;
    return false;
    
    
    tempTotal1 += analogRead(sensorTemp);
    tempTotal2 += analogRead(sensorHiTemp);    
    tempCount++;
    if (tempCount > 1000 || temperature1 == -1 || temperature2 == -1) { 
      temperature1 = convertTemperature(int(tempTotal1 / tempCount));
      temperature2 = convertTemperature(int(tempTotal2 / tempCount));
      tempTotal1 = 0;
      tempTotal2 = 0;
      tempCount  = 0;       
    } 
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
      //| Handle the Errors
      //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||          
      statusError = 0;
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
    String textX = "";    
    int offsetX  = 0;   
    int colorBG  = 0; 
    int colorX   = 0;    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Create Comparison String 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    textX = String(tempSet) + String(getTemp()) + String(digitalRead(relayLow)) + String(digitalRead(relayHigh)) + String(digitalRead(relayHeater)) +  String(digitalRead(relayBlower)) + String(digitalRead(relayLight)) + String(currentMode) + String(cycleType);
    if (textX == lcdStatus) return(false);
    lcdStatus = textX;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Create Areas x,y,w,h
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    tft.setTextWrap(false);    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Create Areas x,y,w,h
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
//    clearScreen();    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Main Temperature
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    tft.fillRect(0,     0, 115, 126, (statusError == 0) ? ST7735_BLACK : ST7735_RED);    
    textX    = String(getTemp());    
    offsetX  = tftOffset(textX, 42, 22, 3, 0, 0);
    tft.setCursor(offsetX,26);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(6);
    tft.print(textX);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| ERROR CODE
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||        
    tft.setTextColor(ST7735_BLUE);
    tft.setTextSize(1);
    tft.setCursor(1,1);        
    switch(statusError) { 
      case 101 : tft.print("LOW PRESSURE"); break;
      case 102 : tft.print("TEMP 2 DIFFERENT"); break;
      case 103 : tft.print("TEMP OVERAGE"); break;
      case 104 : tft.print("MOTOR OFF"); break;      
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Set Temperature
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||        
    textX    = String(tempSet);
    offsetX  = tftOffset(textX, 50, 43, 26, 0, 0);
    tft.setCursor(offsetX,80);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(3);
    tft.print(textX);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Farenheit
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    tft.setCursor(100,15);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.print("F");
    tft.setCursor(100,80);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.print("F");    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Motor Speed
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    colorBG = (digitalRead(relayHigh) == HIGH || digitalRead(relayLow) == HIGH) ? ST7735_WHITE : ST7735_BLACK;
    colorX  = (digitalRead(relayHigh) == HIGH || digitalRead(relayLow) == HIGH) ? ST7735_BLACK : ST7735_WHITE;
    textX  = "OFF";
    textX  = (digitalRead(relayHigh) == HIGH || digitalRead(relayLow) == HIGH)  ? "LOW" : textX;
    textX  = (digitalRead(relayHigh) == HIGH) ? "MAX" : textX;
    tft.fillRect(118,   0,  42,  24, colorBG);        
    tft.setCursor(122,5);
    tft.setTextColor(colorX);
    tft.setTextSize(2);
    tft.print(textX); 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Heater
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    colorBG = (digitalRead(relayHeater) == HIGH) ? ST7735_WHITE : ST7735_BLACK;
    colorX  = (digitalRead(relayHeater) == HIGH) ? ST7735_BLACK : ST7735_WHITE;
    tft.fillRect(118,  25,  42,  24, colorBG);
    tft.setCursor(127,34);
    tft.setTextColor(colorX);
    tft.setTextSize(1);
    tft.print("HEAT");
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Blower
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    colorBG = (digitalRead(relayBlower) == HIGH) ? ST7735_WHITE : ST7735_BLACK;
    colorX  = (digitalRead(relayBlower) == HIGH) ? ST7735_BLACK : ST7735_WHITE;
    tft.fillRect(118,  50,  42,  24, colorBG);    
    tft.setCursor(130,59);
    tft.setTextColor(colorX);
    tft.setTextSize(1);
    tft.print("AIR");
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Light
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    colorBG = (digitalRead(relayLight) == HIGH) ? ST7735_WHITE : ST7735_BLACK;
    colorX  = (digitalRead(relayLight) == HIGH) ? ST7735_BLACK : ST7735_WHITE;
    tft.fillRect(118,  75,  42,  24, colorBG);        
    tft.setCursor(124,84);
    tft.setTextColor(colorX);
    tft.setTextSize(1);
    tft.print("LIGHT");
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //| Write the Cycle
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    switch (cycleType) { 
      case 'X'  : textX = "IDLE"; break; // No Cycle
      case 'C'  : textX = "CLEAN"; break; // Clean Cycle
      case 'H'  : textX = "HOLD";  break; // Hold Cycle
      case 'R'  : 
        textX = "RUN"; 
        //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
        //| Write the Mode Steps
        //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
        switch (currentMode){
          case 0 : 
            tft.fillRect(27, 120, 20, 6, ST7735_BLACK);    
            tft.fillRect(49, 120, 20, 6, ST7735_BLACK);    
            tft.fillRect(71, 120, 20, 6, ST7735_BLACK);                
            break;
          case 1 : 
            tft.fillRect(27, 120, 20, 6, ST7735_WHITE);    
            tft.drawRect(49, 120, 20, 6, ST7735_WHITE);    
            tft.drawRect(71, 120, 20, 6, ST7735_WHITE);    
            break;          
          case 2 : 
            tft.fillRect(27, 120, 20, 6, ST7735_WHITE);    
            tft.fillRect(49, 120, 20, 6, ST7735_WHITE);    
            tft.drawRect(71, 120, 20, 6, ST7735_WHITE);    
            break;          
          case 3 : 
            tft.fillRect(27, 120, 20, 6, ST7735_WHITE);    
            tft.fillRect(49, 120, 20, 6, ST7735_WHITE);    
            tft.fillRect(71, 120, 20, 6, ST7735_WHITE);    
            break;          
        }      
        break;
    }    
    offsetX  = tftOffset(textX, 150, 140, 130, 125, 120);    
    colorBG = (textX != "IDLE") ? ST7735_WHITE : ST7735_BLACK;
    colorX  = (textX != "IDLE") ? ST7735_BLACK : ST7735_WHITE;
    colorX  = (textX != "IDLE") ? ST7735_BLACK : ST7735_WHITE;
    colorX  = (textX != "IDLE") ? ST7735_BLACK : ST7735_WHITE;    
    tft.fillRect(118, 100,  42,  26, colorBG);            
    tft.setCursor(offsetX,110);
    tft.setTextColor(colorX);
    tft.setTextSize(1);
    tft.print(textX);    
  } 

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//| Private Functions 
//| 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    


  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
  //| Set Offset for LCD Text
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    

  int tftOffset(String text, int len1, int len2, int len3, int len4, int len5) { 
    if (text.length() == 1) return(len1);
    if (text.length() == 2) return(len2);
    if (text.length() == 3) return(len3);    
    if (text.length() == 4) return(len4);
    if (text.length() == 5) return(len5);
    return(len1);
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
  //| Clear Screen
  //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
  
  void clearScreen() { 
    tft.fillScreen(ST7735_BLACK);    
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
  
