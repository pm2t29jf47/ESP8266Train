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

unsigned long ChimneyTimer;
unsigned int ChimneyLowHighPeriod = 2000;
bool IsChimneyLow = true;
SoftwareSerial PlayerSerial(PlayerRxPin, PlayerTxPin);
ESP8266WebServer server(80);
DFRobotDFPlayerMini player;
void printDetail(uint8_t type, int value);

void InitializeWiFi();
void InitializeWebServer();
void InitializeEngine();
void InitializeChimney();
void InitializePlayer();

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Train initialization started"));
  InitializeEngine();
  InitializeChimney();
  InitializePlayer();
  InitializeWebServer();
  InitializeWiFi();
  Serial.println(F("Train initialization completed"));
}

void loop() {
  server.handleClient();
  if (millis() - ChimneyTimer >= ChimneyLowHighPeriod) {
    ChimneyTimer = millis();

    if (IsChimneyLow) {
      analogWrite(ChimneyPin, 255);
    } else {
      analogWrite(ChimneyPin, 100);
    }

    IsChimneyLow = !IsChimneyLow;
  }
}

void InitializePlayer() {
  PlayerSerial.begin(9600);
  if (!player.begin(PlayerSerial, false, true)) {
    Serial.println(F("Player NOT initialized"));
    while (true) { delay(0); }
  }
  player.setTimeOut(500);
  delay(1000);
  player.volume(0);
  delay(1000);
  player.play(4);
  Serial.println(F("Player initialized"));
}

void InitializeChimney() {
  pinMode(ChimneyPin, OUTPUT);
  Serial.println(F("Chimney initialized"));
}

void InitializeEngine() {
  pinMode(EnginePin, OUTPUT);
  analogWrite(EnginePin, 0);
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
}

void InitializeWebServer() {
  server.on("/", handle_OnConnect);
  server.on("/changeThrottle", handle_changeThrottle);
  server.on("/changeVolume", handle_changeVolume);
  server.begin();
  Serial.println(F("HTTP server initialized"));
}

void handle_OnConnect() {
  server.send(200, "text/html", F("<!DOCTYPE html><html lang=\"ru\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=utf-8><style>.container{ width: 540px; max-width: 100%; margin: 0 auto; padding: 0 2rem; border: 1px solid #d0d8dd; background-color: white; border-radius: 6px; box-shadow: 0px 2px 8px rgba(0, 0, 0, 0.06), 0px 1px 3px rgba(0, 0, 0, 0.05); box-sizing: border-box; font-weight: normal; font-family: sans-serif;} .input-row{ display: flex; padding: 2rem 0; border-bottom: 1px solid #d0d8dd;} .input-row:last-child{ border-bottom: 0;} .title{ margin-right: 2rem;} .label{ margin-bottom: 0.25rem; font-weight: bold;} .input{ display: flex; align-items: center; margin-left: auto;} button{ display: flex; justify-content: center; align-items: center; width: 3rem; height: 3rem; border: 1px solid #0064fe; border-radius: 1000px; background-color: white;} button:hover{ background-color: #b8dcff; cursor: pointer;} button:focus{ outline: none; box-shadow: 0 0 0 0.25rem #b8dcff;} button[disabled]{ opacity: 0.5; pointer-events: none;} button:active{ background-color: #7ab8ff;} .number{ font-size: 1.25rem; min-width: 5rem; text-align: center;} .numberValue{ display: inline;} .percentSign{ display: inline-block;} .icon{ user-select: none;} .dim{ color: #8d9ca7;} </style></head><body><div class=\"container\"><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Газ</h3></div><div id=\"throttleInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"numberValue\">0</div><div class=\"percentSign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Громкость</h3></div><div id=\"volumeInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"numberValue\">0</div><div class=\"percentSign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div></div><script>const baseThrottleUrl=\"/changeThrottle\"; const baseVolumeUrl=\"/changeVolume\"; const throttleInputButtons=document.getElementById(\"throttleInput\").querySelectorAll(\"button\"); throttleInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseThrottleUrl + '?value=' + newNumberValue; const response=await fetch(url).catch((error)=>{ alert(\"Ошибка Fetch: \" + error);}); if (response===undefined){ return;} if (!response.ok){ alert(\"Ошибка HTTP: \" + response.status);} console.log(\"ThrottleInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); const volumeInputButtons=document.getElementById(\"volumeInput\").querySelectorAll(\"button\"); volumeInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseVolumeUrl + '?value=' + newNumberValue; const response=await fetch(url).catch((error)=>{ alert(\"Ошибка Fetch: \" + error);}); if (response===undefined){ return;} if (!response.ok){ alert(\"Ошибка HTTP: \" + response.status);} console.log(\"VolumeInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); async function getPlusMinusInputValue(event, serverAction){ const pressedButton=event.currentTarget; const minusButton=pressedButton.parentNode.querySelector(\".minus\"); const plusButton=pressedButton.parentNode.querySelector(\".plus\"); const numberContainer=pressedButton.parentNode.querySelector(\".number\"); const numberValueContainer=pressedButton.parentNode.querySelector(\".numberValue\"); const numberValue=parseFloat(numberValueContainer.textContent); const newNumberValue=pressedButton.classList.contains(\"plus\") ? numberValue + 10 : numberValue - 10; minusButton.disabled=true; plusButton.disabled=true; numberContainer.classList.add(\"dim\"); const serverActionResult=await serverAction(newNumberValue); if (serverActionResult !==newNumberValue || serverActionResult===null){ return;} minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\"); numberValueContainer.textContent=serverActionResult; if (newNumberValue===0){ minusButton.disabled=true; plusButton.disabled=false; numberContainer.classList.add(\"dim\"); pressedButton.blur();} else if (newNumberValue >0 && newNumberValue < 100){ minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\");} else if (newNumberValue===100){ minusButton.disabled=false; plusButton.disabled=true; pressedButton.blur();}} </script></body></html>"));
}

void handle_changeThrottle() {
  int value = server.arg(0).toInt();
  int pwmValue = 0;

  switch (value) {
    case 10:
      pwmValue = 100;
      ChimneyLowHighPeriod = 1000;
      break;
    case 20:
      pwmValue = 120;
      ChimneyLowHighPeriod = 900;
      break;
    case 30:
      pwmValue = 135;
      ChimneyLowHighPeriod = 800;
      break;
    case 40:
      pwmValue = 155;
      ChimneyLowHighPeriod = 700;
      break;
    case 50:
      pwmValue = 170;
      ChimneyLowHighPeriod = 600;
      break;
    case 60:
      pwmValue = 185;
      ChimneyLowHighPeriod = 500;
      break;
    case 70:
      pwmValue = 200;
      ChimneyLowHighPeriod = 400;
      break;
    case 80:
      pwmValue = 215;
      ChimneyLowHighPeriod = 300;
      break;
    case 90:
      pwmValue = 230;
      ChimneyLowHighPeriod = 200;
      break;
    case 100:
      pwmValue = 255;
      ChimneyLowHighPeriod = 100;
      break;
    default:
      pwmValue = 0;
      ChimneyLowHighPeriod = 2000;
      break;
  }

  analogWrite(EnginePin, pwmValue);

  String payload = "{ \"value\": " + String(value) + " }";
  Serial.println(payload);
  server.send(200, "text/json", payload);
}

void handle_changeVolume() {
  int value = server.arg(0).toInt();
  int playerValue = 0;

  switch (value) {
    case 10:
      playerValue = 3;
      break;
    case 20:
      playerValue = 6;
      break;
    case 30:
      playerValue = 9;
      break;
    case 40:
      playerValue = 12;
      break;
    case 50:
      playerValue = 15;
      break;
    case 60:
      playerValue = 18;
      break;
    case 70:
      playerValue = 21;
      break;
    case 80:
      playerValue = 24;
      break;
    case 90:
      playerValue = 27;
      break;
    case 100:
      playerValue = 30;
      break;
    default:
      playerValue = 0;
      break;
  }

  player.pause();
  delay(1000);
  player.volume(playerValue);
  delay(1000);
  player.start();

  String payload = "{ \"value\": " + String(value) + " }";
  Serial.println(payload);
  server.send(200, "text/json", payload);
}
