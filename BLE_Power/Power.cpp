#include <Arduino.h>
#include "Power.h"

long minVal, maxVal;
float angle;

float X_accel_offset, Y_accel_offset, Z_accel_offset;

extern int gear_threshhold;
extern int gear;

void CalculatePower::begin() {                         
  Wire.begin(); 
  Wire.beginTransmission(MPU6050_ADDR);               // Contact MPU6050
  Wire.write(MPU6050_SIGNAL_PATH_RESET_REGISTER);     // Power management register
  Wire.write(0x01 );                                  // Send reset code
  Wire.endTransmission(false); 
  Wire.write(MPU6050_ACCEL_CONFIG_REGISTER);          // Accel config register
  Wire.write(0x18);                                   // Set range to +-16G to smooth motion somewhat
  Wire.endTransmission(true); 

  this->getOffsets();                                 // Reduce error (https://www.digikey.co.uk/en/articles/using-an-accelerometer-for-inclination-sensing)
}

void CalculatePower::getOffsets() {                         
  float accumulated_raw[3] = {0,0,0};                 
  
  for(int i = 0; i < OFFSET_LOOP; i++){
    this->readRaw();
    accumulated_raw[0] += X_accel;
    accumulated_raw[1] += Y_accel;
    accumulated_raw[2] += Z_accel;
    delay(1);                                         // Short delay to get different values
  }

  // Get average position
  X_accel_offset = X_accel / OFFSET_LOOP;
  Y_accel_offset = Y_accel / OFFSET_LOOP;
  Z_accel_offset = Z_accel / OFFSET_LOOP;
}

int CalculatePower::setGearLimits(int pointer) {                         
  
  int accumulated_angles = 0;                         // This does the same as getOffsets but with the angle 
  
  for(int i = 0; i < LIMIT_LOOP; i++){
    this->update();
    accumulated_angles += angle;
    delay(1); 
  }

  return limits[pointer] = accumulated_angles / LIMIT_LOOP;
}

void CalculatePower::readRaw() {    
  Wire.beginTransmission(MPU6050_ADDR); 
  
  Wire.write(MPU6050_ACCEL_OUT);                      // Start receiving accelerator values
  Wire.endTransmission(false);                        // Send stop message but remain in control
  Wire.requestFrom(MPU6050_ADDR,6,true);              // Request X,Y and Z data 

  int16_t rawData[3];       
  
  for(int i=0;i<3;i++){
    rawData[i] =  Wire.read()<<8|Wire.read();         // Get rawdata and merge H and L bit
  }

  Wire.endTransmission(true);                         
  
  X_accel =  (float)rawData[0] / 2048.0;              // Convert raw data +-16G needs / 2048
  Y_accel =  (float)rawData[1] / 2048.0;
  Z_accel =  (float)rawData[2] / 2048.0;

  
}

void CalculatePower::update() {
  this->readRaw();
  
    // Calculations from https://www.digikey.co.uk/en/articles/using-an-accelerometer-for-inclination-sensing
    // This calc takes into account the offset error. readRaw does not    
    
    // Range is -90 to +90 because of the tan function being used
    // To get range -180 to 180 Z component is included (https://www.i2cdevlib.com/forums/topic/24-roll-and-pitch-angles-ranges/?do=findComment&comment=843)
    // Get angle and convert to degs * 10 for greater resolution in map function (time by 572.9578 not 57.29578)
  int z_component = (Z_accel-Z_accel_offset) > 1 ? 1 : -1; 
  angle = atan2( (Y_accel-Y_accel_offset), z_component * sqrt( ((X_accel-X_accel_offset)*(X_accel-X_accel_offset)) + ((Z_accel-Z_accel_offset)*(Z_accel-Z_accel_offset)) ) ) * 572.9578; 
  
    // Map angle to a gear
  int tempGear = map( angle, limits[0], limits[1], LOWEST_GEAR, HIGHEST_GEAR);
  tempGear < LOWEST_GEAR ? tempGear = LOWEST_GEAR : tempGear = tempGear;          // Prevent gear going higher or lower than limits
  tempGear > HIGHEST_GEAR ? tempGear = HIGHEST_GEAR: tempGear = tempGear;
            
  gear = tempGear;
  
}

int CalculatePower::CadenceToPower(int cadence) {

  int power;

  if ( cadence > MAX_CADENCE || cadence < MIN_CADENCE ) { // Disregard and cadence outside of those mapped into PowerData
    power = 0;
    return power;
  }

  switch(gear){                                           // Call correct array for result. This is quicker than using a formula
  case 8: 
    power = pgm_read_word( &GEAR8[cadence] );
    break;
  case 9: 
    power = pgm_read_word( &GEAR9[cadence] );
    break;
  case 10: 
    power = pgm_read_word( &GEAR10[cadence] );
    break;
  case 11: 
    power = pgm_read_word( &GEAR11[cadence] );
    break;
  case 12: 
    power = pgm_read_word( &GEAR12[cadence] );
    break;
  case 13: 
    power = pgm_read_word( &GEAR13[cadence] );
    break;
  case 14: 
    power = pgm_read_word( &GEAR14[cadence] );
    break;
  case 15: 
    power = pgm_read_word( &GEAR15[cadence] );
    break;
  case 16: 
    power = pgm_read_word( &GEAR16[cadence] );
    break;                           
  default:                                                // Assume gear is somehow out of bounds 
    power = 0;
    break;
  }

  return power;
  
}
