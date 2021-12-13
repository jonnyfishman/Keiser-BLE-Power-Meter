#include <Wire.h>

class GyroClass {
public:
  int angle_certainty = 0;
  void setupWire();
  long readWire();
  int getGear();

};
