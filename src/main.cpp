#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFiClientSecureBearSSL.h"
//#include <qrcode.h>
//#include <SSD1306.h>
#include <ESP8266WebServer.h>

//SSD1306  display(0x3c, D1, D2);
//QRcode qrcode (&display);

const char* wifiName = "master";
const char* wifiPass = "dash-pay-machine";
bool slave = false;
const float prezzo_prodotto_eur = 0.5;
float prezzo_prodotto_dash;
int scelta_indirizzo = 0;
const char* wallet_dash[] = {
  "Xg25shPiayEecsmDemcRrJE5ZEnL2CeAPs",
  "XpgMpGuWZVKAEzdgk7zCDWtsoGBN98DZZj",
  "XoY8E5f3esWbB7admkCsXV3bNevnNUamkZ",
  "XqRAPygXK3abBWqABPryVdt6AdyWGdx64G",
  "XpR7hwuu2XxnX1GjCYtVRJetzobQprskX4",
  "XsXuu562DD83DidxNeFfciZ3qdhHonZp9w",
  "XqsMnAAePXsn7aBCXWuCXVN2VGbjs546R1",
  "XxaoCGg8ZgrbPVwBCbTJS7o6QmniDMjBSk",
  "Xw4kqBpYjHrvfux5DyVcpcppGh1hH5aWmE",
  "XofYRXMKtaY7BRQZxBUgi133JcZkPfkke1",
  "XxUaCLxQ8Vb6npWZYnK6Mbc4Mg2BHk5z2J",
  "Xe3h9D1n3WH4hEWKyKTHb7yALK2kCTHEmu",
  "Xyf9ZXBQqPJNQdAkHawdFAKASj2MVVrsyA",
  "XnkotY9jE1xivipcXcvq6QLC7mYMoeQEtZ",
  "Xwz7793Z9mAgyVWDj49gPeznrcXGPV161H",
  "XiRfqffP8yw9TKVSNw2NFZnpPnMFG8HQAt",
  "XivtEUW3YzDnezBqjmCZvcvv1GzD6d5ZBj",
  "Xmu8iRjXvezgxS5QbvU4tnzkUE846XZL14"
  };


//Web Server address to read/write from 

WiFiClientSecure client;
ESP8266WebServer server(80);
HTTPClient http; //Object of class HTTPClient
DynamicJsonDocument doc(1024);

void calcola_prezzo_dash() {
  const char* host = "https://www.alfacoins.com/api/rate/DASH_EUR.json";
  if (WiFi.status() == WL_CONNECTED) {            
    client.setInsecure(); //the magic line, use with caution
    client.connect(host, 443);
    http.begin(client, host);

    if (http.GET() == HTTP_CODE_OK) {    
      String payload = http.getString();
      payload.replace("[","");
      payload.replace("]","");
      payload.replace("\"","");
      prezzo_prodotto_dash = prezzo_prodotto_eur / payload.toFloat();
      http.end(); //Close connection
    }
  }
}

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

// void mostra_qrcode() {
//   Serial.println("Avvio sessione da pulsante...");
//   calcola_prezzo_dash();
//   String qrcode_testo;
//   qrcode_testo = "dash:"+String(wallet_dash[scelta_indirizzo])+"?amount="+String(prezzo_prodotto_dash); 
//   qrcode.init();  
//   qrcode.create(qrcode_testo);

//   Serial.print("Mi aspetto un pagamento di ");
//   Serial.print(prezzo_prodotto_dash,6);
//   Serial.println(" Dash");
//   Serial.print("Indirizzo: ");
//   Serial.println(wallet_dash[scelta_indirizzo]);

//   check_pagamento(wallet_dash[scelta_indirizzo]);

//   display.clear();
//   display.display();

//   scelta_indirizzo++;
//   if (scelta_indirizzo > 17 ) {
//     scelta_indirizzo = 0;
//   }
// }

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

  // display.init();
  // display.clear();
  // display.display();

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
  // if(digitalRead(5) == 1) {
  //   slave = false;
  //   mostra_qrcode();
  // }
}