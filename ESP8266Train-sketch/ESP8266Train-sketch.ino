#include <DFPlayerMini_Fast.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "PlayerStateComposition.h"
#include "HttpServerData.h"
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
HttpServerData _httpServerData;
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
  Serial.println(F("HTTP server initialized"));
}

void handle_OnConnect() {
  _server.send(200, "text/html", _httpServerData.GetStaticContent());
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
