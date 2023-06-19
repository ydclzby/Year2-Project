#include <math.h>
#include <ctype.h>
#include <HardwareSerial.h>

#define UART_BAUDRATE 115200
#define RXD2 16 // FPGA side : (ARDUNIO_IO[9])
#define TXD2 17 // FPGA side : (ARDUNIO_IO[8])

HardwareSerial Fpga(2);

unsigned long start = 0;
unsigned long period = 500;

int byte2int(byte* buf, int size){
  int val = 0;
  for (int i=(size-1); i>=0; i--){
    val += buf[i] << (8*i);
  }
  return val;
}


void setup() {
  Fpga.begin (115200, SERIAL_8N1, RXD2, TXD2);
  Serial.begin(115200);
  while (!Serial);

  Serial.println("ESP32 UART Receiver");

}


void loop() {
  static bool newLine = true; // Flag to indicate whether to print a new line
  static int counter = 0; // Counter for printing values in a line
  
  if (Fpga.available() >= 4) {
    byte buf[4];
    Fpga.readBytes(buf, 4);

    if (newLine) {
      Serial.print("Received data: ");
      newLine = false;
    }
    int a = byte2int(buf, 4);
    
    if(a == 0x00524242){
      Serial.print("red ");
    } 
    else if(a == 0x00424242){
      Serial.print("blue ");
    }
    else if(a == 0x00594242){
      Serial.print("yellow ");
    }
    else if(a == 0x0057574d){
      Serial.print("white ");
    }
    else {
      Serial.print(a, HEX);
      Serial.print(" ");
    } 

    counter++;
    if (counter == 8) {
      Serial.println();
      counter = 0;
      newLine = true;
    }
  }
}
