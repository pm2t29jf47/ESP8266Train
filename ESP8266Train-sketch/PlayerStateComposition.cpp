#include "PlayerStateComposition.h"

void PlayerStateComposition::initializePlayer(DFPlayerMini_Fast player, int playerCommandDelay, int playerLoopRepeatCount, int currentTrackInQueue) {
  _player = player;
  PlayerStateComposition::playerCommandDelay = playerCommandDelay;
  PlayerStateComposition::playerLoopRepeatCount = playerLoopRepeatCount;
  _currentTrackInQueue = currentTrackInQueue;
}

int PlayerStateComposition::applyCurrentTrackInQueue() {
  _isQueueTrackChanged = false;
  return _currentTrackInQueue;
}

int PlayerStateComposition::applyVolume() {
  _isVolumeChanged = false;
  return _volume;
}

int PlayerStateComposition::playNextTrackInQueue() {
  switch (_currentTrackInQueue) {
    case 3:
      _currentTrackInQueue = 4;
      break;
    case 4:
      _currentTrackInQueue = 5;
      break;
    case 5:
      _currentTrackInQueue = 3;
      break;
    default:
      _currentTrackInQueue = 3;
      break;
  }
  _isQueueTrackChanged = true;
  return _currentTrackInQueue;
}

int PlayerStateComposition::playPreviousTrackInQueue() {
  switch (_currentTrackInQueue) {
    case 3:
      _currentTrackInQueue = 5;
      break;
    case 4:
      _currentTrackInQueue = 3;
      break;
    case 5:
      _currentTrackInQueue = 4;
      break;
    default:
      _currentTrackInQueue = 5;
      break;
  }
  _isQueueTrackChanged = true;
  return _currentTrackInQueue;
}

int PlayerStateComposition::changeVolume(int percent) {
  switch (percent) {
    case 10:
      _volume = 3;
      break;
    case 20:
      _volume = 6;
      break;
    case 30:
      _volume = 9;
      break;
    case 40:
      _volume = 12;
      break;
    case 50:
      _volume = 15;
      break;
    case 60:
      _volume = 18;
      break;
    case 70:
      _volume = 21;
      break;
    case 80:
      _volume = 24;
      break;
    case 90:
      _volume = 27;
      break;
    case 100:
      _volume = 30;
      break;
    default:
      _volume = 0;
      break;
  }
  _isVolumeChanged = true;
  return _volume;
}

void PlayerStateComposition::changeState(PlayerStateEnum state) {
  _newState = state;
  _isStateChanged = true;
}

PlayerStateEnum PlayerStateComposition::applyState() {
  if (_currentState == stop && _newState == pause) {
    _currentState = stop;
  } else {
    _currentState = _newState;
  }
  _isStateChanged = false;
  return _currentState;
}

void PlayerStateComposition::requestHorn(int hornTrackNo) {
  _isHornRequested = true;
  _hornTrackNo = hornTrackNo;
  changeState(stop);
}

void PlayerStateComposition::requestDemo(int demoTrackNo) {
  _isDemoRequested = true;
  _demoTrackNo = demoTrackNo;
  changeState(stop);
}

void PlayerStateComposition::handlePlayer() {
  if (millis() - _playerTimer >= playerCommandDelay) {
    _playerTimer = millis();
    if (_isVolumeChanged) {
      if (_isPlayerWakedUp) {
        _player.volume(applyVolume());
        _isPlayerWakedUp = false;
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_isQueueTrackChanged) {
      if (_isPlayerWakedUp) {
        _player.play(applyCurrentTrackInQueue());
        _isPlayerWakedUp = false;
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_isStateChanged) {

      switch (_currentState) {
        case play:
          switch (_newState) {
            case play:
              if (_isPlayerWakedUp) {
                applyState();
                _player.play(_currentTrackInQueue);
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            case pause:
              if (_isPlayerWakedUp) {
                applyState();
                _player.pause();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            default:
              if (_isPlayerWakedUp) {
                applyState();
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
          switch (_newState) {
            case play:
              if (_isPlayerWakedUp) {
                applyState();
                _player.resume();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            case pause:
              if (_isPlayerWakedUp) {
                applyState();
                _player.pause();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            default:
              if (_isPlayerWakedUp) {
                applyState();
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
          switch (_newState) {
            case play:
              if (_isPlayerWakedUp) {
                applyState();
                _player.play(_currentTrackInQueue);
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
            default:
              if (_isPlayerWakedUp) {
                applyState();
                _player.stop();
                _isPlayerWakedUp = false;
              } else {
                _player.wakeUp();
                _isPlayerWakedUp = true;
              }
              break;
          }
          break;
      }
    } else if (_isHornRequested) {
      if (_isPlayerWakedUp) {
        _isHornRequested = false;
        _isPlayerWakedUp = false;
        _player.play(_hornTrackNo);
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_isDemoRequested) {
      if (_isPlayerWakedUp) {
        _isDemoRequested = false;
        _isPlayerWakedUp = false;
        _player.play(_demoTrackNo);
      } else {
        _player.wakeUp();
        _isPlayerWakedUp = true;
      }
    } else if (_currentState == play
               && !_isStateChanged
               && !_isQueueTrackChanged
               && !_player.isPlaying()) {
      if (!_player.isPlaying()) {
        _playerLoopRepeater++;
      } else {
        _playerLoopRepeater = 0;
      }
      if (_playerLoopRepeater >= playerLoopRepeatCount) {
        _playerLoopRepeater = 0;
        playNextTrackInQueue();
      }
    }
  }
}
