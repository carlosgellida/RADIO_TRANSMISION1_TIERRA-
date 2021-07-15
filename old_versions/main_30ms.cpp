#include <Arduino.h>
#include <SPI.h>  
#include "RF24.h"

RF24 myRadio (14, 12);
byte addresses[][6] = {"0"};
//uint8_t addresses[][6] = {"1Node", "2Node"};

struct package {
  int id = 1;
  float temperature = 18.3;
  char  text[300] = "Text to be transmit";
  int recibido = 0; 
};

float time1, time2; 

float quaternion2[5] = {1, 2, 3, 6, 10}; 
float quaternion[5] = {1, 1.0, 2.0, 3.0, 6.1}; 

typedef struct package Package;
Package dataRecieve;
Package dataTransmit;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  myRadio.begin();  
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate(RF24_1MBPS); 

}

void loop() {

  bool recieved = false; 
  myRadio.openReadingPipe(1, addresses[0]) ; 
  myRadio.startListening();

  time1 = float(millis()); 
  quaternion[0] = 0; 

  while(!recieved){
  if ( myRadio.available()) {
    while (myRadio.available()){
      myRadio.read( &quaternion, sizeof(quaternion) );
      if(quaternion[0] == 1){
        break; 
      }
    }
    Serial.println("Recieve: ");
    recieved = true; 
  } else{
     time2 = float(millis()); 
     if(time2 - time1 > 30){
       Serial.println("--------- Data Lost -------");
       break; 
     }
   }
  }

  /*while(recieved == false){
    if ( myRadio.available()) {
      myRadio.read( &quaternion, sizeof(quaternion) );
      Serial.println("Recieve: ");
      recieved = true; 
    } 
  }*/
  

  delay(1); 
  
  bool sended = false; 

  myRadio.closeReadingPipe(1); ////
  myRadio.begin();  

  myRadio.stopListening();
  myRadio.openWritingPipe(addresses[0]);

  sended = myRadio.writeFast(&quaternion2, sizeof(quaternion2));

    if(sended){   
      Serial.println("Transmit: ");
    }
  delay(1) ;   
}