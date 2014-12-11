#include <digitalWriteFast.h>

//======================================================================================
// Kurt's hacked up RC receiver Program
//======================================================================================
#include <PinChangeInt.h>    // http://www.arduino.cc/playground/Main/PinChangeInt
#include <PinChangeIntConfig.h>

// Currently I am only using 6 pins, 2-7 so they are all on PortD, if we go up to 7 pins will need
// to enable PortB
// BUGBUG:: THis is pointless?  Need to define in the actual file (that is used by PCChangInt.cpp...)

#define PIN_COUNT 6   //number of channels attached to the reciver
#if PIN_COUNT < 7
#define NO_PORTB_PINCHANGES //PinChangeInt setup
#endif
#define NO_PORTC_PINCHANGES    //only port D pinchanges (see: http://www.arduino.cc/playground/Learning/Pins)

#define MAX_PIN_CHANGE_PINS PIN_COUNT

// Note: will need to change for Mega...
#define PIN0            2
#define PIN1            3
#define PIN2            4
#define PIN3            5
#define PIN4            6
#define PIN5            7

static byte     s_abPins[PIN_COUNT] = {PIN0, PIN1, PIN2, PIN3, PIN4, PIN5};
unsigned long   g_ulLastPinChange;
unsigned long   g_aulStartTime[PIN_COUNT];              // Needed to hold when a pulse started... Often 0
word            g_awPulseWidth[PIN_COUNT];             // The last pulse width for each pin.
word            g_awPulseWidthPrev[PIN_COUNT];             // The last pulse width for each pin.
boolean         g_fPulsesValid;
boolean         g_fPulsesValidPrev;
byte            g_cValidSigs;                           // Counter to know if we have a full set of valid signals...

float throttle = 0;
float left = 0;
float right = 0;


void setup() {
    Serial.begin(115200);
    Serial.println("Kurt's ReciverReading test");

    // This is garbage but I am having seperate PinChange Interrupt functions for each
    // pin as there is no way to know which pin I am being called for...  Maybe later
    // will update the pinchange interrupt to pass back some info...
   g_fPulsesValidPrev = false;     

    for (byte i=0; i<PIN_COUNT; i++) {
        pinMode(s_abPins[i], INPUT);     //set the pin to input
        digitalWrite(s_abPins[i], HIGH); // enable PU - In case nothing connected...
        g_awPulseWidth[i] = 0;
        g_aulStartTime[i] = 0;
    }

    // Here is the first garbage of it...
    g_ulLastPinChange = 0;
    PCintPort::attachInterrupt(s_abPins[0], PinChange0, CHANGE); 
    PCintPort::attachInterrupt(s_abPins[1], PinChange1, CHANGE); 
    PCintPort::attachInterrupt(s_abPins[2], PinChange2, CHANGE); 
    PCintPort::attachInterrupt(s_abPins[3], PinChange3, CHANGE); 
    PCintPort::attachInterrupt(s_abPins[4], PinChange4, CHANGE); 
    PCintPort::attachInterrupt(s_abPins[5], PinChange5, CHANGE); 
}

void loop() {
  Serial.print("throttle");
  Serial.println(throttle);
  Serial.print("left");
  Serial.println(left);
  Serial.print("throttle");
  Serial.println(right);
  analogWrite(9, throttle + right);
  analogWrite(10, throttle + left);


}

void PinChange0() {
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(0, digitalReadFast(PIN0));

}    
    
void PinChange1() {
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(1, digitalReadFast(PIN1));
    
}    

void PinChange2() { //rudder
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(2, digitalReadFast(PIN2));
    float level = (g_awPulseWidth[2] - 980)  ;
    level = level/1000;
    level = (255 * level) - 127;
    
    if(level > 0){
      right = level;
      left = 0;
    }else{
      left = level;
      right = 0;     
    } 

    
}    

void PinChange3() { //throttle
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(3, digitalReadFast(PIN3));
    float level = (g_awPulseWidth[3] - 980)  ;
    level = level/1000;
    level = 255 * level;
    throttle = level - 127;
}    

void PinChange4() {
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(4, digitalReadFast(PIN4));

}    

void PinChange5() {
    // Hack up version to know which interrupt happened and what the state is...
    PinChange(5, digitalReadFast(PIN5));

}    

void PinChange(byte iPin, byte bPinState) {
    unsigned long ulTime = micros();    // get the time when we entered here
    g_ulLastPinChange = ulTime;
    if (bPinState) {
        g_aulStartTime[iPin] = ulTime;
    } else {
        ulTime -= g_aulStartTime[iPin];    // Get the delta time
        if ((ulTime >= 750) && (ulTime <= 2250)) {  // Do some validation of information.
            g_awPulseWidth[iPin] = ulTime;    //    
            if (g_cValidSigs < PIN_COUNT)
                g_cValidSigs++;            // keep a count of valid items up to count of pins...
        }
        else
            g_cValidSigs = 0;    // reset count of valid signals back to zero
    }
}
 
