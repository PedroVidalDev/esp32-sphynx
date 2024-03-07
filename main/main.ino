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

String html = R"HTML(
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
          scrollbar-gutter: stable;
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
          cursor: pointer;
          transition: .5s;
      }

      .correct:hover{
          color:aquamarine;
      }

      table.content-table tbody tr{
          border-bottom: 1px solid var(--color-background);
          text-align: center;
      }

      .container::-webkit-scrollbar{
          width: 10px;
      }

      .container::-webkit-scrollbar-track {
          border-radius: 10px;
          background-color: gray;
      }
      
      .container::-webkit-scrollbar-thumb {
          background: #fff; 
          border-radius: 10px;
      }

      .conn-container{
          position: absolute;
          display: none;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          width: 90%;
          height: 90%;
          background-color: #022534af;
          border-radius: 15px;
          backdrop-filter: blur(10px);
          z-index: 99;
      }

      .conn-container form{
          position: relative;
          display: flex;
          flex-direction: column;
          width: 100%;
          height: 100%;
          align-items: center;
          justify-content: center;
      }

      .conn-container form label{
          color: white;
          font-size: 20px;
      }

      .password{
          border: 0;
          border-radius: 10px;
          width: 40%;
          height: 30px;
          font-size: 20px;
          padding: 10px;
      }

      .password:focus-visible{
          outline: none;
      }

      .submit{
          position: relative;
          margin-top: 20px;
          width: 20%;
          height: 30px;
          border-radius: 10px;
          border: 0;
          cursor: pointer;
      }

      .conn-exit{
          position: relative;
          margin-bottom: 30px;
          width: 20%;
          height: 30px;
          border: 0;
          border-radius: 15px;
          cursor: pointer;
          transition: .5s;
      }

      .conn-exit:hover{
          background-color: #022534;
          color: white;
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
                          <th> Força do sinal </th>
                          <th> Conectar </th>
                      </tr>
                  </thead>
                  <tbody>)HTML";

String htmlFooter = R"footer(</tbody>
  </table>
      </div>
      <div class="conn-container">
          <form action="#" method="POST">
              <label class="label" for="password">Digite a senha:</label>
              <input class="password" type="password" name="password">
              <input class="submit" type="submit" value="Send">
          </form>
          <button class="conn-exit"> Sair </button>
      </div>
  </main>
  <footer></footer>
  </body>
  <script>
    const buttons = document.querySelectorAll(".correct");
    const widget = document.querySelector(".conn-container");
    const label = document.querySelector(".label");
    const buttonExit = document.querySelector(".conn-exit")

    buttons.forEach(element => {
        element.addEventListener("click", () => {
            const ssid = element.parentNode.parentNode.querySelectorAll("td")[0].textContent;
            
            widget.style.display = "flex";
            label.textContent = "Digite a senha da rede " + ssid + ":"
        })
    });

    buttonExit.addEventListener("click", () => {
        widget.style.display = "none";
    })
  </script>
  </html>)footer";

String indexHtml;
String message;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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

void setupWiFi() {
  // WiFi config Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   String linhas = ""; 
   if(WiFi.scanComplete() == -2){
    WiFi.scanNetworks(true);
    linhas += ("<tr><td>Procurando</td><td>Procurando</td><td> <a class='correct'>Procurando</a> </tr>");
   }else if (WiFi.scanComplete() == -1){
    linhas += ("<tr><td>Procurando</td><td>Procurando</td><td> <a class='correct'>Procurando</a> </tr>");
   }else{
    Serial.print("Encontrado ");
    Serial.print(WiFi.scanComplete());
    Serial.println(" redes WiFi na area");
    for (int i = 0; i < WiFi.scanComplete(); ++i) {
      linhas += ("<tr><td>" + WiFi.SSID(i) + "</td><td>" + WiFi.RSSI(i) + "dBm</td><td> <a class='correct'> Connect </a> </tr>");
    }
    WiFi.scanDelete();
  }
  indexHtml = html + linhas + htmlFooter;
  linhas = "";
  request->send_P(200, "text/html", indexHtml.c_str());
  });
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
 
  server.on("/conectar", HTTP_GET, [](AsyncWebServerRequest * request) {
  request->send(200, "text/plain", "Clyio - Sphynx");
  });
}

void setup(){
  Serial.begin(115200);

  size_t ssidSize = strlen(ssid);
  size_t senhaSize = strlen(senha);

  if (ssidSize < 1 && senhaSize < 8){
    // Begins AP MODE for WIFI config
    // only if ssid and Password Size doesnt match for now
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Configurando Acess Point...");
      WiFi.softAP(ssidAP, senhaAP);

      IPAddress IP = WiFi.softAPIP();
      Serial.print("Endereco IP do AP: ");
      Serial.println(IP);

      if (!MDNS.begin("Sphynx-dev")) {
        Serial.println("Erro ao iniciar mDNS");
        return;
      };

      server.begin();

      setupWiFi(); //Starts AP Mode server. Connects to 192.168.4.1 or sphynx-dev.local
    }
  }else{
    WiFi.begin(ssid, senha, 6);
    // Tries to connect to the specified wifi
    while (WiFi.status() != WL_CONNECTED){
      delay(1000);
      Serial.println("Esperando conexão wifi...");
    }
    // Begins Sphynx default operation
    Sphynx();
  }

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
