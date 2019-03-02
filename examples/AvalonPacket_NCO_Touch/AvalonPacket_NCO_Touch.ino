#include <SPI.h>
#include <AvalonPacket.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <nvs.h>

long freq = 1000;
long freqStep = 1000;

SSD1306Wire display(0x3c, 21, 22);

AvalonPacket ap = AvalonPacket();
byte response[4];
int responseSize;

int threshold = 60;
volatile unsigned long touchMillis = 0;
volatile int touchId = 0;

void gotTouch(int id) {
  touchAttachInterrupt(T4, dummy, 0);
  touchAttachInterrupt(T6, dummy, 0);
  touchAttachInterrupt(T8, dummy, 0);
  touchAttachInterrupt(T9, dummy, 0);
  touchMillis = millis();
  touchId = id;
}

void dummy() { }

void gotTouch1() {
  if (touchRead(T4) < threshold) {
    gotTouch(1);
  }
}

void gotTouch2() {
  if (touchRead(T6) < threshold) {
    gotTouch(2);
  }
}

void gotTouch3() {
  if (touchRead(T8) < threshold) {
    gotTouch(3);
  }
}

void gotTouch4() {
  if (touchRead(T9) < threshold) {
    gotTouch(4);
  }
}

unsigned long freqToPhaseInc(double freq) { // in Hz
  double clk = 73714286; // 73.714286MHz
  double phaseInc360 = (double)0x80000000UL * 2; // 32bits full scale
  
  if (freq >= clk)
    freq -= clk;
    
  return (unsigned long)(phaseInc360 * (freq / clk));
}

void sendFreq(double freq) { // in Hz
  unsigned long i = freqToPhaseInc(freq);
  
  Serial.println(freq);
  responseSize = ap.write(0x10, i, response);
  ap.printBytes(response, responseSize);
}

void updateDisplay() {
  display.clear();
  
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, String(freq) + "Hz");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 32,"Step: " + String(freqStep) + "Hz");
  
  display.display();
}

void setup() {
  Serial.begin(57600);

  display.init();
  display.flipScreenVertically();

  delay(1000); // Wait until FPGA is configured

  sendFreq(freq);
  updateDisplay();

  touchAttachInterrupt(T4, gotTouch1, threshold);
  touchAttachInterrupt(T6, gotTouch2, threshold);
  touchAttachInterrupt(T8, gotTouch3, threshold);
  touchAttachInterrupt(T9, gotTouch4, threshold);
}

void loop() {
  if (touchId > 0) {
    Serial.print("Touch: ");
    Serial.println(touchId);

    if (touchId == 1) {
      if (freqStep > 1) {
        freqStep /= 10;
      }
    } else if (touchId == 2) {
      if (freqStep < 1000000) {
        freqStep *= 10;
      }
    } else if (touchId == 3) {
      if (freq > 0) {
        freq -= freqStep;
        if (freq < 0)
          freq = 0;
      }
    } else if (touchId == 4) {
      freq += freqStep;
    }
    
    sendFreq(freq);
    updateDisplay();

    touchId = -1;
  } else if (touchId < 0 && touchMillis + 100 < millis()) {
    touchId = 0;
    touchAttachInterrupt(T4, gotTouch1, threshold);
    touchAttachInterrupt(T6, gotTouch2, threshold);
    touchAttachInterrupt(T8, gotTouch3, threshold);
    touchAttachInterrupt(T9, gotTouch4, threshold);
  }
}
