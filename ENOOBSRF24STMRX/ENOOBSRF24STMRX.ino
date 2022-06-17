/* Receiver code for the STM32 + NRF24
 * Install the NRF24 library to your IDE
 * Upload this code to the STM32F103C series
 * Connect a NRF24 module to it: 
 * 
    Module // STM32
    
    GND    ->   GND
    Vcc    ->   3.3V
    CE     ->   PA4
    CSN    ->   PB0
    CLK    ->   PA5
    MOSI   ->   PA7
    MISO   ->   PA6

This code receive 1 channels and prints the value on the serial monitor
Please, like share and subscribe : https://www.youtube.com/c/ELECTRONOOBS
*/

#include <SPI.h>       
#include <nRF24L01.h>       //Download here: https://electronoobs.com/eng_arduino_NRF24_lib.php
#include <RF24.h>

#define PACKET_SIZE 32

//Remember that this code is the same as in the transmitter
const uint64_t pipeIn = 0xE8E8F0F0E1LL;     
RF24 radio(7, 8); // PB0 - CE, PA4 - CSN on Blue Pill 

uint8_t data[PACKET_SIZE];
uint8_t imgSize;

/**************************************************/

void setup()
{
  Serial.begin(115200);  
  //We reset the received values

  Serial.println("start receiver...");
 
  //Once again, begin and radio configuration
  radio.begin();
  radio.setAutoAck(true);
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);  
  radio.openReadingPipe(1,pipeIn);
  
  //We start the radio comunication
  radio.startListening();
}

/**************************************************/

unsigned long last_Time = 0;

//We create the function that will read the data each certain time
void receive_the_data()
{
//  while ( radio.available() ) {
//    radio.read(&receivedPacket, sizeof(receivedPacket));
//
//    for(int i = 0 ; i++; i < 20){
//      Serial.print(receivedPacket.bytes[i]);  
//      Serial.print(",");
//    }
//    Serial.println();
//  }

  while ( radio.available() ) {
    radio.read(&data, sizeof(data));

    //for images up to 65kB
    if(data[3] == 0 && data[4] == 0 && data[5] == 0){
      imgSize = data[0] * 256 + data[1];

      Serial.print("received size packet: ");
      Serial.print(imgSize);
      Serial.print(" ");
      Serial.print(data[0]);
      Serial.print(" ");
      Serial.print(data[1]);
      Serial.println(" bytes");

      return;
    }

    for(int i = 0 ; i<PACKET_SIZE; i++){
//      if(data[i] < 16){
//        Serial.print('0');
//      }
      Serial.write(data[i]);
//      Serial.print(",");
    }

    Serial.println();
  }
}

/**************************************************/

void loop()
{
  //Receive the radio data
  receive_the_data();

//  Serial.println("received size packet: ");
//  
//  for( int i = 0; i <50; i++)
//    Serial.write(i);
//
//  Serial.write("\n");
//
//  delay(1500);

 }//Loop end
