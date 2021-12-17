#include "BatteryService.h"

void FeatherBattery::readVBAT() {
  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(PIN_VBAT);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  mvolts = raw * REAL_VBAT_MV_PER_LSB;

  if(mvolts < MINIMUM_LPIO_VOLTAGE_MV) {
    mvolts = 0;
  }
  else if(mvolts < MAXIMUM_LPIO_VOLTAGE_MV) {
    mvolts -= MINIMUM_LPIO_VOLTAGE_MV;
    mvolts / 30;
  }
  else {
    mvolts -= MAXIMUM_LPIO_VOLTAGE_MV;
  }
  
  percent = 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666   

}

int FeatherBattery::getPercent() {
  this->readVBAT();
  return percent;
}
