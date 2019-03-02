#ifndef _AVALON_PACKET_H
#define _AVALON_PACKET_H

#include "arduino.h"
#include <SPI.h>

class AvalonPacket
{
	public:
		AvalonPacket();

		void printBytes(byte *bytes, int bytesSize);
		int write(unsigned long addr, byte *array, int arraySize, bool isIncremental, byte *response);
		int write(unsigned long addr, byte data, byte *response);
		int write(unsigned long addr, unsigned int data, byte *response);
		int write(unsigned long addr, unsigned long data, byte *response);
		int write(unsigned long addr, unsigned int *array, int arraySize, bool isIncremental, byte *response);
		int write(unsigned long addr, unsigned long *array, int arraySize, bool isIncremental, byte *response);
		int read(unsigned long addr, byte *array, int arraySize, bool isIncremental);
		int read(unsigned long addr, unsigned int *array, int arraySize, bool isIncremental);
		int read(unsigned long addr, unsigned long *array, int arraySize, bool isIncremental);
		byte readByte(unsigned long addr);
		unsigned int readUInt(unsigned long addr);
		unsigned long readULong(unsigned long addr);

	private:
		bool byteCh = false;
		bool bytesEscape = false;
		bool bitsEscape = false;
		
		int packetToBytes(byte *packet, int packetSize, byte *bytes);
		int bytesToPacket(byte *bytes, int bytesSize, byte *packet);
		int bytesToBits(byte *bytes, int bytesSize, byte *bits);
		int bitsToBytes(byte *bits, int bitsSize, byte *bytes);
		int bitsToBytes(byte *bits, int bitsStart, int bitsLength, byte *bytes);
		int receiveResponse(int requestSize, byte *response);
};

#endif

