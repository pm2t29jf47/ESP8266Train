#include <DFPlayerMini_Fast.h>
#include "PlayerStateEnum.h"

class PlayerStateComposition {
private:
  unsigned long _playerTimer;
  bool _isPlayerWakedUp;
  DFPlayerMini_Fast _player;
  PlayerStateEnum _currentState = stop;
  PlayerStateEnum _newState;
  int _playerLoopRepeater = 0;
  int _currentTrackInQueue = 3;
  int _volume = 0;
  int _hornTrackNo;
  int _demoTrackNo;
  bool _isStateChanged;
  bool _isVolumeChanged;
  bool _isQueueTrackChanged;
  bool _isHornRequested = false;
  bool _isDemoRequested = false;
  int applyVolume();
  int applyCurrentTrackInQueue();
  PlayerStateEnum applyState();
public:
  int playerCommandDelay = 500;
  int playerLoopRepeatCount = 5;
  int playNextTrackInQueue();
  int playPreviousTrackInQueue();
  int changeVolume(int percent);
  void changeState(PlayerStateEnum state);
  void handlePlayer();
  void initializePlayer(DFPlayerMini_Fast player, int playerCommandDelay, int playerLoopRepeatCount, int currentTrackInQueue);
  void requestHorn(int hornTrackNo);
  void requestDemo(int demoTracnNo);
};
