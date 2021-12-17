//#define DEBUG           
#ifdef DEBUG
  #define DEBUG_PRINT(x)        Serial.print(x)
  #define DEBUG_PRINTLN_HEX(x)  Serial.println(x,HEX)
  #define DEBUG_PRINTLN(x)      Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN_HEX(x)
  #define DEBUG_PRINTLN(x) 
#endif 

#define CALIBRATE_WAIT_MS   3000
#define HALL_EFFECT_SENSOR  27
#define LED_PIN             LED_RED
#define TIMING_ERROR        10

// Main BLE stack
#include <bluefruit.h>

// MPU6050 Power Data
#include "Power.h"
CalculatePower calculatepower;

// Power Meter BLE services
#include "BLEService.h"
SetupGATT setupgatt;
BLENotify blenotify;

// Battery BLE service
#include "BatteryService.h"   
FeatherBattery featherbattery;

// Variables used in main loop
int cadence = 0;
unsigned long tick = 0;
unsigned long last_event_time = 0;
unsigned long total_revolutions = 0;
uint16_t event_time = 0;

void setup() {
  Serial.begin(115200);

  pinMode(HALL_EFFECT_SENSOR, INPUT);               // Setup hall effect sensor to detect cadence
  
  calculatepower.begin();                           // Setup MPU6050

  DEBUG_PRINTLN("\nSet gear max and min values");   // Configure set points
  
    digitalWrite(LED_PIN,HIGH);
    delay(CALIBRATE_WAIT_MS);
    calculatepower.setLowGear();
    digitalWrite(LED_PIN,LOW);
    delay(CALIBRATE_WAIT_MS);
    digitalWrite(LED_PIN,HIGH);
    delay(CALIBRATE_WAIT_MS);
    digitalWrite(LED_PIN,LOW);
    calculatepower.setHighGear();

  while ( !Serial ) delay(10);                      // for nrf52840 with native usb

  DEBUG_PRINTLN("Configuring the Cycle Power Service");
  setupgatt.Config();                             
  setupgatt.CyclePower();                         
  setupgatt.CycleSettings();  
  
  // Setup the advertising packet(s)
  DEBUG_PRINTLN("Setting up the advertising payload(s)");
  setupgatt.Advertising();

  DEBUG_PRINTLN("Advertising!");
}

void loop() {

  if ( setupgatt.get_connected() == true ) {                         // Only send values if the peripheral is connected

          // Get cadence information
          
          if ( digitalRead(HALL_EFFECT_SENSOR) == LOW ) {           // Detect a full revolution

            digitalWrite(LED_PIN,HIGH);                             // Trigger visual for revolution
                               
          unsigned long time_diff = millis() - last_event_time;     // Time difference since last rotation (not event time yet)
          last_event_time = millis();
          
          /* 
           *  time_diff = 1 revolution
           *  time_diff / time_diff = 1 revolution / time_diff                                       (gives revolution / ms)
           *  time_diff / time_diff * 60000 (ms in a minute) = 1 revolution * 60000 / time_diff      (gives rev / min)
           */
          cadence = 60000 / time_diff;  
          
          // Add to cumulative value
          total_revolutions++;                                       

          /*
           *  In spec - Unit is in seconds with a resolution of 1/2048 
           *  so millis is multiplied by 1000 (to get seconds) and divided by 1024
           *  Spec accepts a 16bit value for the time so anything higher than 2^16 (65536) needs to rollover using modulus
           */
          event_time = ( millis() * 1000 / 1024 ) % 65536;  

            // Prevent getting repeated values for the same revolution i.e. if cadence sensor stops in front of the hall effect sensor
            while ( digitalRead(HALL_EFFECT_SENSOR) == LOW ) {    
              delay(5);
              //wait until event finishes
            }
            
            digitalWrite(LED_PIN,LOW);                            // Trigger end of visual
            
                    
          }
          else if ( millis() - last_event_time > 6000 ) {         // Return cadence to zero if the pedalling is stopped for 6 seconds
              cadence = 0;          
          }   // End of digitalRead(HALL_EFFECT_SENSOR) == LOW loop

        /* 
         *  Control the timing of the following events using millis() to detect every half second
         *  If sensor information is being missed then increase the TIMING ERROR
         *  Using old_tick ensure that each block only sends once
         *  If power meter response is sluggish, adjust these values to update the information more often
         */
        unsigned long old_tick = tick;        
        if (millis()%500 <= TIMING_ERROR) {               
          tick++;
        }

        // Send sensor information     
        if ( tick != old_tick && tick%2 == 0 ) {              // Send power data every second half second
            uint16_t power = calculatepower.CadenceToPower(cadence);      // Get power from cadence
            blenotify.CyclePower(power, total_revolutions, event_time );  
            
            DEBUG_PRINT("Cadence: " );DEBUG_PRINT(cadence);DEBUG_PRINT(" Power: " );DEBUG_PRINT(power);DEBUG_PRINT(" Total Revs: ");DEBUG_PRINT(total_revolutions);DEBUG_PRINT(" Event Time: ");DEBUG_PRINTLN(event_time);        
        }
        
        if ( tick != old_tick && tick%2 == 0 ) {                   // Send settings information every second tick i.e. 1s
            calculatepower.update();                  // Update gear and angle            
            uint16_t power = calculatepower.CadenceToPower(cadence);      // Get power from cadence
            int battery_percent = featherbattery.getPercent();            // Get battery value
            blenotify.CycleSettings( calculatepower.getGear(), calculatepower.getAngle(), cadence, battery_percent );
            
            DEBUG_PRINT("Gear: " );DEBUG_PRINT(calculatepower.getGear());DEBUG_PRINT(" Angle: ");DEBUG_PRINT(calculatepower.getAngle());DEBUG_PRINT(" Cadence: ");DEBUG_PRINTLN(cadence);
            
            // Check for a command from the app
            int command = setupgatt.get_command();
              if ( command > 0 ) {
                    
                    switch(command){                                           
                        case 'a': 
                          calculatepower.setLowGear();
                          
                          break;
                        case 'b': 
                          calculatepower.setHighGear();
                          
                          break;                          
                        default:                                                // Assume gear is somehow out of bounds 
                          
                          break;
                        }
                        
                     setupgatt.clear_command();
              }
            } 
            
            if ( tick != old_tick && tick%120 == 0 ) {                  // Send battery value every minute
                int battery_percent = featherbattery.getPercent();            // Get battery value
                blenotify.Battery( battery_percent );
                DEBUG_PRINT("Batery: ");DEBUG_PRINTLN(battery_percent);
            } 
         
          
          
  } // End of setupgatt.get_connected() == true loop
  
} 
