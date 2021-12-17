#include <Wire.h>

#define MPU6050_ADDR                        0x68
#define MPU6050_SIGNAL_PATH_RESET_REGISTER  0x6b
#define MPU6050_ACCEL_CONFIG_REGISTER       0x1c
#define MPU6050_ACCEL_OUT                   0x3b

#define OFFSET_LOOP                         100
#define LIMIT_LOOP                          200

#define LOWEST_GEAR     8
#define HIGHEST_GEAR    16
#define MIN_CADENCE     20
#define MAX_CADENCE     140

#include "PowerData.h"

class CalculatePower {

  private:
    float X_accel, Y_accel, Z_accel;
    float X_accel_offset, Y_accel_offset, Z_accel_offset;

    int limits[2] = {0,0};
    int setGearLimits(int pointer);
    
    void readRaw();
    void getOffsets();

    int angle;
    int gear = LOWEST_GEAR;

public:

  int getAngle(){ return angle; };
  int getGear(){ return gear; };

  int setLowGear(){ return setGearLimits(0); };
  int setHighGear(){ return setGearLimits(1); };
  int CadenceToPower(int cadence);
  
  void begin();
  void update();
  
};
