#include <WiFi.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"

#include <stdio.h>
#include <string.h>

#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 21
#define RST_PIN 22

#define button 5

#define led 2

uint8_t New_MAC_Address[] = {0x10, 0xAA, 0xBB, 0xCC, 0x33};

MFRC522 rfid(SS_PIN, RST_PIN);

int acionador = 15;
 
const char* ssid = "Vidals";
const char* password =  "46421148";
const char* hostname =  "pog";

String message;
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
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
    digitalWrite(led, !digitalRead(led));
    delay(1000);    
    digitalWrite(led, !digitalRead(led));
  }
  else if(message == "false"){
    digitalWrite(led, !digitalRead(led));
    delay(500);
    digitalWrite(led, !digitalRead(led));  
    delay(500);
    digitalWrite(led, !digitalRead(led));  
    delay(500);
    digitalWrite(led, !digitalRead(led));  
  }
}
 
void setup(){
  Serial.begin(115200);
  Serial.println(WiFi.getHostname());
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

  WiFi.setHostname(hostname);
  
  WiFi.mode(WIFI_STA);

  esp_wifi_set_mac(WIFI_IF_STA, New_MAC_Address);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(acionador, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
 
  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.getHostname());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();
}


void loop(){
  /*if (! rfid.PICC_IsNewCardPresent()){
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
  Serial.println(id_cartao.substring(1));

  if(digitalRead(button) == 0){
    return;
  }

  else{
    ws.textAll("abc");
  }*/
  

  delay(500);
}
