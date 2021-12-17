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

#include "BLEService.h"

uint16_t UUID16_KEISER_SETUP = 0xFF02;  // Unclaimed UUIDs
uint16_t UUID16_CH_KEISER_GEAR = 0xFFFA;
uint16_t UUID16_CH_KEISER_ADJUST = 0xFFFB;

// Required Power Meter characteristics                                 
BLEService        cycle_power_service = BLEService(UUID16_SVC_CYCLING_POWER);
BLECharacteristic cycle_power_measurement = BLECharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT);
BLECharacteristic cycle_power_feature = BLECharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE);
BLECharacteristic cycle_power_sensor_location = BLECharacteristic(UUID16_CHR_SENSOR_LOCATION);

// Custom characteristics
BLEService        custom_service = BLEService(UUID16_KEISER_SETUP); 
BLECharacteristic custom_current_gear = BLECharacteristic(UUID16_CH_KEISER_GEAR);
BLECharacteristic custom_app_commands = BLECharacteristic(UUID16_CH_KEISER_ADJUST);

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

void SetupGATT::Config() {

  // Initialise the Bluefruit module
  DEBUG_PRINTLN("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  //Bluefruit.autoConnLed(false);         // Turn off Blue LED for lowest power consumption
  Bluefruit.setConnLedInterval(10);     // Slow down the connected blink to reduce power consumption
  Bluefruit.setTxPower( (int8_t)4 );      // 4db is max value

  // Set device name. This is what it will appear when searched
  Bluefruit.setName("Keiser");
  Bluefruit.setAppearance(BLE_APPEARANCE_CYCLING_POWER_SENSOR);

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  // Configure and Start the Device Information Service
  bledis.setManufacturer("Keiser");
  bledis.setModel("Keiser M0");
  bledis.begin();

  // Start the BLE Battery Service and set it to an initial value of 100%
  DEBUG_PRINTLN("Configuring the Battery Service");
  blebas.begin();  
  blebas.write(100);
  
  
}

void SetupGATT::Advertising(void) {
  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
 
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_CYCLING_POWER_SENSOR); // Advertise sensor as a power meter
  Bluefruit.Advertising.addService(cycle_power_service);                    // Connect to the correct characteristics

  Bluefruit.Advertising.addName();  // Include Name
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void SetupGATT::CyclePower() {

  // Create all of the Power Meter characteristics 
  cycle_power_service.begin();
  
  // https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.cycling_power_feature.xml
  // Defines the features of the power meter. 
  cycle_power_feature.setProperties(CHR_PROPS_READ);
  cycle_power_feature.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  cycle_power_feature.begin();
  // Full 32 bits included for clarity
  uint32_t cpfdata[] = { 0b00000000000000000000000000001000 }; // Defines our devices as using crank data
  cycle_power_feature.write(cpfdata, sizeof(cpfdata) );

  // https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.cycling_power_measurement.xml
  // Actually sends the power information
  cycle_power_measurement.setProperties(CHR_PROPS_NOTIFY);
  cycle_power_measurement.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  cycle_power_measurement.begin();
  uint16_t cpmdata[] = { 0b0000000000100000, 0, 0, 0  }; // Defines crank data present, although nothing is added until the first pedal strokes
  cycle_power_measurement.write(cpmdata, sizeof(cpmdata) );

  // https://github.com/sputnikdev/bluetooth-gatt-parser/blob/master/src/main/resources/gatt/characteristic/org.bluetooth.characteristic.sensor_location.xml
  cycle_power_sensor_location.setProperties(CHR_PROPS_READ);
  cycle_power_sensor_location.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  cycle_power_sensor_location.setFixedLen(1);
  cycle_power_sensor_location.begin();
  cycle_power_sensor_location.write8(12);    // 12 refers to 'Rear Wheel'
}

void SetupGATT::CycleSettings() {

  // Create characteristics that can be set by an app
  custom_service.begin();

  // Notify when gear changes
  custom_current_gear.setProperties(CHR_PROPS_NOTIFY);
  custom_current_gear.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  custom_current_gear.begin();
  uint16_t custom_current_geardata[] = { 0  }; // Set initial value as zero
  custom_current_gear.write(custom_current_geardata, sizeof(custom_current_geardata) );

  // Recieve commands from an app
  custom_app_commands.setProperties(CHR_PROPS_WRITE);
  custom_app_commands.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  uint16_t custom_app_commandsdata[] = { 0  }; 
  instance = this;  // Store this instance for the non_static_callback to access
  custom_app_commands.setWriteCallback( write_callback);
  custom_app_commands.begin();
  custom_app_commands.write(custom_app_commandsdata, sizeof(custom_app_commandsdata) );  
  
}

/* 
 *  The following functions send the notification data to the server
 */
  
void BLENotify::Battery(int battery_percent) {
  blebas.write(battery_percent);
}

// Length of the variables is taken from the characteristics above
void BLENotify::CyclePower(uint16_t power, uint16_t cumulative_revolutions, uint16_t event_time) {
    
    // First uint16_t shows as dec not bin
    uint16_t cpmdata[] = { 32, power, cumulative_revolutions, event_time }; //%65536  }; 

    cycle_power_measurement.notify(cpmdata, sizeof(cpmdata) );    
}

// Some of this information is duplicated as cannot subscribe to two characteristics?
void BLENotify::CycleSettings(int current_gear, long current_angle, int current_cadence, int battery_percent) {
    
    uint8_t current_geardata[] = { current_gear,  current_angle, current_angle >> 8, current_cadence, battery_percent }; // Most of the information is smaller than 8bits. Those that are bigger are split using the bitshift
    
    custom_current_gear.notify(current_geardata, sizeof(current_geardata) );  
}

/* 
 *  The following functions are the callbacks referenced above
 */


void SetupGATT::connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  // For debug purposes
  DEBUG_PRINT("Connected to ");
  DEBUG_PRINTLN(central_name);
}


void SetupGATT::disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  // For debug purposes
  DEBUG_PRINT("\nDisconnected, reason = 0x"); DEBUG_PRINTLN_HEX(reason);
  DEBUG_PRINTLN("Advertising!");
}

// This callback stores the returned command (a letter) back into the class so that is can be accessed in the main loop
void SetupGATT::non_static_callback(int data) {
    
    command_from_app = data;

    // For debug purposes
    DEBUG_PRINT("Data f: ");
    DEBUG_PRINTLN(command_from_app);
  
    
}

SetupGATT * SetupGATT::instance;
void SetupGATT::write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
      SetupGATT::instance->non_static_callback( data[0] );   // At present assumes that there is only one command at a time and that it is less than 8 bits long
       
}
