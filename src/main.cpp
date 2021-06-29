#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiClientSecureBearSSL.h"
#include <ESP8266WebServer.h>

const char* wifiName = "master";
const char* wifiPass = "dash-pay-machine";

IPAddress local_IP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

bool slave = false;

float prezzo_prodotto_dash;

WiFiClientSecure client;
ESP8266WebServer server(80);
HTTPClient http; //Object of class HTTPClient
DynamicJsonDocument doc(1024);

void carica_credito() {
  digitalWrite(4, HIGH); 
  delay(1000);
  digitalWrite(4, LOW);
  Serial.println("Credito caricato!");
}

void check_pagamento(String ind) {
  bool esito_pagamento = false;
  Serial.println("Avvio controllo pagamento...");
  String host = "https://chain.so/api/v2/get_address_received/Dash/"+ind;
  int check_num = 0;
  while (esito_pagamento == false && check_num < 3){
    client.setInsecure(); //the magic line, use with caution
    client.connect(host, 443);
    http.begin(client, host);
    if (http.GET() == HTTP_CODE_OK)
    {    
      String payload = http.getString(); 
      // Serial.println(payload);
      deserializeJson(doc, payload);
      JsonObject obj = doc.as<JsonObject>();
      String data = obj["data"]["unconfirmed_received_value"];
      float pay = data.toFloat();
      Serial.print("Ricevuto pagamento di ");
      Serial.print(pay,6);
      Serial.println(" Dash"); 
      http.end(); //Close connection
      if (pay >= (prezzo_prodotto_dash)) {
        carica_credito();
        return;
      }      
    }
    check_num ++;
  }
  Serial.println("Sessione di pagamento terminata");
}

void start() {
  String indirizzo = server.arg("indirizzo");
  String amount = server.arg("amount");
  prezzo_prodotto_dash = amount.toFloat();
  Serial.println("Avvio sessione da smartphone...");
  Serial.print("Mi aspetto un pagamento di ");
  Serial.print(amount);
  Serial.println(" Dash");
  Serial.print("Indirizzo: ");
  Serial.println(indirizzo);

  check_pagamento(indirizzo);
  
}

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
  pinMode(5, INPUT);

  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiName);

  WiFi.begin(wifiName, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
  server.on("/start", start);  
  server.begin();
}

void loop() {
  server.handleClient();
}
