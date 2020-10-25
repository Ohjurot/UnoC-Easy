/*
 * SimpleSerial.h Single header serial libary for avr-c Arduino UNO. Combine this file with your empty
 * Arduino Project (no include Arduino, no setup and no loop). Works best in combination with
 * "OnlySerial.h" no user setup required! Just include OnlySerial bevore this header
 *
 * Licensed under the MIT License
 *
 * Copyright (c) 2020 Ludwig Füchsl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */ 

// BEGIN CPP Guard
#ifdef __cplusplus
extern "C" {
	#endif

// BEGIN SimpleSerial
#ifndef SIMPLESERIAL_H_
#define SIMPLESERIAL_H_

//////////////////////////////////////////////////////////////////////////
// BEGIN USER INTERFACE

#ifndef HIWORD
#define HIWORD(dword) (char)((dword >> 8) & 0xFF)
#endif

#ifndef LOWORD
#define LOWORD(dword) (char)(dword & 0x00FF)
#endif

#define SSER_4BIT_LO(byte) (byte & 0x0F)
#define SSER_4BIT_HI(byte) ((byte & 0xF0) >> 4)

// Print a null terminated string
void SerialPrintStr(char* input);

// Print a null terminated string and a new line
void SerialPrintLn(char* input);

// Print character
void SerialPrintChar(char input);

// Print a new line
void SerialPrintNewLine(void);

// Print a int
void SerialPrintInt(int16_t input);

// Print a uint
void SerialPrintUInt(uint16_t input);

// Print a float
void SerialPrintFloat(double input);

// Print a byte as hex
void SerialPrintHex(char byte);

// Print a bytes as binary
void SerialPrintBin(char byte);

// Read a char from the serial rx buffer
int SerialReadChar(char* pChar);

// Read a word if available (until 0x00)
int SerialReadStr(char* pChar, uint16_t maxRead);

// Read a word if available (until '\n')
int SerialReadLine(char* pChar, uint16_t maxRead);

// Read as long as peek succeeds
int SerialReadAll(char* pChar, uint16_t maxRead);

// END USER INTREFACE
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN DEFINE CHECK

#ifndef SSER_WRITE_COUNT
#error "It is required to define the SSER_WRITE_COUNT(buf, count) function!"
#endif

#ifndef SSER_WRITE_DELIM
#error "It is required to define the SSER_WRITE_DELIM(buffer, delim) function!"
#endif

#ifndef SSER_PEEK
#error "It is required to define the BOOL = SSER_PEEK(pChar) function!"
#endif

#ifndef SSER_READ_COUNT
#error "It is required to define the SSER_READ_COUNT(buf, count) function!"
#endif

#ifndef SSER_READ_DELIM
#error "It is required to define the UINT = SSER_READ_DELIM(buf, count, delim) function!"
#endif

// END DEFINE CHECK
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN INTERNAL DECL

void sser_uiToChar(uint16_t input, uint8_t base, char** ppBuffer);

void sser_4bitChar(char byte, char* pTarget);

// END INTERNAL DECL
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN IMPLEMENTATION

void SerialPrintStr(char* input){
	// Write until \0 reached
	SSER_WRITE_DELIM(input, '\0');
}

void SerialPrintLn(char* input) {
	// Write without newline
	SerialPrintStr(input);
	// Send newline
	SerialPrintChar('\n');
}

void SerialPrintChar(char input) {
	// Write single time
	SSER_WRITE_COUNT(&input, 1);
}

void SerialPrintNewLine(void) {
	// Send new line
	SerialPrintChar('\n');
}

void SerialPrintInt(int16_t input) {
	// Buffer for base calc
	char buffer[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	char* pos = &buffer[4];

	// Absolute to char
	sser_uiToChar(abs(input), 10, &pos);

	// Set minus if required
	if(input < 0){
		// Set minus and print
		*pos = '-';
		SSER_WRITE_DELIM(pos, 0x0);
	}
	else{
		// No minus print
		SSER_WRITE_DELIM((pos + 1), 0x0);
	}
}

void SerialPrintUInt(uint16_t input) {
	// Buffer for base calc
	char buffer[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	char* pos = &buffer[4];

	// UI to char
	sser_uiToChar(input, 10, &pos);

	// Output
	SSER_WRITE_DELIM((pos + 1), 0x0);
}

void SerialPrintFloat(double input) {
	// Buffer
	char buffer[10] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	
	// Convert float
	dtostrf(input, 9, 3, buffer);

	// Move to first char
	char* pos = buffer;
	while(*pos == ' '){
		pos++;
	}
	
	// Print
	SSER_WRITE_DELIM(pos, 0x0);
}

void SerialPrintHex(char byte) {
	// Buffer
	char buffer[4] = {'0', 'x', '?', '?'};

	// Hi 4bit
	sser_4bitChar(SSER_4BIT_HI(byte), &buffer[2]);
	sser_4bitChar(SSER_4BIT_LO(byte), &buffer[3]);

	// Lo 4bit

	// Print
	SSER_WRITE_COUNT(buffer, 4);
}

void SerialPrintBin(char byte) {
	// Empty byte array (all low)
	char buffer[10] = {'0', 'b', '0', '0', '0', '0', '0', '0', '0', '0'};
	
	// Set bytes witch are set
	for(uint8_t i = 0; i < 8; i++){
		if((1U << i) & byte){
			buffer[9 - i] = '1';
 		}
	}

	// Print
	SSER_WRITE_COUNT(buffer, 10);
}

int SerialReadChar(char* pChar) {
	// Peek for character
	if(SSER_PEEK(0)){
		// Set char
		SSER_READ_COUNT(pChar, 1);
		// Char found and set
		return 1;
	}
	
	// No char found
	return 0;
}

int SerialReadStr(char* pChar, uint16_t maxRead) {
	// Peek for character
	if(SSER_PEEK(0)){
		// Set char
		return SSER_READ_DELIM(pChar, maxRead, 0x00);
	}
	
	// No char found
	return 0;
}

int SerialReadLine(char* pChar, uint16_t maxRead) {
	// Peek for character
	if(SSER_PEEK(0)){
		// Set char
		return SSER_READ_DELIM(pChar, maxRead, '\n');
	}
	
	// No char found
	return 0;
}

int SerialReadAll(char* pChar, uint16_t maxRead) {
	// For each char
	uint16_t i;
	for(i = 0; i < maxRead && SSER_PEEK(0); i++){
		SSER_READ_COUNT(&pChar[i], 1);
	}
	
	// Return count
	return i;
}


void sser_uiToChar(uint16_t input, uint8_t base, char** ppBuffer) {
	do {
		uint16_t  workNumber = input;
		input /= base;
		uint8_t output = workNumber - base * input;

		sser_4bitChar(output, *ppBuffer);
		(*ppBuffer)--;
	} while (input);
}

void sser_4bitChar(char byte, char* pTarget) {
	*pTarget = byte < 10 ? byte + '0' : byte - 10 + 'A';
}

// END IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////



// END SimpleSerial
#endif /* SIMPLESERIAL_H_ */

// END CPP Guard
#ifdef __cplusplus
}
#endif