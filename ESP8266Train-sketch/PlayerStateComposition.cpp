#include "PlayerStateComposition.h"

  int PlayerStateComposition::applyCurrentTrackInQueue() {
    isQueueTrackChanged = false;
    return currentTrackInQueue;
  }

  int PlayerStateComposition::applyVolume() {
    isVolumeChanged = false;
    return volume;
  }

  int PlayerStateComposition::playNextTrackInQueue() {
    switch (currentTrackInQueue) {
      case 3:
        currentTrackInQueue = 4;
        break;
      case 4:
        currentTrackInQueue = 5;
        break;
      case 5:
        currentTrackInQueue = 3;
        break;
      default:
        currentTrackInQueue = 3;
        break;
    }
    isQueueTrackChanged = true;
    return currentTrackInQueue;
  }

  int PlayerStateComposition::playPreviousTrackInQueue() {
    switch (currentTrackInQueue) {
      case 3:
        currentTrackInQueue = 5;
        break;
      case 4:
        currentTrackInQueue = 3;
        break;
      case 5:
        currentTrackInQueue = 4;
        break;
      default:
        currentTrackInQueue = 5;
        break;
    }
    isQueueTrackChanged = true;
    return currentTrackInQueue;
  }

  int PlayerStateComposition::changeVolume(int percent) {
    switch (percent) {
      case 10:
        volume = 3;
        break;
      case 20:
        volume = 6;
        break;
      case 30:
        volume = 9;
        break;
      case 40:
        volume = 12;
        break;
      case 50:
        volume = 15;
        break;
      case 60:
        volume = 18;
        break;
      case 70:
        volume = 21;
        break;
      case 80:
        volume = 24;
        break;
      case 90:
        volume = 27;
        break;
      case 100:
        volume = 30;
        break;
      default:
        volume = 0;
        break;
    }
    isVolumeChanged = true;
    return volume;
  }

  void PlayerStateComposition::changeState(PlayerStateEnum state) {
    newState = state;
    isStateChanged = true;
  }

  PlayerStateEnum PlayerStateComposition::applyState() {
    if (currentState == stop && newState == pause) {
      currentState = stop;
    } else {
      currentState = newState;
    }
    isStateChanged = false;
    return currentState;
  }
