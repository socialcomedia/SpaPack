//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  SpaPack v1.0
//|  Topside.ino
//|  copyright MIT License 2014 :: Ralph C. Ferrara Jr.
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  #include <TFT.h>
  #include <Wire.h>    
 
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  TFT Color Bits
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
  
  #define BLUE_BITS 5
  #define GREEN_BITS 6
  #define RED_BITS 5  

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Current Settings
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  boolean statusLight           = false;        // 0 = Off / 1 = On
  boolean statusMotor           = false;        // 0 = Off / 1 = Low / 2 = High
  boolean statusHeater          = false;        // 0 = Off / 1 = On
  boolean statusBlower          = false;        // 0 = Off / 1 = On
  boolean statusAux             = false;        // 0 = Off / 1 = On    
  boolean statusError           = false;
  char currentCycle         = 'Z';       // Current Cycle
  String currentTime        = "";        // Current Time
  int currentError          = 0;         // Current Error
  int currentTemp           = -1;        // Currently Displaying Temperature
  int cycleRemain           = -1;        // Time Remaining in Cycle  
  int tempSet               = -1;        // Temperature Currently Set To    

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Globals
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  boolean synced = false;
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Setup
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void setup() {
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Serial
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    Serial.begin(9600);
    Serial.println("Booting!");
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Setup Display
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    Tft.init();  //init TFT library
    Tft.setDisplayDirect(DOWN2UP);    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Start Communications
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    char* attempt = "Attempting Communications with Master..";
    skinnyText(attempt, 110, textLeft(attempt, 150, 1, 6), 1, WHITE);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Start Wire Sync
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    Serial.println("Setting up Wire");
    Wire.begin(4);                // join i2c bus with address #4
    Wire.onReceive(i2cReceive);   // register event
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Main Loop
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void loop(){
    delay(100);
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  i2C Receive
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void i2cReceive(int howMany) { 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Initialize
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    if (synced == false) { 
      Serial.println("Syncing");
      makeClear();
      buttonLight(true);    
      buttonLight(false);    
      buttonBlower(true);    
      buttonBlower(false);    
      buttonHeater(true);    
      buttonHeater(false);    
      cycle('X',0);
      time("12:00");   
      synced = true;
    }
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Get it All
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    String fullText = "";
    Serial.println("Received Something");
    while(Wire.available()) {
        char c = Wire.read();
        fullText = fullText + c;
    }
    Serial.println("Received : " + fullText);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Update
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
//  TEMP(3)|SETTEMP(3)|MOTOR(1)|BLOW(1)|HEAT(1)|LIGHT(1)|ERROR(3)|CYCLE(1)|REMAIN(4)|TIME(5)
//  0721000000000X-00211:11
    char cycleLetter = 'X';
    temperature(fullText.substring(0,3).toInt());
    setTemp(fullText.substring(3,6).toInt());
    int motorLevel = fullText.substring(6,7).toInt();
    buttonBlower((fullText.substring(7,8).toInt() == 1));
    buttonHeater((fullText.substring(8,9).toInt() == 1));
    buttonLight((fullText.substring(9,10).toInt() == 1));
    error(fullText.substring(10,13).toInt());
    cycleLetter = fullText[13];
    testString("Error", fullText.substring(10,13));
    if (cycleLetter == 'R') cycleLetter = 'L'; //Low Run Cycle
    if (cycleLetter == 'L' && (motorLevel == 2)) cycleLetter = 'M'; //High Run Cycle        
    Serial.println("Mode : " + cycleLetter);
    cycle(cycleLetter, fullText.substring(14,18).toInt());
    time(fullText.substring(18,23));
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Wifi Function
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void wifi(int signal, char* ssidName) { 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Wifi Function
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    int backColor  = (signal > 0 && signal < 10)   ? makeColor(155,155,155)  : RED;
    int frontColor = (signal > 0 && signal < 10)  ? BLACK                   : WHITE;    
    int deadColor  = makeColor(100,100,100);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Background Box
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeRect(0,0,240,25,backColor);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Wifi Symbol
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeRect(15, 12,   3,  8, frontColor);        
    makeRect(3,   4,  18,  2, frontColor);
    makeRect(7,   6,  14,  2, frontColor);
    makeRect(11,  8,  10,  2, frontColor);    
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Signal Strength Dead
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeRect(15, 19,  4,  8, deadColor);        
    makeRect(9,  25,  4, 14, deadColor);        
    makeRect(3,  31,  4, 20, deadColor);        
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Signal Strength Dead
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    if (signal >= 0) makeRect(15, 19,  4,  8, frontColor);        
    if (signal >= 3) makeRect(9,  25,  4, 14, frontColor);        
    if (signal >= 6) makeRect(3,  31,  4, 20, frontColor);        
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Background Box
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeText(ssidName, 9, 45, 1, frontColor);
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Main Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void temperature(int temp) { 
    if (temp == currentTemp) return;
    makeRect(76, 10,  220, 90, BLACK); 
    String tempS = String(temp);
    char charBuf[4];    
    tempS.toCharArray(charBuf, 4);
    skinnyText(charBuf, 80, textLeft(charBuf, 100, 10, 6), 10, WHITE);    
    if (tempS.length() == 2) skinnyText("o", 72, 175, 3, WHITE); else skinnyText("o", 72, 205, 3, WHITE);     
    currentTemp = temp;
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void setTemp(int temp) { 
    if (temp == tempSet) return;    
    makeRect(195, 5,  80,  40, BLACK); 
    String tempS = String(temp);
    char charBuf[4];    
    tempS.toCharArray(charBuf, 4);
    skinnyText(charBuf, 205, 10, 3, WHITE); 
    if (tempS.length() == 2) skinnyText("o", 202, 50, 1, WHITE); else skinnyText("o", 202, 70, 1, WHITE);     
    tempSet = temp;
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Button Light
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void buttonLight(boolean onOff) {
    if (statusLight == onOff) return;
    if (onOff == true) { 
      makeRect(107, 240,  80, 43, makeColor(30,30,30));
      makeRect(107, 240,  6,  43, WHITE);
    } else makeRect(107, 240,  81,  43, makeColor(20,20,20));
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Icon
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeRect(131, 279,  10,  10, makeColor(200,200,200));        
    makeRect(131, 281,  6,  9, makeColor(150,150,150));            
    makeCircle(124, 283, 9, WHITE);  
    makeRect(127, 282,  4,  8, makeColor(150,150,150));              
    statusLight = onOff;
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Button Bubbles
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void buttonBlower(boolean onOff) {
    if (statusBlower == onOff) return;            
    if (onOff == true) { 
      makeRect(151, 240,  80, 43, makeColor(30,30,30));      
      makeRect(151, 240,  6,  43, WHITE);
    } else makeRect(151, 240,  80,  43, makeColor(20,20,20));
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Icon
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeCircle(160, 280, 3, WHITE);  
    makeCircle(160, 300, 5, WHITE);          
    makeCircle(170, 270, 7, WHITE);      
    makeCircle(175, 290, 10, WHITE);      
    makeCircle(182, 276, 3, WHITE);      
    statusBlower = onOff;
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Heater
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void buttonHeater(boolean onOff) { 
    if (statusHeater == onOff) return;                
    if (onOff == true) { 
      makeRect(195, 240,  80, 45, makeColor(30,30,30));
      makeRect(195, 240,  6,  45, WHITE); 
    } else makeRect(195, 240,  80, 45, makeColor(20,20,20));
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Icon
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeCircle(205, 283,  3, WHITE);          
    makeRect(207,   281,  6, 15, WHITE);          
    makeCircle(225, 283,  6, WHITE);          
    makeRect(210,   282,  4, 10, RED);          
    makeCircle(225, 283,  5, RED);       
    statusHeater = onOff;    
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Time
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void time(String timeNow) { 
    if (currentTime == timeNow) return;                
    currentTime = timeNow;
    String tempS = String(timeNow);
    char charBuf[6];    
    tempS.toCharArray(charBuf, 6); 
    makeRect(195, 130,  105,  40, BLACK);     
    skinnyText(charBuf, 205, textLeft(charBuf, 180, 3, 6), 3, WHITE);
  }  
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Set Temperature
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void cycle(char cycleType, int cycleLeft) { 
    testInteger("Remain", cycleLeft);
    testString("Cycle", String(cycleType));
    if (currentCycle == cycleType && cycleLeft == cycleRemain) return;                
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Defaults
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    currentCycle = cycleType;
    cycleRemain  = cycleLeft;
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Draw it
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeRect(0,  240,  106,  80, makeColor(20,20,20));    
    String descTop  = "";
    String descBot  = "";
    int descColor   = BLACK;
    switch(cycleType) {
        case 'X' : descTop    = "OFF";                descBot    = "Press Start";                descColor  = makeColor(100,100,100); break;
        case 'C' : descTop    = String(cycleLeft);    descBot    = "Cleaning";                   descColor  = makeColor(204,102,255); break; // Blue
        case 'H' : descTop    = String(cycleLeft);    descBot    = "Holding";                    descColor  = makeColor(51,173,51); break; // Green
        case 'M' : descTop    = "MAX";                descBot    = String(cycleLeft) + " min";   descColor  = RED; break; // Red
        case 'L' : descTop    = "LOW";                descBot    = String(cycleLeft) + " min";   descColor  = makeColor(255,255,51); break; // Yellow
    }
    makeRect(0, 240,  106,  106, makeColor(30,30,30));
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Big Letter
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||    
    String tempS = String(descTop);
    char charBuf[50];    
    tempS.toCharArray(charBuf, 50); 
    skinnyText(charBuf, 30, textLeft(charBuf, 278, 4, 6), 4, descColor); 
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Cycle Time
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    tempS = String(descBot);
    tempS.toCharArray(charBuf, 50);    
    skinnyText(charBuf, 70, textLeft(charBuf, 280, 1, 6), 1, (cycleType == 'O') ? makeColor(100,100,100) : WHITE);            
  }  

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Error Function
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void error(int errorCode) { 
    if (currentError == errorCode) return;
    currentError = errorCode;
    String errorMessage = "";
    switch(errorCode) { 
      case 101 : errorMessage = "ERROR : LOW PRESSURE"; break;
      case 102 : errorMessage = "ERROR : TEMP TOO DIFFERENT"; break;
      case 103 : errorMessage = "ERROR : TEMP OVERAGE"; break;
      case 104 : errorMessage = "ERROR : MOTOR OFF"; break;      
      case 105 : errorMessage = "ERROR : BAD THERMISTOR"; break;            
      case 999 : errorMessage = "Test Mode Enabled"; break;                  
      default  : errorMessage = ""; break;                  
    }    
    char charBuf[30];    
    errorMessage.toCharArray(charBuf, 30);     
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Wifi Function
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    int backColor  = RED;
    int frontColor = WHITE;
    int deadColor  = makeColor(100,100,100);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Background Box
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    if (errorMessage == "") {
        backColor = WHITE;
        fontColor = BLACK;
    }
    makeRect(0,0,240,25,backColor);
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
    //|  Background Box
    //|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||
    makeText(charBuf, 9, 10, 1, frontColor);
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Text
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void makeClear() { 
    makeRect(0,0,320,240,BLACK);
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Text Left
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  int textLeft(char* textString, int centerX, int textSize, int kernSize) { 
    String newStr = textString;
    float len  = newStr.length();
    float width = textSize * kernSize;
    float l  = len / 2;  
    l  = l * width;
    return int(centerX - floor(l));
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Text
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void makeText(char* textString, int x, int y, int theSize, int color) { 
    y = 320 - y;
    Tft.drawString(textString, x, y, theSize, color);
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Text
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void skinnyText(char *string,unsigned int poX, unsigned int poY,unsigned int size,unsigned int fgcolor) {
    poY = 320 - poY;    
    int kern = 8;
    while(*string) {
        for(unsigned char i=0;i<8;i++) {
            Tft.drawChar(*string, poX, poY, size, fgcolor);
        }
        *string++;
        kern = 6;
        if(poY > 0) poY -= kern * size;
    }
  }
    
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Fill Rectangle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void makeRect(int x, int y, int width, int height, int color) { 
    y = 320 - y;
    Tft.fillRectangle(x, y, height, width, color);    
  }
  
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Fill Rectangle
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  void makeCircle(int x, int y, int radius, int color) { 
    y = 320 - y;
    Tft.fillCircle(x, y, radius, color);    
  }

//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|| 
//|  Create Color
//|=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-||

  unsigned int makeColor(unsigned char r,unsigned char g,unsigned char b){
      return (map((unsigned int)r, 0, 255, 0, 31)<<(BLUE_BITS+GREEN_BITS)) | (map((unsigned int)g, 0, 255, 0, 63)<<BLUE_BITS) | map((unsigned int)b, 0, 255, 0, 31);
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
  
