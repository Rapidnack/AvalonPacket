#include <SPI.h>
#include <AvalonPacket.h>

AvalonPacket ap = AvalonPacket();
byte response[4];
int responseSize;

unsigned long freqToPhaseInc(double freq) { // in Hz
  double clk = 73714286; // 73.714286MHz
  double phaseInc360 = (double)0x80000000UL * 2; // 32bits full scale
  
  if (freq >= clk)
    freq -= clk;
    
  return (unsigned long)(phaseInc360 * (freq / clk));
}

void setup() {
  Serial.begin(57600);

  delay(1000); // Wait until FPGA is configured

  Serial.setTimeout(-1);
}

void loop() {
  String str = Serial.readStringUntil('\n');
  double freq = str.toDouble();
  unsigned long i = freqToPhaseInc(freq);
  
  Serial.println(freq);
  responseSize = ap.write(0x10, i, response);
  ap.printBytes(response, responseSize);
}
