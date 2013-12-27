// Code for Sam's machine, drives the WS2083 and reads switch data.

#include "lightStructs.h"
#define nLEDs 18

// Clock and Data pin definitions for the WS2803
const int ws2803_clockPin = 12;
const int ws2803_dataPin = 11;

// Array representing LED PWM levels (byte size)
uint8_t ledBar[nLEDs]; 

// Accumulate deltas, these can be reset at any point and track the
// number of ticks on the respective cogs since the last reset.
int aDelta, bDelta, cDelta;

// Track previous values of the switches (held as s1+(s2<<1) where s1 
// and s2 are the digital inputs (0-1) of the microswitches connected
// to a given cog's spindle.
int a,b,c;

void setup() {
  Serial.begin(19200);
  
  pinMode(ws2803_clockPin, OUTPUT);
  pinMode(ws2803_dataPin, OUTPUT);

  // Initialize WS2803 - Clock needs to be low at least 600us to prepare itself.
  digitalWrite(ws2803_clockPin, LOW);
  delayMicroseconds(600);

  // Initialize the ledBar array - all LEDs OFF.
  for(int wsOut = 0; wsOut < nLEDs; wsOut++){
    ledBar[wsOut] = 0x00;
  }
  loadWS2803();
  a = digitalRead(6) + (digitalRead(3) << 1);
  b = digitalRead(5) + (digitalRead(8) << 1);
  c = digitalRead(4) + (digitalRead(7) << 1);
  aDelta = 0;
  bDelta = 256 / 3;
  cDelta = 512 / 3;
}

int fudgeDelta = 1;
int fudgeMax = 100;
int fudge = 0;

void loop() {
  calculateEncoderDeltas(5);
  RGB colA = hsvToColour(aDelta+fudge/10, 255, 255);
  RGB colB = hsvToColour(bDelta+fudge/10, 255, 255);
  RGB colC = hsvToColour(cDelta+fudge/10, 255, 255);
  setPixel(2, colA);
  setPixel(4, colB);
  setPixel(0, colC);
  blendWS2803(1);
  blendWS2803(3);
  blendWS2803(5);
  aDelta = aDelta % 256;
  bDelta = bDelta % 256;
  cDelta = cDelta % 256;
  fudge += fudgeDelta;
  if (fudge > fudgeMax) {
    fudge = fudgeMax;
    fudgeDelta = -fudgeDelta;
  }
  else if (fudge < 0) {
    fudge = 0;
    fudgeDelta = -fudgeDelta;
  }
  loadWS2803();
} 



// Read values from the switches and use the encoderDelta() method
// to modify the values of aDelta etc based on any detected motion.
void calculateEncoderDeltas(int increment) {
  int newA = digitalRead(6) + (digitalRead(3) << 1);
  int newB = digitalRead(4) + (digitalRead(7) << 1);
  int newC = digitalRead(5) + (digitalRead(8) << 1);
  aDelta += encoderDelta(a, newA) * increment;
  bDelta += encoderDelta(b, newB) * increment;
  cDelta += encoderDelta(c, newC) * increment;
  a = newA;
  b = newB;
  c = newC;
}

// These are the new values which indicate clockwise rotation
// on the encoders. If the old value is n then the new value which
// indicates a clockwise tick is clockwise[n]
const int clockwise[] = {2,0,3,1};

// Given an old and new value work out whether this represents
// no movement (if equal) or either positive or negative ticks.
// Uses the clockwise[] array to determine direction.
int encoderDelta(int oldValue, int newValue) {
  if (oldValue == newValue) {
    return 0;
  }
  if (newValue == clockwise[oldValue]) {
    return 1;
  }
  return -1;
}

// Reset delta values to zero.
void resetDeltas() {
  aDelta = 0;
  bDelta = 0;
  cDelta = 0;
}

void blendWS2803(int led) {
  ledBar[led*3] = (ledBar[((led-1)*3)%nLEDs] + ledBar[((led+1)*3)%nLEDs]) >> 1;
  ledBar[led*3 + 1] = (ledBar[((led-1)*3 + 1)%nLEDs] + ledBar[((led+1)*3 + 1)%nLEDs])>>1;
  ledBar[led*3 + 2] = (ledBar[((led-1)*3 + 2)%nLEDs] + ledBar[((led+1)*3 + 2)%nLEDs])>>1;
}

// Push data to the LEDs from the buffer.
void loadWS2803(){
  for (int wsOut = 0; wsOut < nLEDs; wsOut++){
    shiftOut(ws2803_dataPin, ws2803_clockPin, MSBFIRST, ledBar[wsOut]);
  }
  // Nominally needs 600ms, actually less as we're doing other things
  // in the meantime. 300 seems a reasonable value given the other
  // processing done in the loop.
  delayMicroseconds(600);
}

// Set the value of the given RGB LED in the buffer.
void setPixel(int index, RGB colour) {
  ledBar[index*3]=colour.r;
  ledBar[index*3+1]=colour.g;
  ledBar[index*3+2]=colour.b;
}

// Build an RGB value from HSV, all values are 0-255
RGB hsvToColour(unsigned int h, unsigned int s, unsigned int v) {
  unsigned char region, remainder, p, q, t;
  h = (h+256) % 256;
  if (s > 255) s = 255;
  if (v > 255) v = 255;
  else v = (v * v) >> 8;
  if (s == 0) return (RGB){v, v, v};
  region = h / 43;
  remainder = (h - (region * 43)) * 6; 
  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  switch (region) {
  case 0:
    return (RGB){v, p, t};
  case 1:
    return (RGB){q, p, v};
  case 2:
    return (RGB){p, t, v};
  case 3:
    return (RGB){p, v, q};
  case 4:
    return (RGB){t, v, p};
  }
  return (RGB){v, q, p};
}
