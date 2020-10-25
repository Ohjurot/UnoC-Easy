/*
 * "OnlySerial.h" Single header serial libary for avr-c Arduino UNO. Combine this file with your empty
 * Arduino Project (no include Arduino, no setup and no loop). Works best in combination with
 * "SimpleSerial.h"
 *
 * Resource usage (ATmega328P):
 * - USART0, USART_RX_vect, USART_TX_vect the other USART vector is not used but not usable
 * - SER_TX_BUFFER_SIZE + SER_RX_BUFFER_SIZE (default 2 * 16 Bytes) for the buffers
 * - 8 Bytes additional global vars
 *
 * Usage:
 * - For basic usage just include this header and use the function shown in the "BEGIN USER INTERFACE"
 *   Section. Only tested with normal Arduino UNO!
 * - The following macros needs to be defined when you have a custom system:
 *   F_CPU: Frequenzy of the core clock in Hz
 *   SER_TX_BUFFER_SIZE: Size of the transmit buffer in bytes (Needs to be power of 2)
 *   SER_RX_BUFFER_SIZE: Size of the recive buffer in bytes (Needs to be power of 2)
 *   SER_WAIT_FUNCTION: Function to be called when waiting for buffer (Default: sleep)
 *   SER_NO_SSER_CONFIG: Prevent automatic "SimpleSerial.h" configuration
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

// BEGIN OnlySerial
#ifndef ONLYSERIAL_H_
#define ONLYSERIAL_H_

#include <avr/interrupt.h>
#include <util/atomic.h>

//////////////////////////////////////////////////////////////////////////
// BEGIN USER INTERFACE

// Start Serial width default baud rate of 9600
void SerialBegin(void);

// Start Serial width defined baud rate
void SerialBegin_baud(uint16_t baudRate);

// Stop the serial interface
void SerialEnd(void);

// Peek if a byte is available in receive buffer and return the value
int SerialPeek(char* value);

// Write a fixed number of bytes
void SerialWrite(char* pByte, uint16_t count);

// Write until a special value is byte array is reached
void SerialWrite_delim(char* pByte, char delimiter);

// Read fixed amount of bytes
void SerialRead_count(char* pBuffer, uint16_t len);

// Read until a delimiter is reached
uint16_t SerialRead_delim(char* pBuffer, uint16_t lenMax, char delimiter);

// Read until rx buffer is empty
uint16_t SerialRead_all(char* pBuffer, uint16_t lenMax);

// END USER INTREFACE
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN DEFINITIONS

// If F_CPU is not defines assume arduino uno default frequency
#ifndef F_CPU
#define F_CPU 16000000ULL
#define F_CPU_LOCAL
#endif

#include <util/delay.h>

#ifndef SER_TX_BUFFER_SIZE
#define SER_TX_BUFFER_SIZE 16
#endif

#ifndef SER_RX_BUFFER_SIZE
#define SER_RX_BUFFER_SIZE 16
#endif

#ifndef SER_WAIT_FUNCTION
#define SER_WAIT_FUNCTION() _delay_us(10)
#endif


// Buffers for RX and TX
volatile char ser_rxBuffer[SER_RX_BUFFER_SIZE];
volatile char ser_txBuffer[SER_TX_BUFFER_SIZE];

// Read / Write heads
volatile uint16_t ser_txReadHead;
volatile uint16_t ser_txWriteHead;
volatile uint16_t ser_rxReadHead;
volatile uint16_t ser_rxWriteHead;

// END DEFINITIONS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN INTERNAL DECL

// INTERNAL - Put byte to TX buffer
void ser_putByte(char byte);

// INTERNAL - Write byte to hardware
void ser_writeByte();

// INTERNAL - Get byte from RX buffer
void ser_getByte(char* pByte);

// INTERNAL - Read byte from hardware
void ser_readByte();

// END INTERNAL DECL
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN IMPLEMENTATION

void SerialBegin(void){
	// Default base 9600
	SerialBegin_baud(9600);
}

void SerialBegin_baud(uint16_t baudRate){
	// Calculate and set baud rate
	uint16_t ubrrValue = F_CPU / 16 / baudRate - 1;
	UBRR0 = ubrrValue;
	
	// Setup internal pointers
	ser_txReadHead = 0;
	ser_txWriteHead = 0;
	ser_rxReadHead = 0;
	ser_rxWriteHead = 0;
	
	// Setup USART config
	UCSR0A = 0x0;
	
	// Enable all interrupts RX TX;  Enable RX and TX; Charakter size
	UCSR0B = (1U << RXCIE0) | (1U << TXCIE0) | (1U << RXEN0) | (1U << TXEN0);
	
	// Asynchronous USART; Parity disabled; One Stop bit; Charakter word size 8-Bit
	UCSR0C = (1U << UCSZ00) | (1U << UCSZ01);
}

void SerialEnd(void){
	// Clear own configuration
	UCSR0B = 0x0;
	UCSR0C = 0x0;
}

int SerialPeek(char* value) {
	// Check if buffer is empty
	if(ser_rxReadHead == ser_rxWriteHead){
		return 0;
	}
	
	// Set byte
	if(value){
		*value = ser_rxBuffer[ser_rxReadHead];
	}

	// Found a char
	return 1;
}

void SerialWrite(char* pByte, uint16_t count) {
	for(uint16_t i = 0; i < count; i++){
		ser_putByte(pByte[i]);
	}
}


void SerialWrite_delim(char* pByte, char delimiter) {
	do {
		ser_putByte(*pByte);
		pByte++;
	} while(*pByte != delimiter);
}

void SerialRead_count(char* pBuffer, uint16_t len) {
	for(uint16_t i = 0; i < len; i++) {
		ser_getByte(&pBuffer[i]);
	}
}

uint16_t SerialRead_delim(char* pBuffer, uint16_t lenMax, char delim){
	uint16_t i;
	for(i = 0; i < lenMax; i++) {
		// Get char
		ser_getByte(&pBuffer[i]);

		// Return on delimiter
		if(pBuffer[i] == delim){
			return i;
		}
	}

	return lenMax;
}

uint16_t SerialRead_all(char* pBuffer, uint16_t lenMax) {
	uint16_t i;
	for(i = 0; (i < lenMax) && SerialPeek(0); i++) {
		// Get char
		ser_getByte(&pBuffer[i]);
	}

	return i;
}

// Write until a special value is reached
void ser_putByte(char byte){
	uint16_t wHead, rHead;
	
	do{
		// Wait
		SER_WAIT_FUNCTION();
		
		// Get current read and write heads
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			wHead = ser_txWriteHead;
			rHead = ser_txReadHead;// Wait
			SER_WAIT_FUNCTION();
		}
		
	// While write head + 1 == read head
	} while(((int)(wHead + 1) % SER_TX_BUFFER_SIZE) == rHead);
	
	// Put byte
	ser_txBuffer[ser_txWriteHead] = byte;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Increment pointer
		ser_txWriteHead = (wHead + 1) % SER_TX_BUFFER_SIZE;
	
		// Check if put is required
		if(UCSR0A & (1U << UDRE0)){
			ser_writeByte();
		}
	}
}

void ser_readByte(){
	// Read byte
	ser_rxBuffer[ser_rxWriteHead] = UDR0;
	
	// Increment write head
	ser_rxWriteHead = (ser_rxWriteHead + 1) % SER_TX_BUFFER_SIZE;
	
	// Check read head
	if(ser_rxWriteHead == ser_rxReadHead){
		// Increment read head
		ser_rxReadHead = (ser_rxReadHead + 1) % SER_TX_BUFFER_SIZE;
	}
}

void ser_getByte(char* pByte){
	uint16_t wHead, rHead;
	
	do{
		// Wait
		SER_WAIT_FUNCTION();
		
		// Get current read and write heads
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			wHead = ser_rxWriteHead;
			rHead = ser_rxReadHead;
		}
		
		// While write head + 1 == read head
	} while(rHead == wHead);
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		// Get pointer
		*pByte = ser_rxBuffer[rHead];
	
		// Increment pointer
		ser_rxReadHead = (rHead + 1) % SER_RX_BUFFER_SIZE;
	}
}

void ser_writeByte() {
	// If has data
	if (ser_txReadHead != ser_txWriteHead){
		UDR0 = ser_txBuffer[ser_txReadHead];
		
		ser_txReadHead = (ser_txReadHead + 1) % SER_TX_BUFFER_SIZE;
	}
}

// END IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN INTERRUPTS 

ISR(USART_RX_vect) {
	ser_readByte();
}

ISR(USART_TX_vect) {
	ser_writeByte();
}

// END INTERRUPTS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN CLEANUP

// Cleanup F_CPU if set locally
#ifdef F_CPU_LOCAL
#undef F_CPU
#undef F_CPU_LOCAL
#endif

// END CLEANUP
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// BEGIN SIMPLESERIAL PRECONFIG

// Config SimpleSerial macros for easy compatibility
#ifndef SER_NO_SSER_CONFIG
#define SSER_WRITE_COUNT(buf, count) SerialWrite(buf, count)
#define SSER_WRITE_DELIM(buf, delim) SerialWrite_delim(buf, delim)
#define SSER_READ_COUNT(buf, count) SerialRead_count(buf, count)
#define SSER_READ_DELIM(buf, count, delim) SerialRead_delim(buf, count, delim)
#define SSER_PEEK(pChar) SerialPeek(pChar)
#endif

// END SIMPLESERIAL PRECONFIG
//////////////////////////////////////////////////////////////////////////

// END OnlySerial
#endif /* ONLYSERIAL_H_ */

// END CPP Guard
#ifdef __cplusplus
}
#endif 