#include <Arduino.h>
#include <SPI.h>  
#include "RF24.h"

RF24 myRadio (14, 12);
byte addresses[][6] = {"0"};
//uint8_t addresses[][6] = {"1Node", "2Node"};
const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 10;

float time1, time2; 

float quaternion2[5] ; //= {1, 2, 3, 6, 10}; 
float quaternion[5] = {1, 1.0, 2.0, 3.0, 6.1}; 

//typedef struct package Package;
//Package dataRecieve;
//Package dataTransmit;

void send(bool recieved){
  bool sended = false; 
  
  if(recieved){
  Serial.println("Program is here!"); 
  myRadio.stopListening();
  float time = float(micros()); 
  sended = myRadio.write(&quaternion, sizeof(quaternion)); 
  Serial.print("time to send: "); 
  Serial.println(float(micros()) - time); 
  myRadio.startListening(); 
  }

  if(sended){   
    Serial.println("Transmit: ");
  }

}

bool getData(void){
  bool recieved = false; 
  //Serial.println("Program is listening"); 
  if ( myRadio.available()) {
      myRadio.read(&quaternion2, sizeof(quaternion2) );
      Serial.println("Recieve: ");
      recieved = true; 
  }
  return recieved; 

}

void showData(bool recieved){
  if(recieved){
    Serial.print("mensaje recibido:"); 
    Serial.println(quaternion2[4]);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  bool radioWorks = myRadio.begin(); 
  if(radioWorks){
    Serial.println("Radio is working"); 
  }else{
    Serial.println("Radio doesn't Work");
    while(true){
      //keep device in a loop to avoid problems
    }
  }
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate(RF24_2MBPS); 
  myRadio.openWritingPipe(masterAddress);
  myRadio.openReadingPipe(1, slaveAddress);
  //myRadio.setChannel(115); 
  myRadio.setRetries(3, 5);
  myRadio.startListening();
  prevMillis = millis();//doesnt matter. dont change
}

void loop() {

  bool recieved = getData();
  showData(recieved) ;
  send(recieved) ; 
}