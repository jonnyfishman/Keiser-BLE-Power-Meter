#include <Arduino.h>
#include "Gear.h"

extern long minVal, maxVal;
extern long angle;
extern int gear_threshhold;
extern int gear;

void GyroClass::setupWire() {      
  Wire.begin(); 
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B); 
  Wire.write(0x01 ); 
  Wire.endTransmission(true); 
}

long GyroClass::readWire() {    
  Wire.beginTransmission(0x68); 
  //Wire.write(0x6B); 
  //Wire.write(0x01 );
  //delay(500);

  Wire.write(0x3B); 
  Wire.endTransmission(false); 
  Wire.requestFrom(0x68,6,true); 
  int16_t AcX=Wire.read()<<8|Wire.read(); 
  int16_t AcY=Wire.read()<<8|Wire.read(); 
  int16_t AcZ=Wire.read()<<8|Wire.read(); 
  //Wire.write(0x6B); 
  //Wire.write(0x02 ); 
  //Wire.endTransmission(true);   
  int xAng = map(AcX,265,402,-90,90); 
  int yAng = map(AcY,265,402,-90,90); 

//Serial.println( (atan2(-yAng, -xAng)+PI) );

  long z= (atan2(-yAng, -xAng)+PI) * 1000;

  if ( z > 1000 && z < 4000 ) {
    return z;
  }
  else {
    return angle;
  }
  
}



int GyroClass::getGear( ) {

     
  
     //calibrate max
      
      double offset = 10;
      
      int certainty_threshhold = gear_threshhold;//(current_cadence<90)?10:14; //the number of successful loops req before triggering
    
      
        long new_angle = GyroClass::readWire();


    
        if ( abs(new_angle - angle)  < offset ) {
          angle_certainty++;
        }
        else {
          angle_certainty<1?angle_certainty=0:angle_certainty--;
        }

        
        //Serial.print("Cert: ");Serial.print(angle_certainty);Serial.print(" Angle ");Serial.println(new_angle);

    
    if ( angle_certainty >= certainty_threshhold ) {
       
       angle_certainty = (int)(certainty_threshhold/2);
                 
              int tempGear = map(new_angle, minVal, maxVal, 0, 47);

              tempGear < 0 ? tempGear = 1 : tempGear = tempGear;
              tempGear > 47 ? tempGear = 46: tempGear = tempGear;
              
              int gearArray[48] = {8,8,8,9,9,9,9,9,9,10,10,10,10,10,10,11,11,11,11,11,11,12,12,12,12,12,12,13,13,13,13,13,13,14,14,14,14,14,14,15,15,15,15,15,15,16,16,16};

            gear = gearArray[tempGear];
              
    }

    angle = new_angle;
  
}
