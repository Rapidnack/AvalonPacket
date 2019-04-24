#include "arduino.h"
#include "AvalonPacket.h"
#include <SPI.h>


AvalonPacket::AvalonPacket()
{
	SPI.begin();
	SPI.setFrequency(1000000);
	SPI.setDataMode(SPI_MODE1);
	SPI.setBitOrder(MSBFIRST);
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
}

void  AvalonPacket::printBytes(byte *bytes, int bytesSize) {
  for (int i = 0; i < bytesSize; i++) {
    Serial.print(bytes[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

int AvalonPacket::write(unsigned long addr, byte *array, int arraySize, bool isIncremental, byte *response)
{
	int packetSize = 8 + arraySize;
	byte *packet = new byte[packetSize];
	byte sizeBytes[2] = { arraySize & 0xff, (arraySize >> 8) & 0xff };
	byte addrBytes[4] = { addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff };
	packet[0] = (byte)(isIncremental ? 0x04 : 0x00);
	packet[1] = 0;
	packet[2] = sizeBytes[1];
	packet[3] = sizeBytes[0];
	packet[4] = addrBytes[3];
	packet[5] = addrBytes[2];
	packet[6] = addrBytes[1];
	packet[7] = addrBytes[0];
	for (int i = 0; i < arraySize; i++)
	{
		packet[8 + i] = array[i];
	}

	byte *bytes = new byte[packetSize * 2];
	int bytesSize = packetToBytes(packet, packetSize, bytes);
	delete [] packet;

	byte *bits = new byte[bytesSize * 2];
	int bitsSize = bytesToBits(bytes, bytesSize, bits);
	delete [] bytes;

	byte *spiReadBuf = new byte[bitsSize];
	digitalWrite(SS, LOW);
	SPI.transferBytes(bits, spiReadBuf, bitsSize);
	digitalWrite(SS, HIGH);
	delete [] bits;
	delete [] spiReadBuf;

	return receiveResponse(4, response);
}

int AvalonPacket::write(unsigned long addr, byte data, byte *response)
{
	byte bytes[1] = { data };
	return write(addr, bytes, 1, false, response);
}

int AvalonPacket::write(unsigned long addr, unsigned int data, byte *response)
{
	byte bytes[2] = { data & 0xff, (data >> 8) & 0xff };
	return write(addr, bytes, 2, false, response);
}

int AvalonPacket::write(unsigned long addr, unsigned long data, byte *response)
{
	byte bytes[4] = { data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff, (data >> 24) & 0xff };
	return write(addr, bytes, 4, false, response);
}

int AvalonPacket::write(unsigned long addr, unsigned int *array, int arraySize, bool isIncremental, byte *response)
{
	int numBytes = 2;
	byte *bytes = new byte[numBytes * arraySize];
	for (int i = 0; i < arraySize; i++)
	{
		bytes[numBytes * i + 0] = array[i] & 0xff;
		bytes[numBytes * i + 1] = (array[i] >> 8) & 0xff;
	}
	int ret = write(addr, bytes, numBytes * arraySize, isIncremental, response);
	delete [] bytes;
	return ret;
}

int AvalonPacket::write(unsigned long addr, unsigned long *array, int arraySize, bool isIncremental, byte *response)
{
	int numBytes = 4;
	byte *bytes = new byte[numBytes * arraySize];
	for (int i = 0; i < arraySize; i++)
	{
		bytes[numBytes * i + 0] = array[i] & 0xff;
		bytes[numBytes * i + 1] = (array[i] >> 8) & 0xff;
		bytes[numBytes * i + 2] = (array[i] >> 16) & 0xff;
		bytes[numBytes * i + 3] = (array[i] >> 24) & 0xff;
	}
	int ret = write(addr, bytes, numBytes * arraySize, isIncremental, response);
	delete [] bytes;
	return ret;
}

int AvalonPacket::read(unsigned long addr, byte *array, int arraySize, bool isIncremental)
{
	if (arraySize == 0)
		return 0;

	int packetSize = 8;
	byte *packet = new byte[packetSize];
	byte sizeBytes[2] = { arraySize & 0xff, (arraySize >> 8) & 0xff };
	byte addrBytes[4] = { addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff };
	packet[0] = (byte)(isIncremental ? 0x14 : 0x10);
	packet[1] = 0;
	packet[2] = sizeBytes[1];
	packet[3] = sizeBytes[0];
	packet[4] = addrBytes[3];
	packet[5] = addrBytes[2];
	packet[6] = addrBytes[1];
	packet[7] = addrBytes[0];

	byte *bytes = new byte[packetSize * 2];
	int bytesSize = packetToBytes(packet, packetSize, bytes);
	delete [] packet;

	byte *bits = new byte[bytesSize * 2];
	int bitsSize = bytesToBits(bytes, bytesSize, bits);
	delete [] bytes;

	byte *spiReadBuf = new byte[bitsSize];
	digitalWrite(SS, LOW);
	SPI.transferBytes(bits, spiReadBuf, bitsSize);
	digitalWrite(SS, HIGH);
	delete [] bits;
	delete [] spiReadBuf;

	return receiveResponse(arraySize, array);
}

int AvalonPacket::read(unsigned long addr, unsigned int *array, int arraySize, bool isIncremental)
{
	if (arraySize == 0)
		return 0;

	int numBytes = 2;
	byte *bytes = new byte[numBytes * arraySize];
	int bytesSize = read(addr, bytes, numBytes * arraySize, isIncremental);
	for (int i = 0; i < arraySize; i++)
	{
		array[i] = bytes[numBytes * i + 0]
			+ (bytes[numBytes * i + 1] << 8);
	}
	delete [] bytes;
	return arraySize;
}

int AvalonPacket::read(unsigned long addr, unsigned long *array, int arraySize, bool isIncremental)
{
	if (arraySize == 0)
		return 0;

	int numBytes = 4;
	byte *bytes = new byte[numBytes * arraySize];
	int bytesSize = read(addr, bytes, numBytes * arraySize, isIncremental);
	for (int i = 0; i < arraySize; i++)
	{
		array[i] = bytes[numBytes * i + 0]
			+ (bytes[numBytes * i + 1] << 8)
			+ (bytes[numBytes * i + 2] << 16)
			+ (bytes[numBytes * i + 3] << 24);
	}
	delete [] bytes;
	return arraySize;
}

byte AvalonPacket::readByte(unsigned long addr)
{
	byte array[1];
	int arraySize = read(addr, array, 1, false);
	if (arraySize == 0)
		return 0;
	return array[0];
}

unsigned int AvalonPacket::readUInt(unsigned long addr)
{
	unsigned int array[1];
	int arraySize = read(addr, array, 1, false);
	if (arraySize == 0)
		return 0;
	return array[0];
}

unsigned long AvalonPacket::readULong(unsigned long addr)
{
	unsigned long array[1];
	int arraySize = read(addr, array, 1, false);
	if (arraySize == 0)
		return 0;
	return array[0];
}

int AvalonPacket::packetToBytes(byte *packet, int packetSize, byte *bytes)
{
	int bytesSize = 0;
	bytes[bytesSize++] = 0x7c;
	bytes[bytesSize++] = 0;
	bytes[bytesSize++] = 0x7a;
	for (int i = 0; i < packetSize; i++)
	{
		byte p = packet[i];
		if (0x7a <= p && p <= 0x7d)
		{
			if (i == packetSize - 1)
			{
				bytes[bytesSize++] = 0x7b;
			}
			bytes[bytesSize++] = 0x7d;
			bytes[bytesSize++] = (byte)(p ^ 0x20);
		}
		else
		{
			if (i == packetSize - 1)
			{
				bytes[bytesSize++] = 0x7b;
			}
			bytes[bytesSize++] = p;
		}
	}
	return bytesSize;
	
}

int AvalonPacket::bytesToPacket(byte *bytes, int bytesSize, byte *packet)
{
	int packetSize = 0;
	for (int i = 0; i < bytesSize; i++)
	{
		byte b = bytes[i];
		if (b == 0x7a || b == 0x7b)
		{
			// Dropped
		}
		else if (b == 0x7c)
		{
			byteCh = true;
			// Dropped
		}
		else if (b == 0x7d)
		{
			bytesEscape = true;
			// Dropped
		}
		else
		{
			if (byteCh)
			{
				byteCh = false;
				// Dropped
			}
			else if (bytesEscape)
			{
				bytesEscape = false;
				packet[packetSize++] = (byte)(b ^ 0x20);
			}
			else
			{
				packet[packetSize++] = b;
			}
		}
	}
	return packetSize;
	
}

int AvalonPacket::bytesToBits(byte *bytes, int bytesSize, byte *bits)
{
	int bitsSize = 0;
	for (int i = 0; i < bytesSize; i++)
	{
		byte b = bytes[i];
		if (b == 0x4a || b == 0x4d)
		{
			bits[bitsSize++] = 0x4d;
			bits[bitsSize++] = (byte)(b ^ 0x20);
		}
		else
		{
			bits[bitsSize++] = b;
		}
	}
	return bitsSize;
	
}

int AvalonPacket::bitsToBytes(byte *bits, int bitsSize, byte *bytes)
{
	return bitsToBytes(bits, 0, bitsSize, bytes);
}

int AvalonPacket::bitsToBytes(byte *bits, int bitsStart, int bitsLength, byte *bytes)
{
	int bytesSize = 0;
	for (int i = bitsStart; i < bitsStart + bitsLength; i++)
	{
		byte b = bits[i];
		if (b == 0x4a)
		{
			// Dropped
		}
		else if (b == 0x4d)
		{
			bitsEscape = true;
			// Dropped
		}
		else
		{
			if (bitsEscape)
			{
				bitsEscape = false;
				bytes[bytesSize++] = (byte)(b ^ 0x20);
			}
			else
			{
				bytes[bytesSize++] = b;
			}
		}
	}
	return bytesSize;
}

int AvalonPacket::receiveResponse(int requestSize, byte *response)
{
	byteCh = false;
	bytesEscape = false;
	bitsEscape = false;

	int responseSize = 0;
	while (responseSize < requestSize)
	{
		int size = (int)((requestSize - responseSize) * 1.1) + 4;
		byte *bits = new byte[size];
		for (int i = 0; i < size; i++)
		{
			bits[i] = 0x4a;
		}
		byte *spiReadBuf = new byte[size];
		digitalWrite(SS, LOW);
		SPI.transferBytes(bits, spiReadBuf, size);
		digitalWrite(SS, HIGH);
		delete [] bits;

		byte *bytes = new byte[size];
		int bytesSize = bitsToBytes(spiReadBuf, size, bytes);
		delete [] spiReadBuf;

		byte *packet = new byte[size];
		int packetSize = bytesToPacket(bytes, bytesSize, packet);
		delete [] bytes;

		for (int i = 0; i < packetSize; i++)
		{
			response[responseSize++] = packet[i];
		}
		delete [] packet;
	}
	return responseSize;
}

