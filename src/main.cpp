#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

const char* wifiName = "master";
const char* wifiPass = "dash-pay-machine";

IPAddress local_IP(192, 168, 43, 214);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

const int pin_gettoniera[]={200,100,20,5}; // valore in centesimi dei pin collegati in gettoniera

void carica_credito(int imp, int del) {
  Serial.print("Carico un credito di ");
  Serial.println(imp);
  while(imp >= pin_gettoniera[3]) {
    if ((pin_gettoniera[0] == imp) or (imp > pin_gettoniera[0] and ((imp - pin_gettoniera[0]) % pin_gettoniera[3] == 0 or (imp - pin_gettoniera[0]) % pin_gettoniera[2] == 0 or (imp - pin_gettoniera[0]) % pin_gettoniera[1] == 0))){
      digitalWrite(12, LOW);
      delay(del);
      digitalWrite(12, HIGH);
      delay(del);
      imp = imp - pin_gettoniera[0];
      Serial.print("caricato ");
      Serial.println(pin_gettoniera[0]);
    }
    else if ((pin_gettoniera[1] == imp) or (imp >= pin_gettoniera[1] and ((imp - pin_gettoniera[1]) % pin_gettoniera[3] == 0 or (imp - pin_gettoniera[1]) % pin_gettoniera[2] == 0 )))
    {
      digitalWrite(13, LOW);
      delay(del);
      digitalWrite(13, HIGH);
      delay(del);  
      imp = imp - pin_gettoniera[1];
      Serial.print("caricato ");
      Serial.println(pin_gettoniera[1]);
    }
    else if ((pin_gettoniera[2] == imp) or (imp > pin_gettoniera[2] and (imp - pin_gettoniera[2]) % pin_gettoniera[3] == 0 ))
    {
      digitalWrite(14, LOW);
      delay(del);
      digitalWrite(14, HIGH);
      delay(del);
      imp = imp - pin_gettoniera[2];
      Serial.print("caricato ");
      Serial.println(pin_gettoniera[2]);
    }
    else if (imp >= pin_gettoniera[3])
    {
      digitalWrite(16, LOW);
      delay(del);
      digitalWrite(16, HIGH);
      delay(del);
      imp = imp - pin_gettoniera[3];
      Serial.print("caricato ");
      Serial.println(pin_gettoniera[3]);
    }
    else{
      Serial.println("nessun importo trovato");
      return;
    }          
  }
}

void start() {
  server.send(200, "text/plain", "Comando ricevuto!");
  int credito = server.arg("credito").toInt();
  int delay_impulsi = server.arg("delay_impulsi").toInt();
  Serial.println("Avvio sessione da smartphone...");
  Serial.print("Carico con delay impulsi a  ");
  Serial.println(delay_impulsi);

  carica_credito(credito,delay_impulsi);
  
}

void setup() {
  httpUpdater.setup(&server);

  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH); 
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH); 
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);  

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
  Serial.print("HTTPUpdateServer ready! Open http://");
  Serial.print(WiFi.localIP());
  Serial.print("/update in your browser");
  Serial.println("");
}

void loop() {
  server.handleClient();
}
