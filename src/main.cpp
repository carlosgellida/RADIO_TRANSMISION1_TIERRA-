#include <Arduino.h>
#include <SPI.h>  
#include "RF24.h"
#include <Wire.h>

// Los siguientes archivos contienen las funciones personalizadas
// exclusivas para este programa

#include <MPU_personalized_functions.h> 
#include <Wifi_personalized_functions.h>
#include <Read_buttons.h> 

WiFiClient client;

RF24 myRadio (14, 12);
const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 10;

float time1, time2; 

Quaternion currentQuat ;
Quaternion desiredQuat ; 

bool test = true; 

// Declare struct with info of buttons
struct Buttons buttons; 

Quaternion getDesiredQuat(void){
  // Dumy function thats simulates to estimate desired quaternion
  Quaternion quaternion ; 
  quaternion.qw = 1 ; 
  quaternion.qx = 0 ; 
  quaternion.qy = 0 ; 
  quaternion.qz = 0 ; 
  return quaternion ; 
}

struct Quaternion getDesiredQuat2(){
  struct Quaternion desiredQuat;
  Matrix<3, 1> m = {0, 0, 0}; 
  Matrix<3, 1> z = {0, 0, 1}; 
  Matrix<3, 1> u ;
  m(0, 0) = buttons.Lx; 
  m(1, 0) = buttons.Ly; 
  //Serial << "m: " << m << "\n";
  u = escProd(-1, crossProduct(m, z)) ; 
  //Serial << "u: " << u << "\n";
  float normU = norm2(u); 
  u = escProd(1/normU, u); // Normalization of u vector
  float c = 0.06; // Constante de proporcionalidad
  float theta = c*normU ; // Angle
  desiredQuat = arr2Struc(arrMod(axisAng2Quat(theta, u)) );
  //desiredQuat = arr2Struc(quatProduct(giro90Y(), arrMod(axisAng2Quat(theta, u)) ));
  return desiredQuat; 
}

void getAndSendInfo()
{
  //Declare a string where data is saved
  String output;

  //Resetar string output
  output = "";

  //Resetear documento JSON
  jsonDocTx.clear();

  //Almacenar info del quaternio en documento JSON
  jsonDocTx["QW"] = currentQuat.qw ; 
  jsonDocTx["QX"] = currentQuat.qx ; 
  jsonDocTx["QY"] = currentQuat.qy ; 
  jsonDocTx["QZ"] = currentQuat.qz ; 

  jsonDocTx["Lx"] = buttons.Lx ; // Agregar lectura de botones al archivo JSON
  jsonDocTx["Ly"] = buttons.Ly ; 
  jsonDocTx["Lw"] = buttons.Lw ; 
  jsonDocTx["Rx"] = buttons.Rx ; 
  jsonDocTx["Ry"] = buttons.Ry ; 
  jsonDocTx["Rw"] = buttons.Rw ; 

  jsonDocTx["dQW"] = desiredQuat.qw; 
  jsonDocTx["dQX"] = desiredQuat.qx; 
  jsonDocTx["dQY"] = desiredQuat.qy; 
  jsonDocTx["dQZ"] = desiredQuat.qz; 

  //Convertir documento JSON en un string
  serializeJson(jsonDocTx, output);

  //Enviar string con info del documento JSON
  webSocket.sendTXT(output);  
}

void taskWifi(void)
{
  // Esta función inicia la conexión WIFI, y se conecta a un 
  // servidor Web Socket sin usar internet, el NodeMCU se 
  // comporta como un punto de acceso suave

  const char* ssid = "ESP32_mando";
  const char* password =  "123456789";

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  while(true){
    Serial.println("Waiting for server conection..."); 
    int isConnected = WiFi.softAPgetStationNum(); 
    if(isConnected > 0){
      break ; 
    }
    delay(3000) ; 
  }

  delay(2000) ; 

  webSocket.begin("192.168.4.2", 8001, "/"); 

  Serial.println("Websocket has begin"); 

  webSocket.onEvent(webSocketEvent) ;

  webSocket.setReconnectInterval(50);  
}

void foundIP(void){
  // Esta función busca automáticamente el servidor disponible en el puerto
  // 5000 de varias IP's para localizar la IP del PC servidor
  IPAddress server(192, 168, 0, 2); 
  String iPdelPC = ""; 
  for(int i=0; i < 256; i++){
    bool connectedServer = false; 
    for(int j=2; j < 25; j++){
      iPdelPC = "192.168." + String(i) + "." + String(j) ; 
      server[2] = i; 
      server[3] = j; 
      if (client.connect(server, 5000)) {
        Serial.print("IP of PC: "); 
        Serial.println(server); 
        Serial.println("connected to server");
        connectedServer = true; 
        break;
      }
      Serial.println("Please wait, looking for the PC IP");
      delay(10) ; 
    }
    if(connectedServer){
      break ; 
    }
  }

  webSocket.begin(server, 8001, "/"); 
  
  webSocket.onEvent(webSocketEvent) ;
  webSocket.setReconnectInterval(50); 

  Serial.println("Websocket has begin"); 
}

void taskWifiInternet(void)
{
  // Esta función inicia la conexión WIFI, y se conecta a un 
  // servidor Web Socket, esta función hace uso de un modem

  char ssid[] = "IZZI-2124" ; 
  char pass[] = "ECA9404C2124" ;
  int status = WL_IDLE_STATUS;     // the Wifi radio's status
  //int WL_CONNECTED = 0; 

  while (status != 6) {

    Serial.print("Attempting to connect to WPA SSID: ");
    status = WiFi.begin(ssid, pass);
    delay(3000);

  }

  Serial.print("You're connected to the network");

  printWifiData();

  Serial.println(WiFi.localIP());

  delay(2000) ; 

  foundIP(); 
}

void send(Quaternion desiredQuat){
    myRadio.stopListening();
    myRadio.write(&desiredQuat, sizeof(desiredQuat));
    myRadio.startListening(); 
}

bool getData(void){
  bool recieved = false; 
  //Serial.println("listenning"); 
  if ( myRadio.available()) {
      myRadio.read(&currentQuat, sizeof(currentQuat) );
      /*Serial.print("currentQuat: "); 
      printQuat(currentQuat); */
      recieved = true; 
  }
  return recieved; 

}

/*void sendDataToWiFi(Buttons buttons, Quaternion desiredQuat){
    webSocket.loop();
    getAndSendInfo(buttons, desiredQuat) ; 
}*/

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializar I2C
  Wire.begin(); 

  delay(500) ; 

  // Inicializar WiFi y WebSocket
  //taskWifi() ; // Cambiar para utilizar sin modem
  taskWifiInternet(); 

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
  myRadio.setRetries(1, 3);
  myRadio.powerUp() ; 
  myRadio.startListening();
  
  Serial.println("Data transmision begins");
  prevMillis = millis();
  
}

void loop() {
  if(getData()){
    // get analog reads for joystick buttons
    buttons = AnalogJoystick() ; // Obtener lectura de botones

    // Calculation of desired Quaternion using the buttons

    desiredQuat = getDesiredQuat2(); 
    //printQuat(desiredQuat); 
    send(desiredQuat) ; // Send back the desired quaternion

    webSocket.loop();
    getAndSendInfo() ; // Send all the info to the computer
  }

}