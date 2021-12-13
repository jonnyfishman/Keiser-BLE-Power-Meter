#include <bluefruit.h>

class SetupGATT {
public:
  void CyclePower();
  void CycleGear();  
  void Advertising();
  void Config();
  static void connect_callback(uint16_t conn_handle);
  static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
  static void keiser_setup_adjust(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);
};

class BLENotify {
  public:
  void Battery();
  void Gear(int, long, long, int, int, int);
  //void Angle(int);  
  void Power(uint16_t, uint16_t, uint16_t);
};
