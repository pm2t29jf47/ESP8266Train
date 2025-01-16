class ThrottleWrapper {
private:
  int _pwmValue = 0;
  int _chimneyLowHighPeriod = 0;
  int _chimneyPin;
  int _enginePin;
  bool _isChimneyLow = true;
  unsigned long _chimneyTimer;
public:
  unsigned long chimneyTimer;
  void applyThrottle(int percent);
  void initializeChimney(int chimneyPin);
  void handleChimney();
  void initializeEngine(int enginePin);
  void handleEngine();
};