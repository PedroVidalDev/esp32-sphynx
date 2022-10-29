#include <WiFi.h>
#include "ESPAsyncWebServer.h"

#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 21
#define RST_PIN 22
#define ID "6A A9 B0 A3"

MFRC522 rfid(SS_PIN, RST_PIN);
int acionador = 15;
 
const char* ssid = "example-ssid";
const char* password =  "example-password";
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
      Serial.println("Websocket client connection received");}      
   
  else if(type == WS_EVT_DISCONNECT){
      Serial.println("Client disconnected");
  }
}
 
void setup(){
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(acionador, OUTPUT);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();
}
 
void loop(){
  if (! rfid.PICC_IsNewCardPresent()){
    return;
  }
    
  if (! rfid.PICC_ReadCardSerial()){
    return;
  }
      
  String id_cartao = "";
  byte i;
        
  for (byte i = 0; i < rfid.uid.size; i++){
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
    id_cartao.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id_cartao.concat(String(rfid.uid.uidByte[i], HEX));
  }
    
  id_cartao.toUpperCase();
    
  if(id_cartao.substring(1) == ID){
    Serial.println("Acesso liberado");
    digitalWrite(acionador, HIGH);
    ws.textAll(id_cartao.substring(1));
  }
      
  else{
    Serial.println("Acesso Negado");
    ws.textAll(id_cartao.substring(1));
  }
  delay(1000);
}
