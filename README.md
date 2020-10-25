# UnoC-Easy
Lightweight header only C libraries for the Arduino UNO board. Used when you write in pure C (Without the Arduino libraries)

UnoC-Easy is a combination of header only libraries witch help you get started programming the Arduino UNO in pure C. 

All headers can work on their own but some work automatically without any configuration if the parent header is included first (Including OnlySerial.h will automatically configure SimpleSerial.h).

Currently supported:

- USART0 Basic buffer IO (OnlySerial.h)
- USART0 String Message IO (SimpleSerial.h)

The goal of the library is not to provide you with any feature Arduino provides you. The goal is to provide a hardware abstraction layer on top of the communication interfaces. All other parts of the ATmega328P are strait forward and are currently not planed to be included. 



## Using OnlySerial.h

OnlySerial uses the USART0 of the ATmega to communicate with a other device. The baud rate can be freely selected but the Formate of the message is fixed (But can be changed by editing the header) :

| Setting             | Value              |
| ------------------- | ------------------ |
| USART Type          | Asynchronous USART |
| Character word size | 8-Bit              |
| Stop Bits           | 1                  |
| Parity              | Disabled           |

Setting the following macros using `#define MARCO Value` will change the following behaviour:

| Macro              | Usage                                      | Effect                                                       |
| ------------------ | ------------------------------------------ | ------------------------------------------------------------ |
| F_CPU              | `#define F_CPU <FREQ_IN_HZ>`               | Changes the frequency of the CPU clock                       |
| SER_TX_BUFFER_SIZE | `#define SER_TX_BUFFER_SIZE <Bytes>`       | Changesthe size of the TX (Transmit) buffer. Should be power of two (2, 4, 8, 16, 32, 64, ...) |
| SER_RX_BUFFER_SIZE | `#define SER_RX_BUFFER_SIZE <Bytes>`       | Changesthe size of the RX (Recive) buffer. Should be power of two (2, 4, 8, 16, 32, 64, ...) |
| SER_WAIT_FUNCTION  | `#define SER_WAIT_FUNCTION() myDelay(100)` | Changes the custom delay function used when the TX buffer is full but the user wants to send more data. |
| SER_NO_SSER_CONFIG | `#define SER_NO_SSER_CONFIG`               | Disables the configuration of SimpleSerial.h                 |

### Enabling the Serial Interface / Setting the baud rate

When opening the serial interface it is possible to set a baud rate. If you want to use the default 9600 you can call just call the simpler function.

```c
// Initialize the serial interface with a baud rate of 9600
void SerialBegin(void);

// Initialize the serial interface with a user defined baud rate
void SerialBegin_baud(uint16_t baudRate);
```

If you want to disable the serial interface at a specific point you can use SerialEnd

```c
// Disabled the serial interface
void SerialEnd(void);
```

### Sending data

Sending data can be done by a fixed size or until a special value / char is reached (Usefull for null terminated strings).

```c
// Write data from pBytes and count - 1 following bytes
void SerialWrite(char* pByte, uint16_t count);

// Write data from pByte and following until the value delimiter is reached
void SerialWrite_delim(char* pByte, char delimiter);
```

### Reciving data

Reading data can be done by specifing the number of bytes to read. It is also possible to read all serial data until a specific value is reached. 

```c
// Reads from the serrial input to pBuffer and len bytes
void SerialRead_count(char* pBuffer, uint16_t len);

// Read from the serial to pBuffer until lenMax is reached or the serial value was delimiter. Returns number of bytes read
uint16_t SerialRead_delim(char* pBuffer, uint16_t lenMax, char delimiter);
```

The functions above will block until their end condition is met. To overcome this limitation a peak function is provided witch can be used to check (and optional set) for a next byte.

```c
// If the at least one byte can be read from the serial interface this function returns true. If value is not 0 the target of the poitnter will be set to the next serial value availibe without removing it from the buffer
int SerialPeek(char* value);
```

There is also combination of SerialRead_count and SerialPeek availible witch reads all bytes from the serial interface. 

```c
// Read until the recive buffer is empty or lenMax is reached. Returns number of bytes read
uint16_t SerialRead_all(char* pBuffer, uint16_t lenMax);
```



## Using SimpleSerial.h

SimpleSerial is an extention to OnlySerial providing several Arduino like function to print strings and numbers formated and unformated. It is important to include SimpleSerial after OnlySerial since it depends on macros set by OnlySerial!

### Printing Raw Data

```c
// Prints a string
void SerialPrintStr(char* input);

// Prints a string and sends a new line controle charakter
void SerialPrintLn(char* input);

// Prints a charakter
void SerialPrintChar(char input);

// Sends a new line controle charakter
void SerialPrintNewLine(void);

// Prints a signed integer
void SerialPrintInt(int16_t input);

// Prints a unsigned integer
void SerialPrintUInt(uint16_t input);

// Prints a float
void SerialPrintFloat(double input);
```

### Formating a byte

```c
// Prints the HEX representation of a byte (Format: 0x??)
void SerialPrintHex(char byte);

// Prints the binary representation of a byte (Format: 0b????????)
void SerialPrintBin(char byte);
```

You can print 16-Bit values using the HI-/LOWORD macros. Example:

```c
uint16_t value = 0xABCD;

SerialPrintHex(HIWORD(value));
SerialPrintStr(" ");
SerialPrintHex(LOWORD(value));

// Output: 0xAB 0xCD
```

### Reading Serial Data

```c
// Reads a whole null terminated string or maxRead bytes. Returns number of bytes read (Can be zero if no data is availible) 
int SerialReadStr(char* pChar, uint16_t maxRead);

// Reads a line (until '\n' is reached) or maxRead bytes. Returns number of bytes read (Can be zero if no data is availible)
int SerialReadLine(char* pChar, uint16_t maxRead);

// Reads as long as data is availible or maxRead bytes. Returns number of bytes read (Can be zero if no data is availible)
int SerialReadAll(char* pChar, uint16_t maxRead);
```

