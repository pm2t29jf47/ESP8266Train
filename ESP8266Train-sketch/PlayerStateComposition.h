#include "PlayerStateEnum.h"

class PlayerStateComposition {
public:
  PlayerStateEnum currentState = stop;
  PlayerStateEnum newState;
  int currentTrackInQueue = 3;
  int volume = 0;
  bool isStateChanged;
  bool isVolumeChanged;
  bool isQueueTrackChanged;

  int applyCurrentTrackInQueue();
  int applyVolume();
  int playNextTrackInQueue();
  int playPreviousTrackInQueue();
  int changeVolume(int percent);
  void changeState(PlayerStateEnum state);
  PlayerStateEnum applyState();
};

