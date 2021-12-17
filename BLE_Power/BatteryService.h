// Battery read information from (https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide?view=all)
// Put into a simple class

#include <Arduino.h>
           
#ifdef NRF52840_XXAA
#define  PIN_VBAT          A7             // A7 for feather nRF52832, A6 for nRF52840

#define VBAT_DIVIDER      (0.5F)          // 150K + 150K voltage divider on VBAT
#define VBAT_DIVIDER_COMP (2.0F)          // Compensation factor for the VBAT divider

#else
#define  PIN_VBAT          A6   

#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider
#endif

#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

#define MINIMUM_LPIO_VOLTAGE_MV 3300
#define MAXIMUM_LPIO_VOLTAGE_MV 3600


class FeatherBattery
{
  private:
    float mvolts;
    int percent = 100;
    
    void readVBAT();
    
public: 
    
    int getPercent();

};
