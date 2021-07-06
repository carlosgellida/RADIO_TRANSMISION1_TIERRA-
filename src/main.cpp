
#include <Arduino.h>
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

String Arr2Str(float Arr1[5]) {
  // This function transform an 5 elements array 
  // into a 5 terms string
  String Arr_transform ; 
  Arr_transform = String(Arr1[0]); 
  for(int i = 1; i < 5; i++){
    Arr_transform += " " ; //Delimitado por espacios
    Arr_transform += String(Arr1[i]) ; 
  }
  Arr_transform += " " ; // Add an aditional space at the end
  return Arr_transform; 
}

// instantiate an object for the nRF24L01 transceiver
RF24 radio(14, 12); // using pin 14 for the CE pin, and pin 12 for the CSN pin

// Let these addresses be used for the pair
uint8_t address[][6] = {"1Node", "2Node"};
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = false;  // true = TX role, false = RX role

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission

float payload[5] ; 

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }

  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));

  radioNumber = true ;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  radio.setPALevel(RF24_PA_MAX) ; 

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);     // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening(); // put radio in RX mode
  }
} // setup

void loop() {

    // This device is a RX node

    uint8_t pipe;
    if (radio.available(&pipe)) {             // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
      radio.read(&payload, bytes);            // fetch payload from FIFO
      Serial.print(F("Received "));
      Serial.print(bytes);                    // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      Serial.print(pipe);                     // print the pipe number
      Serial.print(F(": "));
      Serial.println(Arr2Str(payload));                // print the payload's value
    }

} // loop