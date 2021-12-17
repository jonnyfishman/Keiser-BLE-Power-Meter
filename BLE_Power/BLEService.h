// Bluefruit examples (https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/bluefruit-nrf52-api)
#include <bluefruit.h>

class SetupGATT {       // Configure BLE peripheral characteristics
    
  private: 
    
    static void connect_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
    
    static SetupGATT * instance;    
    static void write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len); 

    void non_static_callback(int data);
    int command_from_app = 0;
     
  public:
      
    int get_command(){ return command_from_app; };
    void clear_command(){ command_from_app = 0; };

    bool get_connected(){ return Bluefruit.connected(); }
    
    void Config();
    void Advertising();
    // Power Meter characteristics from https://github.com/oesmith/gatt-xml
    void CyclePower();    
    // Custom characteristics for setting and reading the angle and gear  
    void CycleSettings(); 
};

class BLENotify {           // Send data to peripheral charactersistics
  
  public:
 
    void Battery(int);
    void CyclePower(uint16_t power, uint16_t cumulative_revolutions, uint16_t event_time);
    void CycleSettings(int current_gear, long current_angle, int current_cadence, int battery_percent);
    
};
