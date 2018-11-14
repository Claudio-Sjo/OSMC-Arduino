/* Program OSMC Watchdog
 * This program takes care of poweron and watchdog of OSMC on RPi
 *   It requires an IR receiver such as TSOP1838
 *   two LEDs being used for Power On indication and IR activity
 *   One program running as rc.local in OSMC generating a 1Hz square wave
 *   Bit Used  
 *   Input  : IR_INPUT
 *            RPI_WATCHDOG
 *   Output : RESET
 *            PWR_LED
 *            IR_BLINK
 *            Required Libraries : https://github.com/cyborg5/IRLib2
 *                                 http://wiringpi.com/
 *            Connections : Arduino Pro Micro    Raspberry Pi   TSOP1838
 *                          ------------------+----------------+------
 *                                  2         |  BCM 18 (12)   | Out
 *                                  3         |  BCM 17 (11)   |
 *                                  4         |      RUN       |
 *                                 Vcc        |  5v Power (2)  | 
 *                                            | 3v3 Power (1)  | Vs
 *                                 GND        |    Ground (6)  |       
 *                                            |    Ground (9)  | GND
 *                          ------------------+----------------+------                  
 */

#include "IRLibAll.h"

#define  IR_INPUT     2
#define  RPI_WATCHDOG 3
#define  RPI_RESET    4
#define  PWR_LED      5
#define  IR_BLINK     6

// State Machine
#define  RESET        0
#define  BOOT         1
#define  BOOT_WL      2
#define  ON           3
#define  STDBY        4

// IR checked values
#define  PWRON        0x10ef18e7

//#define TEST
//#define WDTEST
 
//Create a receiver object to listen on pin 2
IRrecvPCI myReceiver(IR_INPUT);
 
//Create a decoder object 
IRdecode myDecoder;   

unsigned char SysStatus;
unsigned char ForcedRpiRst;

void setup() {
  // put your setup code here, to run once:
  
//  Serial.begin(9600);
//  delay(2000); while (!Serial); //delay for Leonardo
  myReceiver.enableIRIn(); // Start the receiver
//  Serial.println(F("Ready to receive IR signals"));

  pinMode(RPI_WATCHDOG,INPUT);
  digitalWrite(RPI_RESET,LOW);
  pinMode(RPI_RESET,OUTPUT);
  digitalWrite(RPI_RESET,LOW);

#ifdef HASLEDS
  pinMode(PWR_LED, OUTPUT);
  pinMode(IR_BLINK, OUTPUT);
  digitalWrite(PWR_LED, LOW);
  digitalWrite(IR_BLINK, LOW);
#endif
  
  SysStatus = RESET; // must be RESET
  ForcedRpiRst = 0;
  
}

#ifdef HASLEDS
void PowerLedOn(bool on)
{
  if (on == true)
     digitalWrite(PWR_LED, HIGH);
  else
     digitalWrite(PWR_LED, LOW);   
}
#endif

#ifdef TEST
void irLedPulse(bool reset)
{
  static unsigned long StartTime;

  if (reset == true)
  {
     StartTime = millis();
#ifdef HASLEDS
     digitalWrite(IR_BLINK, HIGH);
#endif          
     RXLED1;
  }
  else
     if ((millis() - StartTime) > 100)
     {
#ifdef HASLEDS
       digitalWrite(IR_BLINK, LOW);
#endif       
       RXLED0;    
     }
}
#endif

void restartRPi()
{
//  Serial.println(F("restartRPi()"));
  digitalWrite(RPI_RESET,LOW);
  pinMode(RPI_RESET,OUTPUT);
  digitalWrite(RPI_RESET,LOW);
  delay(500);
  pinMode(RPI_RESET,INPUT);
}

bool IRPwrButton()
{
  if (myReceiver.getResults()) {
      myDecoder.decode();         
      // myDecoder.dumpResults(false); 
      myReceiver.enableIRIn();   
#ifdef TEST
      irLedPulse(true);
#endif
  
     if (myDecoder.value == PWRON)
         return true;
  }  
  
#ifdef TEST
      irLedPulse(false);
#endif    
  return false;  
          
}

// bool timeOutWD : if tmo = 0 reset else if timeout returns true
bool timeOutWD(uint32_t tmo)
{
  static unsigned long StartTime;
  static bool          wdUp;
  
  if (tmo == 0)
  {
    StartTime = millis(); // Start supervision
    wdUp = ((digitalRead(RPI_WATCHDOG) == HIGH) ? true : false);
  }
  else
  {
    if (wdUp != (digitalRead(RPI_WATCHDOG) == HIGH))
    {
      wdUp = ((wdUp == true)? false : true);
      StartTime = millis(); // Start supervision   
    }
    else 
    {      
      if ((millis() - StartTime) > tmo)
        return true;
    }
  }  
#ifdef WDTEST
       if (wdUp == true) 
       TXLED1;
       else
       TXLED0;
#endif            
  return false;
}

// if rst = true reset, otherwise returns the toggle
bool toggleOutWD(bool rst)
{
  static bool          wdUp;
  
  if (rst == true)
  {
    wdUp = ((digitalRead(RPI_WATCHDOG) == HIGH) ? true : false);
  }
  else
  {
    if (wdUp != (digitalRead(RPI_WATCHDOG) == HIGH))
    return true;
  }
  return false;
}


void loop() {
  // put your main code here, to run repeatedly:

  switch (SysStatus) {
    case RESET  : // Wait for remote control for PowerOn signal
                       if (IRPwrButton() == true)
                       {
#ifdef HASLEDS                        
                              PowerLedOn(true);
#endif                              
                              restartRPi();
                              ForcedRpiRst = 0;
                              SysStatus = BOOT;
                              toggleOutWD(true);
                       }
                     break;
                     
    case BOOT   : // Wait for boot phase of RPi to be completed 
                    if (IRPwrButton() == true) // If the PWR button is pressed 10 times in a raw and status is still BOOT we need to restart RPi
                       {
                              if (ForcedRpiRst++ > 10)
                              {
                                  ForcedRpiRst = 0;
                                  restartRPi();
                                  toggleOutWD(true);                                
                              }
                       }
                       
                   // If WD starts 1Hz pulses then boot is completed
                    if (toggleOutWD(false) == true)
                       {
                         ForcedRpiRst = 0;
                         SysStatus = ON;
                         timeOutWD(0); 
                       }
                 break;
   
    case ON    :  // RPi is up and running until the 1Hz signal stops
                  if (IRPwrButton() == true) // If the PWR button is pressed 10 times in a raw and status is still ON we need to restart RPi
                       {
                              if (ForcedRpiRst++ > 10)
                              {
                                  ForcedRpiRst = 0;
                                  restartRPi();
                                  toggleOutWD(true);
                                  SysStatus = BOOT;                                
                              }
                       }
                       
                    // If 1Hz signal is missing RPi has gone in stand-by   
                   if (timeOutWD(1000) == true)
                   {
                       SysStatus = STDBY;                                
                       toggleOutWD(true);
                       RXLED1;                           
                       
#ifdef HASLEDS                        
                       PowerLedOn(false);
#endif                         
                   }
                 break;
                 
    case STDBY : // Wait for remote control for PowerOn signal
                       if (IRPwrButton() == true)
                       {
                              restartRPi();
                              ForcedRpiRst = 0;
                              SysStatus = BOOT;
                              toggleOutWD(true);
                       }
                    if (toggleOutWD(false) == true)
                       {
                         SysStatus = ON;
                         timeOutWD(0); 
                         RXLED0;
#ifdef HASLEDS                        
                         PowerLedOn(true);
#endif  
                       }  
                 break;
                 
    default    : SysStatus = STDBY;
                 break;            
  }
}
