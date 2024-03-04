#include <WiFi.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include <ESPmDNS.h>

#include <stdio.h>
#include <string.h>

#include <MFRC522.h>
#include <SPI.h>

// TODO: retrieve SSID and Password from client on AP MODE
// Save SSID and Password on EPROM to make it auto connect to WiFi without adding those in the code

#define SS_PIN 21
#define RST_PIN 22

#define button 5

#define led 2

MFRC522 rfid(SS_PIN, RST_PIN);

int acionador = 15;
 
const char* ssid = "";
const char* senha =  "";
const char* hostname =  "Sphynx";

const char* ssidAP = "Sphynx-WIFI";
const char* senhaAP = "12345678";

const char* html = R"HTML(
  <!DOCTYPE html>
  <html lang="pt-br">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title> Sphynx - Conexão Wifi </title>

  </head>
  <style>

      *{
          padding: 0;
          margin: 0;
          box-sizing: border-box;
          text-decoration: none;
          list-style: none;
      }

      html{
          font-size: 16px;
          font-family: 'Roboto';
      }

      :root{
          --color-background: #024959;
          --color-background-bright: #026773;
          --color-gray: #D9D9D9;
          --color-submenu: #022534;
      }

      main{
          position: relative;
          display: flex;
          align-items: center;
          justify-content: center;
          padding: 0 30px;
          width: 100%;
          height: 100vh;
          background-color: var(--color-background);
      }

      .container{
          position: relative;
          display: flex;
          align-items: flex-start;
          justify-content: center;
          width: 90%;
          height: 90%;
          background-color: var(--color-background-bright);
          border-radius: 15px;
          overflow-y: scroll;
      }

      table.content-table{
          border-collapse: collapse;
          font-size: 2rem;
          min-width: 90%;
          margin-top: 20px;
      }

      table.content-table thead tr{
          background-color: var(--color-background);
          color: white;
          text-align: center;
          font-weight: bold;
      }

      table.content-table th, table.content-table td {
          padding: 12px 15px;
          color: white;
      }

      table.content-table th, table.content-table td a{
          color: white;
          transition: .5s;
      }

      .correct:hover{
          color: green;
      }

      .incorrect:hover{
          color: red;
      }

      table.content-table tbody tr{
          border-bottom: 1px solid var(--color-background);
          text-align: center;
      }
  </style>
  <body>
      <header></header>
      <main>
          <div class="container">
              <table class="content-table">
                  <thead>
                      <tr>
                          <th> Nome </th>
                          <th> Força do Sinal </th>
                          <th> Conectar </th>
                          <th> Desconectar </th>
                      </tr>
                  </thead>
                  <tbody>)HTML";

const char* htmlFooter = R"footer(                </tbody>
            </table>
        </div>
    </main>
    <footer></footer>
    </body>
    </html>)footer";


String message;
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiServer serverAP(80);
WiFiClient clientAtual;
bool apServerUp = false;

 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
    ws.textAll("$2a$12$X9my8HHbMJYk6y04FnR6ie1B/WnLOlBAeEMRhEOvt.8z/OmOR6kLS");
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

void setupWiFi(WiFiClient client) {
  // WiFi config Web Server
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println(html);
  int numRedes = WiFi.scanNetworks();
  for (int i = 0; i < numRedes; ++i) {
    client.println("                    <tr>");
    client.print("                        <td>");
    client.print(WiFi.SSID(i));
    client.println("</td>");
    client.print("                        <td>");
    client.print(WiFi.RSSI(i));
    client.println(" dBm</td>");
    client.println("                      <td>");
    client.println("<a class=\"correct\" href=\"\"> <i class=\"fa-solid fa-square-check\"></i> </a>");
    client.println("</td>");
    client.println("                      <td>");
    client.println("<a class=\"incorrect\" href=\"\"><i class=\"fa-solid fa-circle-xmark\"></i></a>");
    client.println("</td>");
    client.println("                   </tr>");
  }
  client.println(htmlFooter);
}

void Sphynx(){
  // Sphynx default operation
  SPI.begin();
  rfid.PCD_Init();

  pinMode(acionador, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
 
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.getHostname());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();
}

void setup(){
  Serial.begin(115200);

  size_t ssidSize = strlen(ssid);
  size_t senhaSize = strlen(senha);

  Serial.println("tamanho ssid: "+ssidSize);
  Serial.println("tamanho senha: "+senhaSize);

  if (ssidSize < 1 && senhaSize < 8){
    // Begins AP MODE for WIFI config
    // only if ssid and Password Size doesnt match for now
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Configurando Acess Point...");
      WiFi.softAP(ssidAP, senhaAP);

      IPAddress IP = WiFi.softAPIP();
      Serial.print("Endereco IP do AP: ");
      Serial.println(IP);

      serverAP.begin(); // Starts AP Mode server. Connects to 192.168.4.1 or sphynx-dev.local
      apServerUp = true;

      if (!MDNS.begin("Sphynx-dev")) {
        Serial.println("Erro ao iniciar mDNS");
        return;
      };
    }
  }else{
    WiFi.begin(ssid, senha, 6);
    // Tries to connect to the specified wifi
    while (WiFi.status() != WL_CONNECTED){
      delay(1000);
      Serial.println("Esperando conexão wifi...");
    }
    apServerUp = false;
    // Begins Sphynx default operation
    Sphynx();
  }

}

void loop(){
  if (apServerUp){
    // If AP Mode is on, run this loop to handle web request from clients
    Serial.println("Server is up!");
    WiFiClient client = serverAP.available();
    if (client) {
      Serial.println("Novo Client.");
      clientAtual = client;
      while (client.connected()) {
        if (client.available()) {
            setupWiFi(clientAtual);
            break;
            }
      }
      client.stop();
      Serial.println("Client Desconectado..");
    }
    delay(500);
  }
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
