#include <Arduino.h>
#include <SPI.h>  
#include "RF24.h"

RF24 myRadio (14, 12);
//byte addresses[][6] = {"0"};
//uint8_t addresses[][6] = {"1Node", "2Node"};
const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 10;//doesnt matter. dont change

float time1, time2; 

float quaternion2[5] = {1, 2, 3, 6, 10}; 
float quaternion[5] ;

void send(void){
  bool sended = false; 
  myRadio.stopListening();
  sended = myRadio.write(&quaternion2, sizeof(quaternion2));
  Serial.print("currnt time: "); 
  Serial.println(float(micros())); 
  myRadio.startListening(); 

    if(sended){   
      Serial.println("Transmit: ");
    }
}

bool getData(void){
  //Serial.println("Program is listening");
  //myRadio.startListening();
  bool recieved = false; 
  if ( myRadio.available()) {
    Serial.println("Mesaje available!!!"); 
    myRadio.read( &quaternion, sizeof(quaternion) );
    recieved = true; 
  } 
  return recieved; 
}

void showData(bool recieved){
  if(recieved){
    Serial.print("mensaje recibido: "); 
    Serial.println(quaternion[4]); 
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
  myRadio.setPALevel(RF24_PA_MAX); //Probar con menor potencia
  myRadio.setDataRate(RF24_2MBPS); 
  myRadio.openWritingPipe(slaveAddress);
  myRadio.openReadingPipe(1, masterAddress);
  //myRadio.setChannel(115); 	
  myRadio.setRetries(3,5);

  //myRadio.setRetries(3,5);
  myRadio.startListening();
  prevMillis = millis();//doesnt matter. dont change
  send(); 
}

void loop() {

  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis) {
    prevMillis = millis();
    send();
  }else

  bool recieved = getData();

  showData(recieved) ;

}