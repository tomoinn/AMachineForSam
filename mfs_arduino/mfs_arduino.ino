#include "lightStructs.h"

const int ws2803_clockPin = 11;
const int ws2803_dataPin = 12;

#define nLEDs 18

uint8_t ledBar[nLEDs]; // Array representing LED PWM levels (byte size)

void setup() {

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
}

int i = 0;
int value;
void loop() {
  for (int led = 0; led < 6; led++) {
    setPixel(led, hsvToColour(i+(led*255/6), 255, 255)); 
  }
  i++;
  loadWS2803();
  delay(40);
} //loop

void loadWS2803(){
  for (int wsOut = 0; wsOut < nLEDs; wsOut++){
    shiftOut(ws2803_dataPin, ws2803_clockPin, MSBFIRST, ledBar[wsOut]);
  }
  delayMicroseconds(600); // 600us needed to reset WS2803s
}

void setPixel(int index, RGB colour) {
  ledBar[index*3]=colour.r;
  ledBar[index*3+1]=colour.g;
  ledBar[index*3+2]=colour.b;
}

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
