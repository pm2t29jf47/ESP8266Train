#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#define AP_SSID "FuryTrain"
#define ChimneyPin 5
#define EnginePin 4
#define PlayerRxPin 12
#define PlayerTxPin 14

SoftwareSerial PlayerSerial(PlayerRxPin, PlayerTxPin);
DFRobotDFPlayerMini player;

void InitializeWiFi();
void InitializeEngine();
void InitializeChimney();
void InitializePlayer();

void setup() {
  InitializeWiFi();
  InitializeEngine();
  InitializeChimney();
  InitializePlayer();
  Serial.begin(115200);

  delay(3000);
  player.volume(0);
  player.play(2);
  delay(3000);
  analogWrite(EnginePin, 100);
}

void loop() {
  analogWrite(ChimneyPin, 255);
  delay(200);
  analogWrite(ChimneyPin, 100);
  delay(500);
}

void InitializePlayer() {
  PlayerSerial.begin(9600);
  if (!player.begin(PlayerSerial, false, true)) {
    Serial.println("Player NOT started");
    while (true) { delay(0); }
  }
  Serial.println("Player started");
}

void InitializeChimney() {
  pinMode(ChimneyPin, OUTPUT);
}

void InitializeEngine() {
  pinMode(EnginePin, OUTPUT);
}

void InitializeWiFi() {
  IPAddress local_ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  ESP8266WebServer server(80);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.begin();
  server.on("/", handle_OnConnect);
  Serial.println("HTTP server started");
}

void handle_OnConnect() 
{ 
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

String SendHTML(uint8_t led1stat,uint8_t led2stat)
{
  String ptr = "<!DOCTYPE html> <html> 
  <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">
  <title>LED Control</title>
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
  body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
  if(led1stat)
    ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
  else
    ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";



  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}