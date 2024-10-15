/* Serial 7-Segment Display Example Code
    SPI Mode Stopwatch
   by: Jim Lindblom
     SparkFun Electronics
   date: November 27, 2012
   license: This code is public domain.

   This example code shows how you could use the Arduino SPI 
   library to interface with a Serial 7-Segment Display.

   There are example functions for setting the display's
   brightness, decimals and clearing the display.

   The SPI.transfer() function is used to send a byte of the
   SPI wires. Notice that each SPI transfer(s) is prefaced by
   writing the SS pin LOW and closed by writing it HIGH.

   Each of the custom functions handle the ssPin writes as well
   as the SPI.transfer()'s.

   There's a custom function used to send a sequence of bytes
   over SPI - s7sSendStringSPI, which can be used somewhat like
   the serial print statements.
*/
#include <SPI.h>

const int csPin = 7; //The CS pin of J1 is A0

char tempString[10];

void setup()
{
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  
  clearDisplaySPI();

  setBrightnessSPI(255);  // High brightness
  delay(10);
  s7sSendStringSPI("1234");
}

void loop()
{
  delay(1000);
  s7sSendStringSPI("1234");
}

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringSPI(String toSend)
{
  digitalWrite(csPin, LOW);
  for (int i=0; i<4; i++)
  {
    SPI.transfer(toSend[i]);
  }
  digitalWrite(csPin, HIGH);
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplaySPI()
{
  digitalWrite(csPin, LOW);
  SPI.transfer(0x76);  // Clear display command
  digitalWrite(csPin, HIGH);
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightnessSPI(byte value)
{
  digitalWrite(csPin, LOW);
  SPI.transfer(0x7A);  // Set brightness command byte
  SPI.transfer(value);  // brightness data byte
  digitalWrite(csPin, HIGH);
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimalsSPI(byte decimals)
{
  digitalWrite(csPin, LOW);
  SPI.transfer(0x77);
  SPI.transfer(decimals);
  digitalWrite(csPin, HIGH);
}
