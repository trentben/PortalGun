// Code to control a Rick and Morty Portal Gun
// Written by Brandon Pomeroy, 2015

/*
******** Required Libraries *************
* ClickEncoder - https://github.com/0xPIT/encoder
* Adafruit_GFX - https://github.com/adafruit/Adafruit-GFX-Library
* Adafruit_LEDBackpack - https://github.com/adafruit/Adafruit-LED-Backpack-Library
*/


/*
********** Required Hardware *********************
* Adafruit Pro Trinket 5V 16MHz - http://www.adafruit.com/product/2000
* LiPoly BackPack - http://www.adafruit.com/product/2124
* LiPoly Battety 3.7V - http://www.adafruit.com/products/1578
* Rotary Encoder - http://www.adafruit.com/products/377
* Metal Knob - http://www.adafruit.com/products/2056
* Quad Alphanumeric Display (Red 0.54") - http://www.adafruit.com/products/1911
* 10mm Diffused Green LED (x4) - https://www.adafruit.com/products/844
* 10mm Plastic Bevel LED Holder (x4) - https://www.adafruit.com/products/2171
* 150 Ohm Resistor (x4) for LEDs
* Inductive Charging Set - 5V - https://www.adafruit.com/products/1407
* 2.1mm Panel Mount Barrel Jack - http://www.adafruit.com/products/610
* 9VDC Power Supply - http://www.adafruit.com/products/63
*/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Set up our LED display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
char displayBuffer[4];
uint8_t dimensionLetter='C';

// Set up the click encoder
ClickEncoder *encoder;
int16_t last, value;
#define encoderPinA          A1
#define encoderPinB          A0
#define encoderButtonPin     A2

// Steps per notch can be 1, 4, or 8. If your encoder is counting
// to fast or too slow, change this!
#define stepsPerNotch        4

// Comment this line to make the encoder increment in the opposite direction
#define reverseEncoderWheel


// FX Board output delay (ms)
const int msDelay = 500;

// Set up the Green LEDs
#define topBulbPin           9
#define frontRightPin        3
#define frontCenterPin       5
#define frontLeftPin         6
int maxBright = 254;
int mediumBright = 50;

// Set up what we need to sleep/wake the Trinket
// Define the pins you'll use for interrupts - CHANGE THESE to match the input pins
// you are using in your project
#define NAV0_PIN A2

//Let us know if our Trinket woke up from sleep
volatile bool justWokeUp;

char *messages[] = {"GET SWIFTY", "WUBBA LUBBA DUB DUB", "PICKLE RICK", "EXISTENCE IS PAIN"};

void timerIsr() {
  encoder->service();
}

void setup() {
  enablePinInterupt(NAV0_PIN);
  
  //Set up pin modes
  pinMode(topBulbPin, OUTPUT);
  pinMode(frontRightPin, OUTPUT);
  pinMode(frontLeftPin, OUTPUT);
  pinMode(frontCenterPin, OUTPUT);
  
  
  setFrontLights(mediumBright);
  digitalWrite(topBulbPin, HIGH);
  
  
  
  encoderSetup();
  alpha4.begin(0x70);  // pass in the address for the LED display
  
  justWokeUp = false;

  randomSeed(analogRead(A3));
  
  //uncomment this to make the display run through a test at startup
  //displayTest();
}

void loop() {
  if (justWokeUp) {
    updateDimension();
    digitalWrite(topBulbPin, HIGH);
    fadeDisplayLights(0, 16, 300);
    fadeFrontLights(0, mediumBright, 500);
  }  

  
  ClickEncoder::Button b = encoder->getButton();
  switch (b) {
    case ClickEncoder::Held:
      // Holding the button will put your trinket to sleep.
      // The trinket will wake on the next button press
      setDisplay("RICK");
      fadeFrontLights(mediumBright, 0, 500);
      digitalWrite(topBulbPin, LOW);
      fadeDisplayLights(15, 0, 300);
      setDisplay("");
      delay(500);
      justWokeUp = true;
      goToSleep();
    return;
    case ClickEncoder::Clicked:
      if (!justWokeUp) {
        fadeFrontLights(mediumBright, maxBright, 100);
        fadeFrontLights(maxBright, mediumBright, 500);

        String msg = getDimensionMessage();
        if (!msg.equals("")) {
          scrollText(msg);
        }
      }
      justWokeUp = false;
    break;
    case ClickEncoder::DoubleClicked:
      //If you double click the button, it sets the dimension to C137
      dimensionLetter = 'C';
      value = 137;
    break;
    case ClickEncoder::Open:
      // The dimension will increment from 0-999, then roll over to the next
      // letter. (A999 -> B000)
      updateDimension();
    break;
  }
}

void encoderSetup(){
    // set up encoder
    encoder = new ClickEncoder(encoderPinA, encoderPinB, encoderButtonPin, stepsPerNotch);
    encoder->setAccelerationEnabled(true);
  
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr); 
    last = -1;
    value = 137;
}


String getDimensionMessage() {
  if (value == 132) {
    return "GET SWIFTY";
    
  } else if (value == 777){
    int rand = random(sizeof(messages)/2);
    return messages[rand];
    
  } else if (dimensionLetter == 'A' && (value/10) == 55) {
    //Any dimension A55X
    return "HOLE";
   
  } else if (dimensionLetter == 'C' && value == 420) {
    return "GET LIT";
    
  } else if (value == 444) {
    return "KALAXIAN CRYSTALS";
    
  } else if (dimensionLetter == 'R') {
    return "BLIPS AND CHITZ";
  } else if (dimensionLetter == 'C' && value == 345) {
    return "BURNING MAN 2019";
  } else if (dimensionLetter == 'A' && value == 421) {
    return "ART OUTSIDE";
  } else if (dimensionLetter == 'F' && value == 010) {
    return "BURNING FLIPSIDE";
  }
  return "";
}



void fadeFrontLights(int from, int to, int duration) {
    int delta = to - from;
    int increment = from < to ? 1 : -1;
    int wait = duration/abs(delta);

    if (wait == 0) {
      wait = 1;
      increment = delta/duration;
    }
    
    int i = from;
    bool done = false;
    while (!done) {
      setFrontLights(i);
      delay(wait);

      i += increment;
      done = delta >= 0 ? i > to : i < to;
    }

    setFrontLights(to);
    
}

void fadeDisplayLights(int from, int to, int duration) {
    int delta = to - from;
    int increment = from < to ? 1 : -1;
    int wait = duration/abs(delta);

    if (wait == 0) {
      wait = 1;
      increment = delta/duration;
    }
    
    int i = from;
    bool done = false;
    while (!done) {
      alpha4.setBrightness(i);
      delay(wait);

      i += increment;
      done = delta >= 0 ? i > to : i < to;
    }

    alpha4.setBrightness(to);
}

void setFrontLights(int brightness) {
      analogWrite(frontRightPin, brightness);
      analogWrite(frontCenterPin, brightness);
      analogWrite(frontLeftPin, brightness);
}

void setDisplay(String text) {
      alpha4.clear();

      for (int i=0; i < text.length() && i < 4; i++) {
        alpha4.writeDigitAscii(i, text.charAt(i));
      }
      alpha4.writeDisplay();
}

void scrollText(String msg) {

  for (int i = -4; i <= (int) msg.length(); i++) {
    alpha4.clear();
    for (int n=0; n < 4; n++) {
      alpha4.writeDigitAscii(n, safeCharAt(msg, i + n));
    }
    alpha4.writeDisplay();

    // Chnage the delay to adjust scroll speed
    delay(200);
  }
}

char safeCharAt(String text, int idx) {
  
  if (idx < 0 || idx >= text.length() ) {
    return ' ';
  } else {
    return text.charAt(idx);
  }
}

void updateDimension(){
  #ifdef reverseEncoderWheel
  value -= encoder->getValue();
  #endif
  
  #ifndef reverseEncoderWheel
  value += encoder->getValue();
  #endif
  
  if (value != last) {
    if (value > 999){
      value = 0;
      if (dimensionLetter == 'Z') {
        dimensionLetter = 'A';
      } else {
        dimensionLetter ++;        
      }
    } else if ( value < 0 ) {
      value = 999;
      if (dimensionLetter == 'A') {
        dimensionLetter = 'Z';
      } else {
        dimensionLetter --;
      }
    }
    last = value;
  }
  
  sprintf(displayBuffer, "%03i", value);
  alpha4.clear();
  alpha4.writeDigitAscii(0, dimensionLetter);
  alpha4.writeDigitAscii(1, displayBuffer[0]);
  alpha4.writeDigitAscii(2, displayBuffer[1]);
  alpha4.writeDigitAscii(3, displayBuffer[2]);
  alpha4.writeDisplay();
}





/*
============== Sleep/Wake Methods ==================
====================================================
*/

// Most of this code comes from seanahrens on the adafruit forums
// http://forums.adafruit.com/viewtopic.php?f=25&t=59392#p329418


void enablePinInterupt(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void goToSleep()   
{
// The ATmega328 has five different sleep states.
// See the ATmega 328 datasheet for more information.
// SLEEP_MODE_IDLE -the least power savings 
// SLEEP_MODE_ADC
// SLEEP_MODE_PWR_SAVE
// SLEEP_MODE_STANDBY
// SLEEP_MODE_PWR_DOWN -the most power savings
// I am using the deepest sleep mode from which a
// watchdog timer interrupt can wake the ATMega328

 


set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode.
sleep_enable(); // Enable sleep mode.
sleep_mode(); // Enter sleep mode.
// After waking the code continues
// to execute from this point.

sleep_disable(); // Disable sleep mode after waking.                   
}

ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{    
  // if I wired up D8-D13 then I'd need some code here
} 

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here // NAV0
{
    /* This will bring us back from sleep. */
  
  /* We detach the interrupt to stop it from 
   * continuously firing while the interrupt pin
   * is low.
   */
  
  detachInterrupt(0);

}

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here // NAV1, NAV2
{
  // Check it was NAV1 or NAV2 and nothing else
}
  

/*
============== Testing Methods ==================
=================================================
*/

void displayTest() {
  
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  
  alpha4.clear();
  alpha4.writeDisplay();

  // display every character, 
  for (uint8_t i='!'; i<='z'; i++) {
    alpha4.writeDigitAscii(0, i);
    alpha4.writeDigitAscii(1, i+1);
    alpha4.writeDigitAscii(2, i+2);
    alpha4.writeDigitAscii(3, i+3);
    alpha4.writeDisplay();
    delay(300);
  }
}
