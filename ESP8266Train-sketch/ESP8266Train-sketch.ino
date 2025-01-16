#include <DFPlayerMini_Fast.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "PlayerStateComposition.h"
#include "HttpServerData.h"
#include "ThrottleWrapper.h"
#define AP_SSID "furyTrain"
#define ChimneyPin 5
#define EnginePin 4
#define PlayerRxPin 12
#define PlayerTxPin 14
#define VoltageRxPin 13

SoftwareSerial _playerSerial(PlayerRxPin, PlayerTxPin);
DFPlayerMini_Fast _player;
ESP8266WebServer _server(80);
HttpServerData _httpServerData;
PlayerStateComposition _playerStateComposition;
ThrottleWrapper _throttleWrapper;

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
  _throttleWrapper.handleChimney();
  _playerStateComposition.handlePlayer();
}

void InitializePlayer() {
  _playerSerial.begin(9600);
  if (!_player.begin(_playerSerial, false, 500)) {
    Serial.println(F("Player NOT initialized"));
    while (true) {
      delay(0);
    }
  }
  _playerStateComposition.initializePlayer(_player, 500, 5, 3);
  Serial.println(F("Player initialized"));
}

void InitializeChimney() {
  _throttleWrapper.initializeChimney(ChimneyPin);
  Serial.println(F("Chimney initialized"));
}

void InitializeEngine() {
  _throttleWrapper.initializeEngine(EnginePin);
  Serial.println(F("Engine initialized"));
}

void InitializeWiFi() {
  WiFi.begin("furycat", "j9S9wRkt$*Pj");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println(F("WiFi initialized"));
  Serial.println(WiFi.localIP());
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
  _throttleWrapper.applyThrottle(value);
  _throttleWrapper.handleEngine();
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
