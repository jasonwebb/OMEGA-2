#include <Adafruit_NeoPixel.h>

/************************************************
 Pin definitions
*************************************************/
// Targeting system
#define  TARGETING_LEFT       PIN_C0
#define  TARGETING_RIGHT      PIN_C1
#define  TARGETING_LOCK       PIN_C2
#define  TARGETING_LED        PIN_B7
#define  TARGETING_NEOPIXELS  PIN_E1

// Torpedo system
#define  TORPEDOS_ARM          PIN_F2
#define  TORPEDOS_FIRE1        PIN_F3
#define  TORPEDOS_FIRE2        PIN_F4
#define  TORPEDOS_RELOAD1      PIN_F5
#define  TORPEDOS_RELOAD2      PIN_F6
#define  TORPEDOS_RELOAD1_LED  PIN_F7
#define  TORPEDOS_RELOAD2_LED  PIN_A0

// Laser system
#define  LASERS_ARM        PIN_A1
#define  LASERS_FIRE1      PIN_A2
#define  LASERS_FIRE2      PIN_A3
#define  LASERS_FIRE1_LED  PIN_A4
#define  LASERS_FIRE2_LED  PIN_A5
#define  LASERS_CHARGING1  PIN_A7
#define  LASERS_CHARGING2  PIN_A6

// Shield system
#define  SHIELDS_DAT         PIN_B2
#define  SHIELDS_CLR         PIN_B3
#define  SHIELDS_CLK         PIN_B1
#define  SHIELDS_EN          PIN_B0
#define  SHIELDS_FREQ_LATCH  PIN_F0
#define  SHIELDS_MOD_LATCH   PIN_F1
#define  SHIELDS_MOD_A       PIN_E6
#define  SHIELDS_MOD_B       PIN_E7
#define  SHIELDS_MOD_A_INT   6
#define  SHIELDS_MOD_B_INT   7
#define  SHIELDS_FREQ_A      PIN_D2
#define  SHIELDS_FREQ_B      PIN_D3
#define  SHIELDS_FREQ_A_INT  2
#define  SHIELDS_FREQ_B_INT  3
#define  SHIELDS_FREQ_LED_R  PIN_C4
#define  SHIELDS_FREQ_LED_G  PIN_C5
#define  SHIELDS_FREQ_LED_B  PIN_C6
#define  SHIELDS_MOD_LED_R   PIN_B6
#define  SHIELDS_MOD_LED_G   PIN_B5
#define  SHIELDS_MOD_LED_B   PIN_B4
#define  SHIELDS_NEOPIXELS   PIN_D4

// NeoPixels
Adafruit_NeoPixel targetingPixels = Adafruit_NeoPixel(12, TARGETING_NEOPIXELS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel shieldPixels = Adafruit_NeoPixel(16, SHIELDS_NEOPIXELS, NEO_GRB + NEO_KHZ800);

// General use variables
unsigned long currentMillis;

// Encoder variables
volatile int freqPosition = 0;
volatile int lastFreqEncoded = 0;
volatile int modPosition = 0;
volatile int lastModEncoded = 0;
int encoderSpeed = 1;

// Button state variables
boolean targetingLeft, targetingRight, targetingLock, targetingLED;
boolean torpedosArmed, torpedosFire1, torpedosFire2, torpedosReload1, torpedosReload2;
boolean lasersArmed, lasersFire1, lasersFire2, lasersReload1, lasersReload2;

// Targeting system variables
boolean animateTarget = true;
boolean targetAcquired = false;
int targetPosition = random(targetingPixels.numPixels());
int targetBrightness = 255;
int targetMoveTo = random(targetingPixels.numPixels());
int crosshairPosition = targetingPixels.numPixels()/2;
int crosshairWidth = 4;
byte targetColor[] = {255,0,0};
byte crosshairColor[] = {0,255,255};
byte overlayColor[] = {255,255,255};
int targetingUpdateInterval = 50;
unsigned long targetingLastUpdate = 0;

// Torpedo system variables
boolean torpedosLoaded1 = true;
boolean torpedosLoaded2 = true;
boolean torpedosReloadLED1 = true;
boolean torpedosReloadLED2 = true;
int torpedoUpdateInterval = 100;
unsigned long torpedoLastUpdate1 = 0;
unsigned long torpedoLastUpdate2 = 0;

// Laser system variables
boolean lasersCharged1 = true;
boolean lasersCharged2 = true;
boolean lasersChargingLED1 = true;
boolean lasersChargingLED2 = true;
int laserUpdateIntervalStart = 500;
int laserUpdateInterval1 = laserUpdateIntervalStart;
int laserUpdateInterval2 = laserUpdateIntervalStart;
unsigned long laserLastUpdate1 = 0;
unsigned long laserLastUpdate2 = 0;

// Shield system variables
int shieldUpdateInterval = 1000;
unsigned long shieldLastUpdate = 0;

byte shieldTopLEDs = 5;
byte shieldLeftLEDs = 3;
byte shieldBottomLEDs = 5;
byte shieldRightLEDs = 3;
byte shieldTopLEDPins[] = {0,1,2,3,4};
byte shieldLeftLEDPins[] = {5,6,7};
byte shieldBottomLEDPins[] = {8,9,10,11,12};
byte shieldRightLEDPins[] = {15,14,13};

byte shieldTopHealth = 30;
byte shieldLeftHealth = 10;
byte shieldBottomHealth = 40;
byte shieldRightHealth = 80;

byte shieldHealthGood = 80;
byte shieldHealthBad = 40;

byte shieldGreen[] = {0,255,0};
byte shieldYellow[] = {255,255,0};
byte shieldRed[] = {255,0,0};

byte shieldFreqBaseColor[] = {0,255,255};
byte shieldModBaseColor[] = {0,255,255};
byte shieldFreqCurrentColor[] = {0,255,255};
byte shieldModCurrentColor[] = {0,255,255};

// Serial variables
String inputString;
int commandPayloadSize = 4;
String commandType, commandID;
int commandValue;

void setup() {
  // Set up serial connection
  Serial.begin(115200);
  
  // Set up targeting system =====================================
  pinMode(TARGETING_LEFT, INPUT);
  pinMode(TARGETING_RIGHT, INPUT);
  pinMode(TARGETING_LOCK, INPUT);
  pinMode(TARGETING_LED, OUTPUT);
  
  digitalWrite(TARGETING_LEFT, HIGH);
  digitalWrite(TARGETING_RIGHT, HIGH);
  digitalWrite(TARGETING_LOCK, HIGH);
  
  targetingPixels.begin();
  targetingPixels.show();
  
  // Set up torpedo system =======================================
  pinMode(TORPEDOS_ARM, INPUT);
  pinMode(TORPEDOS_FIRE1, INPUT);
  pinMode(TORPEDOS_FIRE2, INPUT);
  pinMode(TORPEDOS_RELOAD1, INPUT);
  pinMode(TORPEDOS_RELOAD2, INPUT);
  pinMode(TORPEDOS_RELOAD1_LED, OUTPUT);
  pinMode(TORPEDOS_RELOAD2_LED, OUTPUT);
  
  digitalWrite(TORPEDOS_FIRE1, HIGH);
  digitalWrite(TORPEDOS_FIRE2, HIGH);
  digitalWrite(TORPEDOS_RELOAD1, HIGH);
  digitalWrite(TORPEDOS_RELOAD2, HIGH);
  digitalWrite(TORPEDOS_RELOAD1_LED, LOW);
  digitalWrite(TORPEDOS_RELOAD2_LED, LOW);
  
  // Set up laser system ===========================================
  pinMode(LASERS_ARM, INPUT);
  pinMode(LASERS_FIRE1, INPUT);
  pinMode(LASERS_FIRE2, INPUT);
  pinMode(LASERS_FIRE1_LED, OUTPUT);
  pinMode(LASERS_FIRE2_LED, OUTPUT);
  pinMode(LASERS_CHARGING1, OUTPUT);
  pinMode(LASERS_CHARGING2, OUTPUT);
  
  digitalWrite(LASERS_FIRE1, HIGH);
  digitalWrite(LASERS_FIRE2, HIGH);
  digitalWrite(LASERS_FIRE1_LED, LOW);
  digitalWrite(LASERS_FIRE2_LED, LOW);
  digitalWrite(LASERS_CHARGING1, LOW);
  digitalWrite(LASERS_CHARGING2, LOW);
  
  // Set up shield system ===========================================
  shieldPixels.begin();
  shieldPixels.show();
  
  // Circular LED arrays ------------------------------------
  pinMode(SHIELDS_EN, OUTPUT);
  pinMode(SHIELDS_CLK, OUTPUT);
  pinMode(SHIELDS_CLR, OUTPUT);
  pinMode(SHIELDS_DAT, OUTPUT);
  
  digitalWrite(SHIELDS_EN, LOW);
  digitalWrite(SHIELDS_CLK, LOW);
  digitalWrite(SHIELDS_CLR, HIGH);
  digitalWrite(SHIELDS_DAT, LOW);
  
    // FREQUENCY ---------------------------------------------
    // Encoder
    pinMode(SHIELDS_FREQ_A, INPUT);
    pinMode(SHIELDS_FREQ_B, INPUT);

    digitalWrite(SHIELDS_FREQ_A, HIGH);
    digitalWrite(SHIELDS_FREQ_B, HIGH);

    attachInterrupt(SHIELDS_FREQ_A_INT, updateEncoders, CHANGE);
    attachInterrupt(SHIELDS_FREQ_B_INT, updateEncoders, CHANGE);
    
    // RGB LED
    pinMode(SHIELDS_FREQ_LED_R, OUTPUT);
    pinMode(SHIELDS_FREQ_LED_G, OUTPUT);
    pinMode(SHIELDS_FREQ_LED_B, OUTPUT);

    analogWrite(SHIELDS_FREQ_LED_R, 255);
    analogWrite(SHIELDS_FREQ_LED_G, 255);
    analogWrite(SHIELDS_FREQ_LED_B, 255);
    
    // LED ring
    pinMode(SHIELDS_FREQ_LATCH, OUTPUT);
    digitalWrite(SHIELDS_FREQ_LATCH, LOW); 
  
    // MODULATION --------------------------------------------
    // Encoder
    pinMode(SHIELDS_MOD_A, INPUT);
    pinMode(SHIELDS_MOD_B, INPUT);

    digitalWrite(SHIELDS_MOD_A, HIGH);
    digitalWrite(SHIELDS_MOD_B, HIGH);

    attachInterrupt(SHIELDS_MOD_A_INT, updateEncoders, CHANGE);
    attachInterrupt(SHIELDS_MOD_B_INT, updateEncoders, CHANGE);  

    // RGB LED
    pinMode(SHIELDS_MOD_LED_R, OUTPUT);
    pinMode(SHIELDS_MOD_LED_G, OUTPUT);
    pinMode(SHIELDS_MOD_LED_B, OUTPUT);
  
    analogWrite(SHIELDS_MOD_LED_R, 255);
    analogWrite(SHIELDS_MOD_LED_G, 255);
    analogWrite(SHIELDS_MOD_LED_B, 255);

    // LED ring
    pinMode(SHIELDS_MOD_LATCH, OUTPUT);
    digitalWrite(SHIELDS_MOD_LATCH, LOW);
    
  // Initialize circular LED arrays to off
  updateEncoders();
}

void loop() {
  // Update current millis counter
  currentMillis = millis();
  
  // Reset all subsystem counters when millis() overflows 
  if(currentMillis == 0) {
    targetingLastUpdate = 0;
    torpedoLastUpdate1 = 0;
    torpedoLastUpdate2 = 0;
    laserLastUpdate1 = 0;
    laserLastUpdate2 = 0;
    shieldLastUpdate = 0;
  }
  
  // Update all systems
  updateTargetingSystem();
  updateTorpedoSystem();
  updateLaserSystem();
  updateShieldSystem();
  
  // Process any serial information
  processSerial();
}

/***************************************************************************
 Update targeting system
****************************************************************************/
void updateTargetingSystem() {
  // Get button states
  targetingLeft = digitalRead(TARGETING_LEFT);
  targetingRight = digitalRead(TARGETING_RIGHT);
  targetingLock = digitalRead(TARGETING_LOCK);
  
  // Update at right time, only when target is not acquired
  if(currentMillis - targetingLastUpdate >= targetingUpdateInterval && !targetAcquired) {
    targetingLastUpdate = currentMillis;
    
    // Clear all pixels
    for(int i=0; i<targetingPixels.numPixels(); i++)
      targetingPixels.setPixelColor(i, 0, 0, 0);

    // Move target
    if(animateTarget) {
      if(targetPosition != targetMoveTo) {
        if(targetPosition - targetMoveTo > 0)
          targetPosition--;
        else
          targetPosition++; 
      } else {
        targetMoveTo = random(targetingPixels.numPixels());
      }
    }
 
    // Move crosshairs
    if(!targetingLeft && ((crosshairPosition + crosshairWidth/2) < targetingPixels.numPixels()-1))
      crosshairPosition++;
    
    if(!targetingRight && ((crosshairPosition - crosshairWidth/2) > 0))
      crosshairPosition--;

    // Display target
    targetingPixels.setPixelColor(targetPosition, targetColor[0], targetColor[1], targetColor[2]);

    // Display crosshairs (checking for overlay with target)
    if((crosshairPosition - crosshairWidth/2) == targetPosition)
      targetingPixels.setPixelColor(crosshairPosition - crosshairWidth/2, overlayColor[0], overlayColor[1], overlayColor[2]);
    else
      targetingPixels.setPixelColor(crosshairPosition - crosshairWidth/2, crosshairColor[0], crosshairColor[1], crosshairColor[2]);

    if((crosshairPosition + crosshairWidth/2) == targetPosition)
      targetingPixels.setPixelColor(crosshairPosition + crosshairWidth/2, overlayColor[0], overlayColor[1], overlayColor[2]);
    else
      targetingPixels.setPixelColor(crosshairPosition + crosshairWidth/2, crosshairColor[0], crosshairColor[1], crosshairColor[2]);

    // Push to NeoPixels
    targetingPixels.show();
    
    // Acquire target when it is between crosshairs and lock button pressed
    if(!targetingLock && !targetAcquired)
      if((targetPosition < crosshairPosition + crosshairWidth/2) && (targetPosition > crosshairPosition - crosshairWidth/2))
        targetAcquired = true;

    // Display target acquired notification on LED
    if(targetAcquired)
      digitalWrite(TARGETING_LED, HIGH);
    else
      digitalWrite(TARGETING_LED, LOW);
  }
    
    /**** Debug ****************************/
//    Serial.print(targetingLeft);
//    Serial.print("\t");
//    Serial.print(targetingRight);
//    Serial.print("\t");
//    Serial.println(targetingLock);
    /***************************************/
}

  int getTargetPosition()     {  return targetPosition;     }
  int getCrosshairPosition()  {  return crosshairPosition;  }
  boolean getTargetAcquired() {  return targetAcquired;     }
  boolean getAnimateTarget()  {  return animateTarget;      }
  int getCrosshairWidth()     {  return crosshairWidth;     }
  
  void setTargetPosition(int targetPosition_)       {  targetPosition = targetPosition_;        }
  void setCrosshairPosition(int crosshairPosition_) {  crosshairPosition = crosshairPosition_;  }
  void setTargetAcquired(boolean targetAcquired_)   {  targetAcquired = targetAcquired_;        }
  void setAnimateTarget(boolean animateTarget_)     {  animateTarget = animateTarget_;          }
  void setCrosshairWidth(int crosshairWidth_)       {  crosshairWidth = crosshairWidth_;        }

/***************************************************************************
 Update torpedo system
****************************************************************************/
void updateTorpedoSystem() {
  // Get button states
  torpedosArmed = digitalRead(TORPEDOS_ARM);
  torpedosFire1 = digitalRead(TORPEDOS_FIRE1);
  torpedosFire2 = digitalRead(TORPEDOS_FIRE2);
  torpedosReload1 = digitalRead(TORPEDOS_RELOAD1);
  torpedosReload2 = digitalRead(TORPEDOS_RELOAD2);
  
  if(torpedosArmed) {
    // Torpedo 1 ---------------------------------------------------
    // Fire
    if(!torpedosFire1 && torpedosLoaded1)
      fireTorpedo(1);

    // Reload
    if(!torpedosLoaded1) {
      // Toggle LED at correct interval
      if(currentMillis - torpedoLastUpdate1 >= torpedoUpdateInterval) {
        if(torpedosReloadLED1) {
          digitalWrite(TORPEDOS_RELOAD1_LED, HIGH);
          torpedosReloadLED1 = false;
        } else {
          digitalWrite(TORPEDOS_RELOAD1_LED, LOW);
          torpedosReloadLED1 = true;
        }
        
        torpedoLastUpdate1 = currentMillis;
      }
      
      // Reload when reload button pressed
      if(!torpedosReload1)
        reloadTorpedo(1);
    } else {
      digitalWrite(TORPEDOS_RELOAD1_LED, LOW);
    }    
    
    // Torpedo 2 -------------------------------------------------
    // Fire
    if(!torpedosFire2 && torpedosLoaded2)
      fireTorpedo(2);
    
    // Reload
    if(!torpedosLoaded2) {
      // Toggle LED at correct interval
      if(currentMillis - torpedoLastUpdate2 >= torpedoUpdateInterval) {
        if(torpedosReloadLED2) {
          digitalWrite(TORPEDOS_RELOAD2_LED, HIGH);
          torpedosReloadLED2 = false;
        } else {
          digitalWrite(TORPEDOS_RELOAD2_LED, LOW);
          torpedosReloadLED2 = true;
        }
        
        torpedoLastUpdate2 = currentMillis;
      }
      
      if(!torpedosReload2)
        reloadTorpedo(2);
    } else {
      digitalWrite(TORPEDOS_RELOAD2_LED, LOW);
    }    
  } else {
    // Disable everything when not armed
    digitalWrite(TORPEDOS_RELOAD1_LED, LOW);
    digitalWrite(TORPEDOS_RELOAD2_LED, LOW);    
  }
  
    /**** Debug *********************************/
//    Serial.print(torpedosArmed);
//    Serial.print("\t");
//    Serial.print(torpedosFire1);
//    Serial.print("\t");
//    Serial.print(torpedosFire2);
//    Serial.print("\t");
//    Serial.print(torpedosReload1);
//    Serial.print("\t");
//    Serial.println(torpedosReload2);
    /********************************************/  
}

void fireTorpedo(byte torpedo) {
  if(torpedo == 1) {
//    Serial.println("Fired torpedo 1");
    torpedosLoaded1 = false;
  } else if(torpedo == 2) {
//    Serial.println("Fired torpedo 2");
    torpedosLoaded2 = false;
  }
}

void reloadTorpedo(byte torpedo) {
  if(torpedo == 1) {
//    Serial.println("Reloaded torpedo 1");
    torpedosLoaded1 = true;
  } else if(torpedo == 2) {
//    Serial.println("Reloaded torpedo 2");
    torpedosLoaded2 = true;
  }
}

  boolean getTorpedosArmed()   {  return torpedosArmed;    }
  boolean getTorpedosLoaded1() {  return torpedosLoaded1;  }
  boolean getTorpedosLoaded2() {  return torpedosLoaded2;  }
  
  void setTorpedosArmed(boolean torpedosArmed_)     {  torpedosArmed = torpedosArmed_;      }
  void setTorpedosLoaded1(boolean torpedosLoaded1_) {  torpedosLoaded1 = torpedosLoaded1_;  }
  void setTorpedosLoaded2(boolean torpedosLoaded2_) {  torpedosLoaded2 = torpedosLoaded2_;  }

/***************************************************************************
 Update laser system
****************************************************************************/
void updateLaserSystem() {
  // Laser system
  lasersArmed = digitalRead(LASERS_ARM);
  lasersFire1 = digitalRead(LASERS_FIRE1);
  lasersFire2 = digitalRead(LASERS_FIRE2);
      
  if(lasersArmed) {
    // Laser 1 ---------------------------------------------------
    // Fire
    if(!lasersFire1 && lasersCharged1)
      fireLaser(1);

    // Charge
    if(!lasersCharged1) {
      // Turn off button LED
      digitalWrite(LASERS_FIRE1_LED, LOW);
      
      // Toggle charging LED
      if(currentMillis - laserLastUpdate1 >= laserUpdateInterval1) {
        if(lasersChargingLED1) {
          digitalWrite(LASERS_CHARGING1, HIGH);
          lasersChargingLED1 = false;
        } else {
          digitalWrite(LASERS_CHARGING1, LOW);
          lasersChargingLED1 = true;
        }
        
        laserLastUpdate1 = currentMillis;
        
        // Speed up charging LED toggle frequency
        if(laserUpdateInterval1 > 200) {
          laserUpdateInterval1 -= 75;
        } else if(laserUpdateInterval1 <= 200 && laserUpdateInterval1 > 20) {
          laserUpdateInterval1 -= 10;
        } else if(laserUpdateInterval1 <= 20) {
          digitalWrite(LASERS_CHARGING1, LOW);
          laserUpdateInterval1 = laserUpdateIntervalStart;
          lasersCharged1 = true;
        }
      }
    } else {
      // Keep LED on until button is pressed
      digitalWrite(LASERS_FIRE1_LED, HIGH);
    }
    
    // Laser 2 ---------------------------------------------------
    // Fire
    if(!lasersFire2 && lasersCharged2)
      fireLaser(2);

    // Charge
    if(!lasersCharged2) {
      // Turn off button LED
      digitalWrite(LASERS_FIRE2_LED, LOW);
      
      // Toggle charging LED
      if(currentMillis - laserLastUpdate2 >= laserUpdateInterval2) {
        if(lasersChargingLED2) {
          digitalWrite(LASERS_CHARGING2, HIGH);
          lasersChargingLED2 = false;
        } else {
          digitalWrite(LASERS_CHARGING2, LOW);
          lasersChargingLED2 = true;
        }
        
        laserLastUpdate2 = currentMillis;
        
        // Speed up charging LED toggle frequency
        if(laserUpdateInterval2 > 200) {
          laserUpdateInterval2 -= 75;
        } else if(laserUpdateInterval2 <= 200 && laserUpdateInterval2 > 20) {
          laserUpdateInterval2 -= 10;
        } else if(laserUpdateInterval2 <= 20) {
          digitalWrite(LASERS_CHARGING2, LOW);
          laserUpdateInterval2 = laserUpdateIntervalStart;
          lasersCharged2 = true;
        }
      }
    } else {
      // Keep LED on until button is pressed
      digitalWrite(LASERS_FIRE2_LED, HIGH);
    }    
  } else {
    // When not armed, turn off all LEDs and reset charging pattern
    digitalWrite(LASERS_FIRE1_LED, LOW);
    digitalWrite(LASERS_FIRE2_LED, LOW);
    digitalWrite(LASERS_CHARGING1, LOW);
    digitalWrite(LASERS_CHARGING2, LOW);

    // Reset charging LED animations    
    laserUpdateInterval1 = laserUpdateIntervalStart;
    laserUpdateInterval2 = laserUpdateIntervalStart;
    lasersChargingLED1 = true;
    lasersChargingLED2 = true;
  }
  
    /**** Debug ********************************/
//    Serial.print(lasersArmed);
//    Serial.print("\t");
//    Serial.print(lasersFire1);
//    Serial.print("\t");
//    Serial.println(lasersFire2);
    /*******************************************/  
}

void fireLaser(byte laser) {
  if(laser == 1) {
//    Serial.println("Fired laser 1");
    lasersCharged1 = false;
  } else if(laser == 2) {
//    Serial.println("Fired laser 2");
    lasersCharged2 = false;
  }
}

  boolean getLasersArmed()   {  return lasersArmed;     }
  boolean getLaserCharged1() {  return lasersCharged1;  }
  boolean getLaserCharged2() {  return lasersCharged2;  }
  
  void setLasersArmed(boolean lasersArmed_)       {  lasersArmed = lasersArmed_;        }
  void setLasersCharged1(boolean lasersCharged1_) {  lasersCharged1 = lasersCharged1_;  }
  void setLasersCharged2(boolean lasersCharged2_) {  lasersCharged2 = lasersCharged2_;  }

/***************************************************************************
 Update shield system
****************************************************************************/
void updateShieldSystem() {
  // Update LED colors for each shield group based on health
  if(currentMillis - shieldLastUpdate >= shieldUpdateInterval) {
    // Top
    if(shieldTopHealth >= shieldHealthGood) {
      for(int i=0; i<shieldTopLEDs; i++)
        shieldPixels.setPixelColor(shieldTopLEDPins[i], shieldGreen[0], shieldGreen[1], shieldGreen[2]);
    } else if(shieldTopHealth < shieldHealthGood && shieldTopHealth >= shieldHealthBad) {
      for(int i=0; i<shieldTopLEDs; i++)
        shieldPixels.setPixelColor(shieldTopLEDPins[i], shieldYellow[0], shieldYellow[1], shieldYellow[2]);
    } else if(shieldTopHealth < shieldHealthBad) {
      for(int i=0; i<shieldTopLEDs; i++)
        shieldPixels.setPixelColor(shieldTopLEDPins[i], shieldRed[0], shieldRed[1], shieldRed[2]);      
    }
    
    // Left
    if(shieldLeftHealth >= shieldHealthGood) {
      for(int i=0; i<shieldLeftLEDs; i++)
        shieldPixels.setPixelColor(shieldLeftLEDPins[i], shieldGreen[0], shieldGreen[1], shieldGreen[2]);
    } else if(shieldLeftHealth < shieldHealthGood && shieldLeftHealth >= shieldHealthBad) {
      for(int i=0; i<shieldLeftLEDs; i++)
        shieldPixels.setPixelColor(shieldLeftLEDPins[i], shieldYellow[0], shieldYellow[1], shieldYellow[2]);
    } else if(shieldLeftHealth < shieldHealthBad) {
      for(int i=0; i<shieldLeftLEDs; i++)
        shieldPixels.setPixelColor(shieldLeftLEDPins[i], shieldRed[0], shieldRed[1], shieldRed[2]);      
    }
    
    // Bottom
    if(shieldBottomHealth >= shieldHealthGood) {
      for(int i=0; i<shieldBottomLEDs; i++)
        shieldPixels.setPixelColor(shieldBottomLEDPins[i], shieldGreen[0], shieldGreen[1], shieldGreen[2]);
    } else if(shieldBottomHealth < shieldHealthGood && shieldBottomHealth >= shieldHealthBad) {
      for(int i=0; i<shieldBottomLEDs; i++)
        shieldPixels.setPixelColor(shieldBottomLEDPins[i], shieldYellow[0], shieldYellow[1], shieldYellow[2]);
    } else if(shieldBottomHealth < shieldHealthBad) {
      for(int i=0; i<shieldBottomLEDs; i++)
        shieldPixels.setPixelColor(shieldBottomLEDPins[i], shieldRed[0], shieldRed[1], shieldRed[2]);      
    }
    
    // Right
    if(shieldRightHealth >= shieldHealthGood) {
      for(int i=0; i<shieldRightLEDs; i++)
        shieldPixels.setPixelColor(shieldRightLEDPins[i], shieldGreen[0], shieldGreen[1], shieldGreen[2]);
    } else if(shieldRightHealth < shieldHealthGood && shieldRightHealth >= shieldHealthBad) {
      for(int i=0; i<shieldRightLEDs; i++)
        shieldPixels.setPixelColor(shieldRightLEDPins[i], shieldYellow[0], shieldYellow[1], shieldYellow[2]);
    } else if(shieldRightHealth < shieldHealthBad) {
      for(int i=0; i<shieldRightLEDs; i++)
        shieldPixels.setPixelColor(shieldRightLEDPins[i], shieldRed[0], shieldRed[1], shieldRed[2]);      
    }
   
    // Push to NeoPixels
    shieldPixels.show();

    // Frequency knob
    analogWrite(SHIELDS_FREQ_LED_R, 255-shieldFreqCurrentColor[0]);
    analogWrite(SHIELDS_FREQ_LED_G, 255-shieldFreqCurrentColor[1]);
    analogWrite(SHIELDS_FREQ_LED_B, 255-shieldFreqCurrentColor[2]);
  
    // Modulation knob
    analogWrite(SHIELDS_MOD_LED_R, 255-shieldModCurrentColor[0]);
    analogWrite(SHIELDS_MOD_LED_G, 255-shieldModCurrentColor[1]);
    analogWrite(SHIELDS_MOD_LED_B, 255-shieldModCurrentColor[2]);
  }
}

  int getShieldHealthTop()    {  return shieldTopHealth;      }
  int getShieldHealthRight()  {  return  shieldRightHealth;   }
  int getShieldHealthBottom() {  return  shieldBottomHealth;  }
  int getShieldHealthLeft()   {  return shieldLeftHealth;     }

  int getFrequency()  {  return freqPosition;  }
  int getModulation() {  return modPosition;   }
  
  void setShieldHealth(byte top, byte right, byte bottom, byte left) {
    setShieldHealthTop(top);
    setShieldHealthRight(right);
    setShieldHealthBottom(bottom);
    setShieldHealthLeft(left);
  }
  
  void setShieldHealthTop(byte health)    {  shieldTopHealth = health;     }
  void setShieldHealthRight(byte health)  {  shieldRightHealth = health;   }
  void setShieldHealthBottom(byte health) {  shieldBottomHealth = health;  }
  void setShieldHealthLeft(byte health)   {  shieldLeftHealth = health;    }
  
  void setFrequency(int frequency)   {  freqPosition = frequency;  updateLEDRings();  }
  void setModulation(int modulation) {  modPosition = modulation;  updateLEDRings();  }
  
  void setFrequencyColor(byte r, byte g, byte b) {
    shieldFreqBaseColor[0] = r;
    shieldFreqBaseColor[1] = g;
    shieldFreqBaseColor[2] = b;
  }
  
  void setModulationColor(byte r, byte g, byte b) {
    shieldModBaseColor[0] = r;
    shieldModBaseColor[1] = g;
    shieldModBaseColor[2] = b;
  }  

/***************************************************************************
 Retreieve and process new rotary encoder values
 - called automatically on change via interrupts
****************************************************************************/
void updateEncoders() {  
  // Frequency ---------------------------------------------------------
  int freqMSB = digitalRead(SHIELDS_FREQ_A);
  int freqLSB = digitalRead(SHIELDS_FREQ_B);
 
  int freqEncoded = (freqMSB << 1) | freqLSB;
  int freqSum = (lastFreqEncoded << 2) | freqEncoded;
  
  if(freqSum == 0b1101 || freqSum == 0b0100 || freqSum == 0b0010 || freqSum == 0b1011) freqPosition += encoderSpeed;
  if(freqSum == 0b1110 || freqSum == 0b0111 || freqSum == 0b0001 || freqSum == 0b1000) freqPosition -= encoderSpeed;

  if(freqPosition > 127)  freqPosition = 127;
  if(freqPosition < 0)    freqPosition = 0;

  lastFreqEncoded = freqEncoded;
  
  // Modulation --------------------------------------------------------
  int modMSB = digitalRead(SHIELDS_MOD_A);
  int modLSB = digitalRead(SHIELDS_MOD_B);
 
  int modEncoded = (modMSB << 1) | modLSB;
  int modSum = (lastModEncoded << 2) | modEncoded;
  
  if(modSum == 0b1101 || modSum == 0b0100 || modSum == 0b0010 || modSum == 0b1011) modPosition += encoderSpeed;
  if(modSum == 0b1110 || modSum == 0b0111 || modSum == 0b0001 || modSum == 0b1000) modPosition -= encoderSpeed;

  if(modPosition > 127)  modPosition = 127;
  if(modPosition < 0)    modPosition = 0;

  lastModEncoded = modEncoded;
  
  // Push the new values to the LED rings ------------------------------
  updateLEDRings();
}

/*******************************************************************************
 Push new values to the circular LED rings (freq. and mod.) shift registers
 - called automtically through updateEncoders(), which is called 
   automatically via interrupts
********************************************************************************/
void updateLEDRings() {
  // Frequency --------------------------------------------------
  unsigned int ledShift = 0;
  unsigned int ledOutput = 0;
  
  if (freqPosition != 0) {
    ledShift = freqPosition & (0xFF >> (1));
    ledShift /= 0x10>>(1);

    for (int i=ledShift; i>=0; i--)
      ledOutput |= 1<<i;
  }
  
  digitalWrite(SHIELDS_FREQ_LATCH, LOW);  // first send latch low
  shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
  digitalWrite(SHIELDS_FREQ_LATCH, HIGH);  // send latch high to indicate data is done sending 

  // Modulation ----------------------------------------------------  
  ledShift = 0;
  ledOutput = 0;
  
  if(modPosition != 0) {
    ledShift = modPosition & (0xFF >> (1));
    ledShift /= 0x10>>(1);

    for (int i=ledShift; i>=0; i--)
      ledOutput |= 1<<i;
  }
  
  digitalWrite(SHIELDS_MOD_LATCH, LOW);  // first send latch low
  shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
  digitalWrite(SHIELDS_MOD_LATCH, HIGH);  // send latch high to indicate data is done sending   
}

/***********************************************************************************
 Modified shiftOut function that shifts 16 bits instead of the normal 8
 - used to push values to circular LED rings (freq. and mod.), which 
   each contain two shift registers.   
************************************************************************************/
void shiftOut16(uint16_t data) {
  byte datamsb, datalsb;
  
  // Isolate the MSB and LSB
  datamsb = (data&0xFF00)>>8;  // mask out the MSB and shift it right 8 bits
  datalsb = data & 0xFF;  // Mask out the LSB
  
  // First shift out the MSB, MSB first, then LSB
  shiftOut(SHIELDS_DAT, SHIELDS_CLK, MSBFIRST, datamsb);
  shiftOut(SHIELDS_DAT, SHIELDS_CLK, MSBFIRST, datalsb);
}

// Map float value from one range to another
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*************************************************************************
 Process all incoming serial data and execute commands given
**************************************************************************/
void processSerial() {
  if(Serial.available() > 0) {
    // Get entire string
    inputString = Serial.readStringUntil('\n');

    // Verify string length
    if( inputString.length() >= commandPayloadSize ) {
        commandType = inputString[0];            // Get type [G|S]
        commandID = inputString.substring(1,4);  // Get ID
        
        // Getters
        if(commandType == 'G') {
          if(commandID == "TAP")
            Serial.println( getTargetPosition() );
          else if(commandID == "ATD")
            Serial.println( getAnimateTarget() );            
          else if(commandID == "CHP")
            Serial.println( getCrosshairPosition() );
          else if(commandID == "CHW")
            Serial.println( getCrosshairWidth() );
          else if(commandID == "TAA")
            Serial.println( getTargetAcquired() );
            
          else if(commandID == "TOA")
            Serial.println( getTorpedosArmed() );
          else if(commandID == "TL1")
            Serial.println( getTorpedosLoaded1() );
          else if(commandID == "TL2")
            Serial.println( getTorpedosLoaded2() );
            
          else if(commandID == "LAA")
            Serial.println( getLasersArmed() );
          else if(commandID == "LC1")
            Serial.println( getLaserCharged1() );
          else if(commandID == "LC2")
            Serial.println( getLaserCharged2() );
            
          else if(commandID == "SHT")
            Serial.println( getShieldHealthTop() );
          else if(commandID == "SHR")
            Serial.println( getShieldHealthRight() );
          else if(commandID == "SHB")
            Serial.println( getShieldHealthBottom() );
          else if(commandID == "SHL")
            Serial.println( getShieldHealthLeft() );
            
          else if(commandID == "SFR")
            Serial.println( getFrequency() );
          else if(commandID == "SMO")
            Serial.println( getModulation() );
            
          else
            Serial.println("Command ID not recognized");
            
        // Setters
        } else if(commandType == 'S') {
          // Get all numbers at end of command and convert to int
          commandValue = inputString.substring(4).toInt();
          
          if(commandID == "TAP")
            setTargetPosition( commandValue );
          else if(commandID == "ATD")
            setAnimateTarget( commandValue );
          else if(commandID == "CHP")
            setCrosshairPosition( commandValue );
          else if(commandID == "CHW")
            setCrosshairWidth( commandValue );
          else if(commandID == "TAA")
            setTargetAcquired( commandValue );
        
          else if(commandID == "TOA")
            setTorpedosArmed( commandValue );
          else if(commandID == "TL1")
            setTorpedosLoaded1( commandValue );
          else if(commandID == "TL2")
            setTorpedosLoaded2( commandValue );
            
          else if(commandID == "LAA")
            setLasersArmed( commandValue );
          else if(commandID == "LC1")
            setLasersCharged1( commandValue );
          else if(commandID == "LC2")
            setLasersCharged2( commandValue );
            
          else if(commandID == "SHT")
            setShieldHealthTop( commandValue );
          else if(commandID == "SHR")
            setShieldHealthRight( commandValue );
          else if(commandID == "SHB")
            setShieldHealthBottom( commandValue );
          else if(commandID == "SHL")
            setShieldHealthLeft( commandValue );
            
          else if(commandID == "SFR")
            setFrequency( commandValue );
          else if(commandID == "SMO")
            setModulation( commandValue );
            
          else
            Serial.println("Command ID not recognized");
          
        // Unrecognized command
        } else {
          Serial.print("Command type not recognized: ");
          Serial.println(commandType);          
        }
        
    // Command not long enough
    } else {
      Serial.print("Command not long enough: ");
      Serial.println(inputString);
    }
  }
}
