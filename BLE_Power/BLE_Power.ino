#include <bluefruit.h>

#include "Gear.h"
GyroClass gyroclass;

#include "BLEService.h"
SetupGATT setupgatt;
BLENotify blenotify;

#include "geardata.h"

int cadence = 0;

  long minVal, maxVal;
  long angle;
  
  int gear = 16;
  bool lockGear = false;
  uint16_t pwr;
  int pcnt=100;
  int timing_loop = 0;
  bool calibrating = false;
  
unsigned long last_event_time = 0;
    long total_revolutions = 0;

    int gear_threshhold = 10;

bool is_connected = false;


const int hallPin = 27;     // the number of the hall effect sensor pin


void setup()
{
  Serial.begin(115200);

  pinMode(hallPin, INPUT);     
  gyroclass.setupWire();
digitalWrite(LED_RED,HIGH);

  Serial.println("Setup max/min values");
  
  //delay(2000);


calibrate(true);
digitalWrite(LED_RED,HIGH);

delay(6000);
calibrate(false);


  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  setupgatt.Config();
  Serial.println("Configuring the Cycle Power Service");
  setupgatt.CyclePower();  
  setupgatt.CycleGear();  
  
  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  setupgatt.Advertising();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");
  digitalWrite(LED_RED,LOW);
}



void loop()
{
  
  if (is_connected == true && calibrating == false) {

//           Serial.println(millis());        
          if ( digitalRead(hallPin) == LOW ) {   // millis()%428 == 0 
                     
          
          unsigned long time_diff = millis() - last_event_time; //check that we are still pedaling
          //Serial.print("time_diff ");Serial.println(time_diff);

          //Serial.print("rev_diff ");Serial.println(revolutions_diff);
          int new_cadence = 61000 / time_diff;  //(60000 / time_diff);


          if ( new_cadence < 140 ) {
                    int cadence_offset = 0;
                    total_revolutions++;
                    cadence  = new_cadence + cadence_offset;
                    last_event_time = millis();
          }
            digitalWrite(LED_RED,HIGH);
            //Serial.println("REV");
        
            delay(5);
            digitalWrite(LED_RED,LOW);
            
            while ( digitalRead(hallPin) == LOW ) {

              //wait until event finishes
            }
          
            //Serial.print("cadence ");Serial.println(cadence);
                    
          }
          else if ( millis() - last_event_time > 6000 ) {  //after 6 seconds of inaction
              cadence = 0;
            
          }
            /*
          Serial.print("Event Time ");Serial.print(event_time);
          Serial.print(" Time_Diff ");Serial.println(time_diff);
          */
        
          
          //Serial.print("Cadence ");Serial.println(cadence);  
            

        if ( millis()%100 == 0 ) { //every tenth of a second
            timing_loop++;
            //Serial.println(timing_loop);
              
            
            
            if ( timing_loop%15 == 0 ) {
                //Serial.println("notify");
                  blenotify.Gear( gear, pwr, angle, cadence, pcnt, gear_threshhold );
              
            }
            else if ( timing_loop%10 == 0 ) {
                //Serial.println("pwr");
          
            pwr = calcPwr(cadence, gear);
            blenotify.Power(pwr, total_revolutions, last_event_time );
            
              //Serial.print("Gear ");Serial.print(gear);
              //Serial.print(" Angle ");Serial.println(angle);
              //Serial.print("Gear: ");Serial.print(gear);  Serial.print(" Cadence: ");Serial.print(cadence); Serial.print(" Power: ");Serial.println(pwr);      
            }
            else if ( timing_loop%2 == 0 ) {
              if ( lockGear == false ) {
                gyroclass.getGear( );      //add app option to change threshhold
              }
            }
            
            if ( timing_loop == 60 ) {
                 blenotify.Battery();
                timing_loop = 0;
            }
         }
          delay(1);
  }
} 

uint16_t calcPwr(int current_cadence, int current_gear) {

  if ( current_cadence != 0 && current_cadence <= 140 ) {
    //pwr = (cadence * cadence * (gear2power[gear][0]/100) ) + ( cadence * (gear2power[gear][1]/100) ) + (gear2power[gear][2]);
    //pwr<0?pwr=0:pwr=pwr;
    if ( current_gear == 8 ) {
      pwr = pgm_read_word( &GEAR8[current_cadence] );
    }
    if ( current_gear == 9 ) {
      pwr = pgm_read_word( &GEAR9[current_cadence] );
    }
    if ( current_gear == 10 ) {
      pwr = pgm_read_word( &GEAR10[current_cadence] );
    }    
    if ( current_gear == 11 ) {
      pwr = pgm_read_word( &GEAR11[current_cadence] );
    }
    if ( current_gear == 12 ) {
      pwr = pgm_read_word( &GEAR12[current_cadence] );
    }
    if ( current_gear == 13 ) {
      pwr = pgm_read_word( &GEAR13[current_cadence] );
    }
    if ( current_gear == 14 ) {
      pwr = pgm_read_word( &GEAR14[current_cadence] );
    }
    if ( current_gear == 15 ) {
      pwr = pgm_read_word( &GEAR15[current_cadence] );
    }
    if ( current_gear == 16 ) {
      pwr = pgm_read_word( &GEAR16[current_cadence] );
    }
    

    
  }
  else if ( current_cadence > 140 ) {
    
      pwr = pwr;
    
  }
  else if ( current_cadence == 0 ) {
    pwr = 0;
  }
  
  
  
  
  return pwr;
}

void calibrate(bool setMin) {

  

     //calibrate max
      int certainty = 0;
      double offset = 10;
      int sway = 50; //check that there hasn't been sudden big change in angle
      int certainty_threshhold = 16; //the number of successful loops req before triggering
    
      while ( certainty < certainty_threshhold ) {
        long new_angle = gyroclass.readWire();
    
        if ( abs(new_angle - angle)  < offset && abs(new_angle - angle) < sway ) {
          certainty++;
        }
        else {
          certainty<1?certainty=0:certainty--;
        }
    Serial.println(certainty);
          Serial.print("Angle ");Serial.println(angle);
        digitalToggle(LED_RED);
        delay(120);
        angle = new_angle;
      }
    

      if (setMin == true) {
        minVal = angle;
      }
      else {
        maxVal = angle;
      }
      digitalWrite(LED_RED,LOW);

  
      
}

//TODO
//power consumption on mpu6050
