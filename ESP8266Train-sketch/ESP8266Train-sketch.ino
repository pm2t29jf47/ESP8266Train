#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#define AP_SSID "furyTrain"
#define ChimneyPin 5
#define EnginePin 4
#define PlayerRxPin 12
#define PlayerTxPin 14

SoftwareSerial PlayerSerial(PlayerRxPin, PlayerTxPin);
ESP8266WebServer server(80);
DFRobotDFPlayerMini player;

void InitializeWiFi();
void InitializeEngine();
void InitializeChimney();
void InitializePlayer();

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Train initialization started"));
  InitializeWiFi();
  InitializeEngine();
  InitializeChimney();
  InitializePlayer();  

  delay(3000);
  player.volume(0);
  player.play(2);
  delay(3000);
  analogWrite(EnginePin, 100);
  Serial.println(F("Train initialization completed"));
}

void loop() {
  server.handleClient();
  analogWrite(ChimneyPin, 255);
  delay(200);
  analogWrite(ChimneyPin, 100);
  delay(500);
}

void InitializePlayer() {
  PlayerSerial.begin(9600);
  if (!player.begin(PlayerSerial, false, true)) {
    Serial.println(F("Player NOT initialized"));
    while (true) { delay(0); }
  }
  Serial.println(F("Player initialized"));
}

void InitializeChimney() {
  pinMode(ChimneyPin, OUTPUT);
  Serial.println(F("Chimney initialized"));
}

void InitializeEngine() {  
  pinMode(EnginePin, OUTPUT);
  Serial.println(F("Engine initialized"));
}

void InitializeWiFi() {
  IPAddress local_ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println(F("WiFi initialized"));
  delay(100); 
  server.on("/", handle_OnConnect);
  server.on("/changeThrottle", handle_changeThrottle);
  server.on("/changeVolume", handle_changeVolume);  
  server.begin();
  Serial.println(F("HTTP server initialized"));
}

void handle_OnConnect() 
{ 
  server.send(200, "text/html", SendHTML());
}

void handle_changeThrottle()
{
  int value = server.arg(0).toInt();
  
  Serial.println(value);
  server.send(200, "text/plain", String(value)); 
}

void handle_changeVolume()
{
  int value = server.arg(0).toInt();
  Serial.println(value);
  server.send(200, "text/plain", String(value)); 
}

String SendHTML()
{
  String ptr = "<!DOCTYPE html><html lang=\"ru\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=utf-8><style>.container{ width: 540px; max-width: 100%; margin: 0 auto; padding: 0 2rem; border: 1px solid #d0d8dd; background-color: white; border-radius: 6px; box-shadow: 0px 2px 8px rgba(0, 0, 0, 0.06), 0px 1px 3px rgba(0, 0, 0, 0.05); box-sizing: border-box; font-weight: normal; font-family: sans-serif;} .input-row{ display: flex; padding: 2rem 0; border-bottom: 1px solid #d0d8dd;} .input-row:last-child{ border-bottom: 0;} .title{ margin-right: 2rem;} .label{ margin-bottom: 0.25rem; font-weight: bold;} .input{ display: flex; align-items: center; margin-left: auto;} button{ display: flex; justify-content: center; align-items: center; width: 3rem; height: 3rem; border: 1px solid #0064fe; border-radius: 1000px; background-color: white;} button:hover{ background-color: #b8dcff; cursor: pointer;} button:focus{ outline: none; box-shadow: 0 0 0 0.25rem #b8dcff;} button[disabled]{ opacity: 0.5; pointer-events: none;} button:active{ background-color: #7ab8ff;} .number{ font-size: 1.25rem; min-width: 5rem; text-align: center;} .numberValue{ display: inline;} .percentSign{ display: inline-block;} .icon{ user-select: none;} .dim{ color: #8d9ca7;} </style></head><body><div class=\"container\"><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Газ</h3></div><div id=\"throttleInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"numberValue\">0</div><div class=\"percentSign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Громкость</h3></div><div id=\"volumeInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"numberValue\">0</div><div class=\"percentSign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div></div><script>const baseThrottleUrl=\"/changeThrottle\"; const baseVolumeUrl=\"/changeVolume\"; const throttleInputButtons=document.getElementById(\"throttleInput\").querySelectorAll(\"button\"); throttleInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseThrottleUrl + '?value=' + newNumberValue; const response=await fetch(url).catch((error)=>{ alert(\"Ошибка Fetch: \" + error);}); if (response===undefined){ return;} if (!response.ok){ alert(\"Ошибка HTTP: \" + response.status);} console.log(\"ThrottleInput value: \" + newNumberValue); return await response.text;});});}); const volumeInputButtons=document.getElementById(\"volumeInput\").querySelectorAll(\"button\"); volumeInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseVolumeUrl + '?value=' + newNumberValue; const response=await fetch(url).catch((error)=>{ alert(\"Ошибка Fetch: \" + error);}); if (response===undefined){ return;} if (!response.ok){ alert(\"Ошибка HTTP: \" + response.status);} console.log(\"VolumeInput value: \" + newNumberValue); return await response.text;});});}); function getPlusMinusInputValue(event, serverAction){ const pressedButton=event.currentTarget; const minusButton=pressedButton.parentNode.querySelector(\".minus\"); const plusButton=pressedButton.parentNode.querySelector(\".plus\"); const numberContainer=pressedButton.parentNode.querySelector(\".number\"); const numberValueContainer=pressedButton.parentNode.querySelector(\".numberValue\"); const numberValue=parseFloat(numberValueContainer.textContent); const newNumberValue=pressedButton.classList.contains(\"plus\") ? numberValue + 20 : numberValue - 20; const serverActionResult=serverAction(newNumberValue); if (serverActionResult !==newNumberValue || serverActionResult===null){ return;} numberValueContainer.textContent=newNumberValue; if (newNumberValue===0){ minusButton.disabled=true; numberContainer.classList.add(\"dim\"); pressedButton.blur();} else if (newNumberValue >0 && newNumberValue < 100){ minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\");} else if (newNumberValue===100){ plusButton.disabled=true; pressedButton.blur();}} </script></body></html>";

  return ptr;
}