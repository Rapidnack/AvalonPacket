#include <SPI.h>
#include <AvalonPacket.h>

AvalonPacket ap = AvalonPacket();
byte response[4];
int responseSize;

unsigned long hexToULong(String str) {
  char buf[80];
  str.toCharArray(buf, 80);
  Serial.println(buf);
  return strtoul(buf, 0, 16);  
}

void setup() {
  Serial.begin(57600);

  delay(1000); // Wait until FPGA is configured

  Serial.setTimeout(-1);
}

void loop() {
  String str = Serial.readStringUntil('\n');
  unsigned long i = hexToULong(str);  
  responseSize = ap.write(0, i, response);
  ap.printBytes(response, responseSize);  
}
