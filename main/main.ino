#include <WiFi.h>
#include "ESPAsyncWebServer.h"

#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 21
#define RST_PIN 22

#define led 2

MFRC522 rfid(SS_PIN, RST_PIN);
int acionador = 15;
 
const char* ssid = "Vidals";
const char* password =  "46421148";

String message;
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
    ws.textAll("abcd");    
  }      
   
  else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
  }

  else if(type == WS_EVT_DATA){
    handleWebSocketMessage(arg, data, len);
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    controlDoor(message);
  }
}
 
void controlDoor(String message){
  if(message == "true"){
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    Serial.println("Poggers");
  }
  else if(message == "false"){
    Serial.println("Noggers");
  }
}
 
void setup(){
  Serial.begin(115200);

  pinMode(led, OUTPUT);

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
  ws.textAll(id_cartao.substring(1));

  delay(500);
}
