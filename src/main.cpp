#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* wifiName = "master";
const char* wifiPass = "dash-pay-machine";

IPAddress local_IP(192, 168, 43, 214);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

void carica_credito(int imp, int del) {
  for (int i=1; i<=imp; i++){
    digitalWrite(4, HIGH); 
    delay(del);
    digitalWrite(4, LOW);
    delay(del);
  }
    Serial.println("Credito caricato!");
}

void start() {
  server.send(200, "text/plain", "Comando ricevuto!");
  int impulsi = server.arg("impulsi").toInt();
  int delay_impulsi = server.arg("delay_impulsi").toInt();
  Serial.println("Avvio sessione da smartphone...");
  Serial.print("Carico ");
  Serial.print(impulsi);
  Serial.println(" impulsi");
  Serial.print("Con delay ");
  Serial.println(delay_impulsi);

  carica_credito(impulsi,delay_impulsi);
  
}

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 

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
