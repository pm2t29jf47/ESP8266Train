#include <DFPlayerMini_Fast.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "PlayerStateComposition.h"
#define AP_SSID "furyTrain"
#define ChimneyPin 5
#define EnginePin 4
#define PlayerRxPin 12
#define PlayerTxPin 14
#define VoltageRxPin 13

unsigned long _chimneyTimer;
unsigned long _playerTimer;
int _playerLoopRepeater = 0;
int _playerLoopRepeatCount = 5;
unsigned int _playerCommandDelay = 500;
unsigned int _chimneyLowHighPeriod = 2000;
bool _isChimneyLow = true;
bool _isPlayerWakedUp;
SoftwareSerial _playerSerial(PlayerRxPin, PlayerTxPin);
ESP8266WebServer _server(80);
DFPlayerMini_Fast _player;
PlayerStateComposition _playerStateComposition;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Train initialization started"));
  InitializeEngine();
  InitializeChimney();
  InitializePlayer();
  InitializeWiFi();
  InitializeWebServer();
  _player.volume(15);
  delay(500);
  _player.play(2);
  delay(5000);
  _player.volume(0);
  Serial.println(F("Train initialization completed"));
}

void loop() {
  _server.handleClient();

  if (millis() - _chimneyTimer >= _chimneyLowHighPeriod) {
    _chimneyTimer = millis();

    if (_isChimneyLow) {
      analogWrite(ChimneyPin, 255);
    } else {
      analogWrite(ChimneyPin, 100);
    }

    _isChimneyLow = !_isChimneyLow;
  }

  if (millis() - _playerTimer >= _playerCommandDelay) {
    _playerTimer = millis();
    if (_playerStateComposition.isVolumeChanged) {
      if (_isPlayerWakedUp) {
        _player.volume(_playerStateComposition.applyVolume());
        _isPlayerWakedUp = false;
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_playerStateComposition.isQueueTrackChanged) {
      if (_isPlayerWakedUp) {
        _player.play(_playerStateComposition.applyCurrentTrackInQueue());
        _isPlayerWakedUp = false;
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_playerStateComposition.isStateChanged) {

      switch (_playerStateComposition.currentState) {
        case play:
          switch (_playerStateComposition.newState) {
            case play:
              _playerStateComposition.applyState();
              break;
            case pause:
              if (_isPlayerWakedUp) {
                _playerStateComposition.applyState();
                _player.pause();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            default:
              if (_isPlayerWakedUp) {
                _playerStateComposition.applyState();
                _player.stop();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
          }
          break;
        case pause:
          switch (_playerStateComposition.newState) {
            case play:
              if (_isPlayerWakedUp) {
                _playerStateComposition.applyState();
                _player.resume();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            case pause:
              _playerStateComposition.applyState();
              break;
            default:
              if (_isPlayerWakedUp) {
                _playerStateComposition.applyState();
                _player.stop();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
          }
          break;
        default:
          switch (_playerStateComposition.newState) {
            case play:
              if (_isPlayerWakedUp) {
                _playerStateComposition.applyState();
                _player.play(_playerStateComposition.currentTrackInQueue);
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            default:
              _playerStateComposition.applyState();
              break;
          }
          break;
      }
    } else if (_playerStateComposition.currentState == play
               && !_playerStateComposition.isStateChanged
               && !_playerStateComposition.isQueueTrackChanged
               && !_player.isPlaying()) {
      if (!_player.isPlaying()) {
        _playerLoopRepeater++;
      } else {
        _playerLoopRepeater = 0;
      }
      if (_playerLoopRepeater >= _playerLoopRepeatCount) {
        _playerLoopRepeater = 0;
        _playerStateComposition.playNextTrackInQueue();
      }
    }
  }
}

void InitializePlayer() {
  _playerSerial.begin(9600);
  if (!_player.begin(_playerSerial, false, 500)) {
    Serial.println(F("Player NOT initialized"));
    while (true) {
      delay(0);
    }
  }  
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
  _server.on("/", handle_OnConnect);
  _server.on("/changeThrottle", handle_changeThrottle);
  _server.on("/changeVolume", handle_changeVolume);
  _server.on("/nextTrack", handle_nextTrack);
  _server.on("/previousTrack", handle_previousTrack);
  _server.on("/play", handle_play);
  _server.on("/pause", handle_pause);
  _server.on("/stop", handle_stop);
  _server.on("/checkBatteryStatus", hsndle_checkBatteryStatus);
    _server.begin();
  Serial.println(F("HTTP _server initialized"));
}

void handle_OnConnect() {
  _server.send(200, "text/html", F("<!DOCTYPE html><html lang=\"ru\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" charset=utf-8><style>.container{ width: 540px; max-width: 100%; margin: 0 auto; padding: 0 2rem; border: 1px solid #d0d8dd; background-color: white; border-radius: 6px; box-shadow: 0px 2px 8px rgba(0, 0, 0, 0.06), 0px 1px 3px rgba(0, 0, 0, 0.05); box-sizing: border-box; font-weight: normal; font-family: sans-serif;} .input-row{ display: flex; padding: 2rem 0; border-bottom: 1px solid #d0d8dd;} .input-row:last-child{ border-bottom: 0;} .title{ margin-right: 2rem;} .label{ margin-bottom: 0.25rem; font-weight: bold;} .input{ display: flex; align-items: center; margin-left: auto;} ._player-inpit{ display: flex; align-items: center; margin-left: auto; margin-right: auto;} button{ display: flex; justify-content: center; align-items: center; width: 3rem; height: 3rem; border: 1px solid #0064fe; border-radius: 1000px; background-color: white;} button.previous{ margin-right: 0.5rem;} button.play{ margin-left: 0.3rem; margin-right: 0.1rem;} button.pause{ margin-left: 0.1rem; margin-right: 0.1rem;} button.stop{ margin-left: 0.1rem; margin-right: 0.3rem;} button.next{ margin-left: 0.5rem;} button.horn{ margin-right: 0.4rem;} button:focus{ outline: none; box-shadow: 0 0 0 0.25rem #b8dcff;} button[disabled]{ opacity: 0.5; pointer-events: none;} button:active{ background-color: #7ab8ff;} .number{ font-size: 1.25rem; min-width: 5rem; text-align: center;} .number-value{ display: inline;} .percent-sign{ display: inline-block;} .icon{ user-select: none;} .dim{ color: #8d9ca7;} </style></head><body><div class=\"container\"><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Газ</h3></div><div id=\"throttleInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"number-value\">0</div><div class=\"percent-sign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div class=\"title\"><h3 class=\"label\">Громкость</h3></div><div id=\"volumeInput\" class=\"input\"><button class=\"minus\" disabled><svg width=\"16\" height=\"2\" viewBox=\"0 0 16 2\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line y1=\"1\" x2=\"16\" y2=\"1\" stroke=\"#0064FE\" stroke-width=\"2\" class=\"icon\" /></svg></button><div class=\"number dim\"><div class=\"number-value\">0</div><div class=\"percent-sign\">%</div></div><button class=\"plus\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" class=\"icon\"><line x1=\"8\" y1=\"4.37114e-08\" x2=\"8\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line y1=\"8\" x2=\"16\" y2=\"8\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button></div></div><div class=\"input-row\"><div id=\"playerInput\" class=\"_player-inpit\"><button class=\"previous\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"1\" y1=\"0\" x2=\"1\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><polygon points=\"4,8 16,0 16,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"play\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"16,8 4,0 4,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"pause\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"4\" y1=\"0\" x2=\"4\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><line x1=\"12\" y1=\"0\" x2=\"12\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /></svg></button><button class=\"stop\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><polygon points=\"16,0 0,0 0,16 16,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button><button class=\"next\" disabled><svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><line x1=\"15\" y1=\"0\" x2=\"15\" y2=\"16\" stroke=\"#0064FE\" stroke-width=\"2\" /><polygon points=\"12,8 0,0 0,16\" fill=\"#0064FE\" stroke=\"#0064FE\" stroke-width=\"1\" /></svg></button></div></div></div><script>const baseThrottleUrl=\"/changeThrottle\"; const baseVolumeUrl=\"/changeVolume\"; const previousTrackUrl=\"/previousTrack\"; const playTrackUrl=\"/play\"; const pauseTrackUrl=\"/pause\"; const stopTrackUrl=\"/stop\"; const nextTrackUrl=\"/nextTrack\"; const throttleInputButtons=document.getElementById(\"throttleInput\").querySelectorAll(\"button\"); const volumeInputButtons=document.getElementById(\"volumeInput\").querySelectorAll(\"button\"); const playerInputButtons=document.getElementById(\"playerInput\").querySelectorAll(\"button\"); throttleInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseThrottleUrl + '?value=' + newNumberValue; const response=await fetch(url).catch(()=>{ return;}); if (response===undefined || !response.ok){ alert(\"Ошибка HTTP: \" + response?.status);} console.log(\"ThrottleInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); volumeInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ getPlusMinusInputValue( event, async (newNumberValue)=>{ const url=baseVolumeUrl + '?value=' + newNumberValue; const response=await fetch(url).catch(()=>{ return;}); if (response===undefined || !response.ok){ alert(\"Ошибка HTTP: \" + response?.status);} console.log(\"VolumeInput value: \" + newNumberValue); const jsonResponse=await response.json(); return jsonResponse.value;});});}); async function getPlusMinusInputValue(event, _serverAction){ const pressedButton=event.currentTarget; const minusButton=pressedButton.parentNode.querySelector(\".minus\"); const plusButton=pressedButton.parentNode.querySelector(\".plus\"); const numberContainer=pressedButton.parentNode.querySelector(\".number\"); const numberValueContainer=pressedButton.parentNode.querySelector(\".number-value\"); const numberValue=parseFloat(numberValueContainer.textContent); const newNumberValue=pressedButton.classList.contains(\"plus\") ? numberValue + 10 : numberValue - 10; minusButton.disabled=true; plusButton.disabled=true; numberContainer.classList.add(\"dim\"); const _serverActionResult=await _serverAction(newNumberValue); if (_serverActionResult !==newNumberValue || _serverActionResult===null){ return;} minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\"); numberValueContainer.textContent=_serverActionResult; if (newNumberValue===0){ minusButton.disabled=true; plusButton.disabled=false; numberContainer.classList.add(\"dim\"); pressedButton.blur();} else if (newNumberValue >0 && newNumberValue < 100){ minusButton.disabled=false; plusButton.disabled=false; numberContainer.classList.remove(\"dim\");} else if (newNumberValue===100){ minusButton.disabled=false; plusButton.disabled=true; pressedButton.blur();}} playerInputButtons.forEach((button)=>{ button.addEventListener(\"click\", async (event)=>{ const pressedButton=event.currentTarget; const previousButton=pressedButton.parentNode.querySelector(\".previous\"); const playButton=pressedButton.parentNode.querySelector(\".play\"); const pauseButton=pressedButton.parentNode.querySelector(\".pause\"); const stopButton=pressedButton.parentNode.querySelector(\".stop\"); const nextButton=pressedButton.parentNode.querySelector(\".next\"); let currentState={ previousButtonIsDisabled: previousButton.disabled, playButtonIsDisabled: playButton.disabled, pauseButtonIsDisabled: pauseButton.disabled, stopButtonIsDisabled: stopButton.disabled, nextButtonIsDisabled: nextButton.disabled}; previousButton.disabled=true; playButton.disabled=true; pauseButton.disabled=true; stopButton.disabled=true; nextButton.disabled=true; switch (pressedButton){ case previousButton: const previousResponse=await fetch(previousTrackUrl).catch(()=>{ return;}); if (previousResponse===undefined || !previousResponse.ok){ alert(\"Ошибка HTTP: \" + previousResponse?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break; case playButton: const playResponse=await fetch(playTrackUrl).catch(()=>{ return;}); if (playResponse===undefined || !playResponse.ok){ alert(\"Ошибка HTTP: \" + playResponse?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break; case pauseButton: const pauseResponse=await fetch(pauseTrackUrl).catch(()=>{ return;}); if (pauseResponse===undefined || !pauseResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=false; pauseButton.disabled=true; stopButton.disabled=false; nextButton.disabled=false;} break; case stopButton: const stopResponse=await fetch(stopTrackUrl).catch(()=>{ return;}); if (stopResponse===undefined || !stopResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=false; pauseButton.disabled=true; stopButton.disabled=true; nextButton.disabled=false;} break; case nextButton: const nextResponse=await fetch(nextTrackUrl).catch(()=>{ return;}); if (nextResponse===undefined || !nextResponse.ok){ alert(\"Ошибка HTTP: \" + response?.status); previousButton.disabled=currentState.previousButtonIsDisabled; playButton.disabled=currentState.playButtonIsDisabled;; pauseButton.disabled=currentState.pauseButtonIsDisabled;; stopButton.disabled=currentState.stopButtonIsDisabled;; nextButton.disabled=currentState.nextButtonIsDisabled;;} else{ previousButton.disabled=false; playButton.disabled=true; pauseButton.disabled=false; stopButton.disabled=false; nextButton.disabled=false;} break;}});}); </script></body></html>"));
}

void handle_changeThrottle() {
  int value = _server.arg(0).toInt();
  int pwmValue = 0;

  switch (value) {
    case 10:
      pwmValue = 100;
      _chimneyLowHighPeriod = 1000;
      break;
    case 20:
      pwmValue = 120;
      _chimneyLowHighPeriod = 900;
      break;
    case 30:
      pwmValue = 135;
      _chimneyLowHighPeriod = 800;
      break;
    case 40:
      pwmValue = 155;
      _chimneyLowHighPeriod = 700;
      break;
    case 50:
      pwmValue = 170;
      _chimneyLowHighPeriod = 600;
      break;
    case 60:
      pwmValue = 185;
      _chimneyLowHighPeriod = 500;
      break;
    case 70:
      pwmValue = 200;
      _chimneyLowHighPeriod = 400;
      break;
    case 80:
      pwmValue = 215;
      _chimneyLowHighPeriod = 300;
      break;
    case 90:
      pwmValue = 230;
      _chimneyLowHighPeriod = 200;
      break;
    case 100:
      pwmValue = 255;
      _chimneyLowHighPeriod = 100;
      break;
    default:
      pwmValue = 0;
      _chimneyLowHighPeriod = 2000;
      break;
  }

  analogWrite(EnginePin, pwmValue);

  String payload = "{ \"value\": " + String(value) + " }";
  Serial.println(payload);
  _server.send(200, "text/json", payload);
}

void handle_changeVolume() {
  int percent = _server.arg(0).toInt();
  int value = _playerStateComposition.changeVolume(percent);
  String payload = "{ \"value\": " + String(percent) + " }";
  Serial.println(payload);
  _server.send(200, "text/json", payload);
}

void handle_nextTrack() {
  _playerStateComposition.playNextTrackInQueue();
  _server.send(200, "text/json");
}

void handle_previousTrack() {
  _playerStateComposition.playPreviousTrackInQueue();
  _server.send(200, "text/json");
}

void handle_play() {
  _playerStateComposition.changeState(play);
  _server.send(200, "text/json");
}

void handle_pause() {
  _playerStateComposition.changeState(pause);
  _server.send(200, "text/json");
}

void handle_stop() {
  _playerStateComposition.changeState(stop);
  _server.send(200, "text/json");
}

void hsndle_checkBatteryStatus() {
  bool isBatteryGood = digitalRead(VoltageRxPin) == HIGH;
  String payload = "{ \"isBatteryGood\": " + String(isBatteryGood) + " }";
  Serial.println(payload);
  _server.send(200, "text/json", payload);
}
