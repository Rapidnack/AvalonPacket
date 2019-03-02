#include <SPI.h>
#include <AvalonPacket.h>

AvalonPacket ap = AvalonPacket();
byte response[4];
int responseSize;

unsigned long p0;

void setup() {
  Serial.begin(57600);

  delay(1000); // Wait until FPGA is configured

  p0 = 0;
}

void loop() {
  responseSize = ap.write(0, p0++, response);
  ap.printBytes(response, responseSize);
  
  delay(1000);
}
