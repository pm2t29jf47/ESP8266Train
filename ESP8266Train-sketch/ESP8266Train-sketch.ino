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
unsigned long PlayerTimer;
unsigned int PlayerCommandDelay = 2000;
unsigned int ChimneyLowHighPeriod = 2000;
bool IsChimneyLow = true;
bool CurrentIsPlaying = false;
bool NewIsPlaying = false;
bool CurrentIsPaused = false;
bool NewIsPaused = false;
bool CurrentIsStopped = false;
bool NewIsStopped = false;
unsigned int CurrentVolume = 1;
unsigned int NewVolume = 0;
unsigned int CurrentTrack = 0;
unsigned int NewTrack = 3;
SoftwareSerial PlayerSerial(PlayerRxPin, PlayerTxPin);
ESP8266WebServer server(80);
DFRobotDFPlayerMini player;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Train initialization started"));
  InitializeEngine();
  InitializeChimney();
  InitializePlayer();
  InitializeWiFi();
  InitializeWebServer();
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

  if (millis() - PlayerTimer >= PlayerCommandDelay) {
    if (CurrentVolume != NewVolume) {
      CurrentVolume = NewVolume;
      player.volume(CurrentVolume);
    } else if (CurrentTrack != NewTrack) {
      CurrentTrack = NewTrack;
      player.play(CurrentTrack);
    } else if (CurrentIsPlaying) {
      if (NewIsPaused) {
        CurrentIsPlaying = false;
        CurrentIsPaused = true;
        CurrentIsStopped = false;
        player.pause();
      } else if (NewIsStopped) {
        CurrentIsPlaying = false;
        CurrentIsPaused = false;
        CurrentIsStopped = true;
        player.stop();
      }
    } else if (CurrentIsPaused) {
      if (NewIsPlaying) {
        CurrentIsPlaying = true;
        CurrentIsPaused = false;
        CurrentIsStopped = false;
        player.start();
      } else if (NewIsStopped) {
        CurrentIsPlaying = false;
        CurrentIsPaused = false;
        CurrentIsStopped = true;
        player.stop();
      }
    } else if (CurrentIsStopped) {
      if (NewIsPlaying) {
        CurrentIsPlaying = true;
        CurrentIsPaused = false;
        CurrentIsStopped = false;
        player.play(CurrentTrack);
      } else if (NewIsPaused) {
        CurrentIsPlaying = false;
        CurrentIsPaused = false;
        CurrentIsStopped = true;
        //do nothing
      }
    }
  }
}

void InitializePlayer() {
  PlayerSerial.begin(9600);
  if (!player.begin(PlayerSerial, false, true)) {
    Serial.println(F("Player NOT initialized"));
    while (true) { delay(0); }
  }
  player.setTimeOut(500);
  delay(3000);
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
  server.on("/nextTrack", handle_nextTrack);
  server.on("/previousTrack", handle_previousTrack);
  server.on("/play", handle_play);
  server.on("/pause", handle_pause);
  server.on("/stop", handle_stop);
  server.begin();
  Serial.println(F("HTTP server initialized"));
}

void handle_OnConnect() {
  server.send(200, "text/html", F("<!DOCTYPE html><html lang=\"ru\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=utf-8><style>.container{ width: 540px; max-width: 100%; margin: 0 auto; padding: 0 2rem; border: 1px solid #d0d8dd; background-color: white; border-radius: 6px; box-shadow: 0px 2px 8px rgba(0, 0, 0, 0.06), 0px 1px 3px rgba(0, 0, 0, 0.05); box-sizing: border-box; font-weight: normal; font-family: sans-serif;} .input-row{ display: flex; padding: 2rem 0; border-bottom: 1px solid #d0d8dd;} .input-row:last-child{ border-bottom: 0;} .title{ margin-right: 2rem;} .label{ margin-bottom: 0.25rem; font-weight: bold;} .input{ display: flex; align-items: center; margin-left: auto;} .player-inpit{ display: flex; align-items: center; margin-left: auto; margin-right: auto;} button{ display: flex; justify-content: center; align-items: center; width: 3rem; height: 3rem; border: 1px solid #0064fe; border-radius: 1000px; background-color: white;} button.previous{ margin-right: 0.5rem;} button.play{ margin-left: 0.3rem; margin-right: 0.1rem;} button.pause{ margin-left: 0.1rem; margin-right: 0.1rem;} button.stop{ margin-left: 0.1rem; margin-right: 0.3rem;} button.next{ margin-left: 0.5rem;} button.horn{ margin-right: 0.4rem;} button:focus{ outline: none; box-shadow: 0 0 0 0.25rem #b8dcff;} button[disabled]{ opacity: 0.5; pointer-events: none;} button:active{ background-color: #7ab8ff;} .number{ font-size: 1.25rem; min-width: 5rem; text-align: center;} .number-value{ display: inline;} .percent-sign{ display: inline-block;} .icon{ user-select: none;} .dim{ color: #8d9ca7;} </style></head><body><div class=\"container\"><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Газ</h3></div><div id=\"throttleInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"number-value\">0</div><div class=\"percent-sign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Громкость</h3></div><div id=\"volumeInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"number-value\">0</div><div class=\"percent-sign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div id=\"playerInput\" class=\"player-inpit\"><button class=\"previous\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"1\" y1=\"0\" x2=\"1\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><polygon points=\"4,8 16,0 16,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"play\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"16,8 4,0 4,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"pause\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"4\" y1=\"0\" x2=\"4\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line x1=\"12\" y1=\"0\" x2=\"12\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button><button class=\"stop\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"16,0 0,0 0,16 16,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"next\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"15\" y1=\"0\" x2=\"15\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><polygon points=\"12,8 0,0 0,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button></div></div></div><script>const baseThrottleUrl=\"/changeThrottle\"; const baseVolumeUrl=\"/changeVolume\"; const previousTrackUrl=\"/previousTrack\"; const playTrackUrl=\"/play\"; const pauseTrackUrl=\"/pause\"; const stopTrackUrl=\"/stop\"; const nextTrackUrl=\"/nextTrack\"; const throttleInputButtons=document.getElementById(\"throttleInput\").querySelectorAll(\"button\"); const volumeInputButtons=document.getElementById(\"volumeInput\").querySelectorAll(\"button\"); const playerInputButtons=document.getElementById(\"playerInput\").querySelectorAll(\"button\"); throttleInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseThrottleUrl + '?value=' + newNumberValue; const response=await fetch(url).catch(()=>{ return;}); if (response===undefined || !response.ok){ alert(\"Ошибка HTTP: \" + response?.status);} console.log(\"ThrottleInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); volumeInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseVolumeUrl + '?value=' + newNumberValue; const response=await fetch(url).catch(()=>{ return;}); if (response===undefined || !response.ok){ alert(\"Ошибка HTTP: \" + response?.status);} console.log(\"VolumeInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); async function getPlusMinusInputValue(event, serverAction){ const pressedButton=event.currentTarget; const minusButton=pressedButton.parentNode.querySelector(\".minus\"); const plusButton=pressedButton.parentNode.querySelector(\".plus\"); const numberContainer=pressedButton.parentNode.querySelector(\".number\"); const numberValueContainer=pressedButton.parentNode.querySelector(\".number-value\"); const numberValue=parseFloat(numberValueContainer.textContent); const newNumberValue=pressedButton.classList.contains(\"plus\") ? numberValue + 10 : numberValue - 10; minusButton.disabled=true; plusButton.disabled=true; numberContainer.classList.add(\"dim\"); const serverActionResult=await serverAction(newNumberValue); if (serverActionResult !==newNumberValue || serverActionResult===null){ return;} minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\"); numberValueContainer.textContent=serverActionResult; if (newNumberValue===0){ minusButton.disabled=true; plusButton.disabled=false; numberContainer.classList.add(\"dim\"); pressedButton.blur();} else if (newNumberValue >0 && newNumberValue < 100){ minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\");} else if (newNumberValue===100){ minusButton.disabled=false; plusButton.disabled=true; pressedButton.blur();}} playerInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ const pressedButton=event.currentTarget; const previousButton=pressedButton.parentNode.querySelector(\".previous\"); const playButton=pressedButton.parentNode.querySelector(\".play\"); const pauseButton=pressedButton.parentNode.querySelector(\".pause\"); const stopButton=pressedButton.parentNode.querySelector(\".stop\"); const nextButton=pressedButton.parentNode.querySelector(\".next\"); let currentState={ previousButtonIsDisabled: previousButton.disabled, playButtonIsDisabled: playButton.disabled, pauseButtonIsDisabled: pauseButton.disabled, stopButtonIsDisabled: stopButton.disabled, nextButtonIsDisabled: nextButton.disabled}; previousButton.disabled=true; playButton.disabled=true; pauseButton.disabled=true; stopButton.disabled=true; nextButton.disabled=true; switch (pressedButton){ case previousButton: const previousResponse=await fetch(previousTrackUrl).catch(()=>{ return;}); if (previousResponse===undefined || !previousResponse.ok){ alert(\"Ошибка HTTP: \" + previousResponse?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break; case playButton: const playResponse=await fetch(playTrackUrl).catch(()=>{ return;}); if (playResponse===undefined || !playResponse.ok){ alert(\"Ошибка HTTP: \" + playResponse?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break; case pauseButton: const pauseResponse=await fetch(pauseTrackUrl).catch(()=>{ return;}); if (pauseResponse===undefined || !pauseResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=false; pauseButton.disabled=true; stopButton.disabled=false; nextButton.disabled=false;} break; case stopButton: const stopResponse=await fetch(stopTrackUrl).catch(()=>{ return;}); if (stopResponse===undefined || !stopResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=false; pauseButton.disabled=true; stopButton.disabled=true; nextButton.disabled=false;} break; case nextButton: const nextResponse=await fetch(nextTrackUrl).catch(()=>{ return;}); if (nextResponse===undefined || !nextResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break;}});}); </script></body></html>"));
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

  NewVolume = playerValue;

  String payload = "{ \"value\": " + String(value) + " }";
  Serial.println(payload);
  server.send(200, "text/json", payload);
}

void handle_nextTrack() {
  switch (CurrentTrack) {
    case 3:
      NewTrack = 4;
      break;
    case 4:
      NewTrack = 5;
      break;
    case 5:
      NewTrack = 3;
      break;
  }

  server.send(200, "text/json");
}

void handle_previousTrack() {
  switch (CurrentTrack) {
    case 3:
      NewTrack = 5;
      break;
    case 4:
      NewTrack = 3;
      break;
    case 5:
      NewTrack = 4;
      break;
  }

  server.send(200, "text/json");
}

void handle_play() {
  NewIsPlaying = true;
  NewIsPaused = false;
  NewIsStopped = false;

  server.send(200, "text/json");
}

void handle_pause() {
  NewIsPlaying = false;
  NewIsPaused = true;
  NewIsStopped = false;

  server.send(200, "text/json");
}

void handle_stop() {
  NewIsPlaying = false;
  NewIsPaused = false;
  NewIsStopped = true;

  server.send(200, "text/json");
}
