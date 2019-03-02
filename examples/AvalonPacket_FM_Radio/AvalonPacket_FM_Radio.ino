#include <SPI.h>
#include <AvalonPacket.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <nvs.h>

uint8_t volume = 0;
uint8_t station = 0;
const int NUM_STATIONS = 6;
double freq[NUM_STATIONS] = {
  80.0e6,
  81.3e6,
  82.5e6,
  90.5e6,
  91.6e6,
  93.0e6
};
String freqString[NUM_STATIONS] = {
  "80.0 MHz",
  "81.3 MHz",
  "82.5 MHz",
  "90.5 MHz",
  "91.6 MHz",
  "93.0 MHz"
};
String stationName[NUM_STATIONS] = {
  "TOKYO FM",
  "J-WAVE",
  "NHK FM",
  "TBS FM",
  "JOQR FM",
  "NIPPON FM"
};

SSD1306Wire display(0x3c, 21, 22);

nvs_handle handle = 0;

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

void sendVolume(unsigned long volume) { // 0 - 15
  responseSize = ap.write(0, volume, response);
  ap.printBytes(response, responseSize);  
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
  display.drawString(0, 0, stationName[station]);
  display.drawString(0, 24, freqString[station]);
  
  display.setFont(ArialMT_Plain_24);
  String str = "";
  for (int i = 0; i < volume; i++) {
    str += "*";
  }
  display.drawString(0, 48, str);
  
  display.display();
}

void setup() {
  Serial.begin(57600);

  display.init();
  display.flipScreenVertically();

  nvs_open("FPGA Radio", NVS_READWRITE, &handle);
  if (handle != 0) {
    nvs_get_u8(handle, "volume", &volume);
    nvs_get_u8(handle, "station", &station);
  }
  if (15 < volume) volume = 15;
  if (NUM_STATIONS-1 < station) station = NUM_STATIONS-1;

  delay(1000); // Wait until FPGA is configured

  sendVolume(volume);
  sendFreq(freq[station]);
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
      if (station > 0) {
        station -= 1;
      } else {
        station = NUM_STATIONS-1;
      }
    } else if (touchId == 2) {
      if (station < NUM_STATIONS-1) {
        station += 1;
      } else {
        station = 0;
      }
    } else if (touchId == 3) {
      if (volume > 0) {
        volume -= 1;
      }
    } else if (touchId == 4) {
      if (volume < 15) {
        volume += 1;
      }
    }
    
    sendFreq(freq[station]);
    sendVolume(volume);
    if (handle != 0) {
      nvs_set_u8(handle, "station", station);
      nvs_set_u8(handle, "volume", volume);
    }
    updateDisplay();

    touchId = -1;
  } else if (touchId < 0 && touchMillis + 300 < millis()) {
    touchId = 0;
    touchAttachInterrupt(T4, gotTouch1, threshold);
    touchAttachInterrupt(T6, gotTouch2, threshold);
    touchAttachInterrupt(T8, gotTouch3, threshold);
    touchAttachInterrupt(T9, gotTouch4, threshold);
  }
}
