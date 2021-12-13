#include "BLEService.h"

extern bool lockGear;
extern void calibrate(bool setMin);
extern bool calibrating;
extern bool is_connected;
extern int pcnt;
extern int gear_threshhold;

#include "BatteryService.h"   
BatteryClass batteryclass;

uint16_t UUID16_KEISER_SETUP = 0xFF02;//{0x29,0x9f,0xee,0x7d,0xfb,0xfb,0x72,0xd0,0xe9,0x19,0xd8,0x5d,0x4d,0xfc,0xeb,0x46};//custom service 46ebfc4d 5dd8 19e9 d072 fbfb7dee9f29 LSB
uint16_t UUID16_CH_KEISER_GEAR = 0xFFFA;//{0x29,0x9f,0xee,0x7d,0xfb,0xfb,0x72,0xd0,0xe9,0x19,0xd8,0x5d,0x4e,0xfc,0xeb,0x46};//custom service 46ebfc4d 5dd8 19e9 d072 fbfb7dee9f29 LSB
uint16_t UUID16_CH_KEISER_ADJUST = 0xFFFB;//{0x29,0x9f,0xee,0x7d,0xfb,0xfb,0x72,0xd0,0xe9,0x19,0xd8,0x5d,0x50,0xfc,0xeb,0x46};//custom service 46ebfc4d 5dd8 19e9 d072 fbfb7dee9f29 LSB
uint16_t UUID16_CH_KEISER_ANGLE = 0XFFFC;//{0x29,0x9f,0xee,0x7d,0xfb,0xfb,0x72,0xd0,0xe9,0x19,0xd8,0x5d,0x4f,0xfc,0xeb,0x46};//custom service 46ebfc4d 5dd8 19e9 d072 fbfb7dee9f29 LSB
                                 
BLEService        cps = BLEService(UUID16_SVC_CYCLING_POWER);
BLEService        cus = BLEService(UUID16_KEISER_SETUP); 
BLECharacteristic cpmc = BLECharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT);
BLECharacteristic cpfc = BLECharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE);
BLECharacteristic cplc = BLECharacteristic(UUID16_CHR_SENSOR_LOCATION);
BLECharacteristic cusg = BLECharacteristic(UUID16_CH_KEISER_GEAR);
BLECharacteristic cusw = BLECharacteristic(UUID16_CH_KEISER_ADJUST);
BLECharacteristic cusa = BLECharacteristic(UUID16_CH_KEISER_ANGLE);

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

void SetupGATT::CyclePower()
{
  cps.begin();

  cpfc.setProperties(CHR_PROPS_READ);
  cpfc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  //hrmc.setFixedLen(8);
  cpfc.begin();
  uint32_t cpfdata[] = { 0b00000000000000000000000000001000 }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  cpfc.write(cpfdata, sizeof(cpfdata) );

  cpmc.setProperties(CHR_PROPS_NOTIFY);
  cpmc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  //hrmc.setFixedLen(8);
  cpmc.begin();
  uint16_t cpmdata[] = { 0b00100000, 0x0000, 0x0000, 0x0000  }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  cpmc.write(cpmdata, sizeof(cpmdata) );

  cplc.setProperties(CHR_PROPS_READ);
  cplc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  cplc.setFixedLen(1);
  cplc.begin();
  cplc.write8(12);    // Set the characteristic to 'Wrist' (2)
}

void SetupGATT::CycleGear()
{
  cus.begin();

  cusg.setProperties(CHR_PROPS_NOTIFY);
  cusg.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  //hrmc.setFixedLen(8);
  cusg.begin();
  uint16_t cusgdata[] = { 0x00  }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  cusg.write(cusgdata, sizeof(cusgdata) );
/*
  cusa.setProperties(CHR_PROPS_NOTIFY);
  cusa.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  //hrmc.setFixedLen(8);
  cusa.begin();
  uint16_t cusadata[] = { 0x00  }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  cusa.write(cusadata, sizeof(cusadata) );
*/

  cusw.setProperties(CHR_PROPS_WRITE);
  cusw.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  uint16_t cuswdata[] = { 0x00  }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  cusw.setWriteCallback(keiser_setup_adjust);
  cusw.begin();
  cusw.write(cuswdata, sizeof(cuswdata) );  
  
}

void SetupGATT::keiser_setup_adjust(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
    digitalWrite(LED_RED,HIGH);
    // Display the raw request packet
    Serial.print("CCCD Updated: ");
    //Serial.printBuffer(request->data, request->len);
    Serial.printf("Data: %i %i %i %i %i %i %i %i\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
/*
    if ( data[0] == 102 ) {
      Serial.println("Set Fine");
       Mode = 1;
       delay(120);
       digitalWrite(LED_RED,HIGH);
       delay(120);
       digitalWrite(LED_RED,LOW);
    }
    else if ( data[0] == 98 ) {
      Serial.println("Set Broad");
      Mode = 2;
       delay(120);
       digitalWrite(LED_RED,HIGH);
       delay(120);
       digitalWrite(LED_RED,LOW);
    }
    */
    if ( data[0] == 110 ) {
      Serial.println("Disable");
      (lockGear == false)?lockGear=true:lockGear=false;
       delay(120);
       digitalWrite(LED_RED,HIGH);
       delay(120);
       digitalWrite(LED_RED,LOW);
    }
    else if ( data[0] == 99 ) {
      Serial.println("recalibrate");
      calibrating = true;
      calibrate(true);
      digitalWrite(LED_RED,HIGH);
      delay(6000);
      calibrate(false);
      calibrating = false;
    }
    else if ( data[0] == 117 ) {
      Serial.println("offset up");
      gear_threshhold++;
    }
    else if ( data[0] == 100 ) {
      Serial.println("offset down");
      gear_threshhold--;
    }
        
    digitalWrite(LED_RED,LOW);
}


void SetupGATT::Advertising(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
 Serial.println(Bluefruit.getTxPower());
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_CYCLING_POWER_SENSOR);
  // Include HRM Service UUID

  Bluefruit.Advertising.addService(cps);

  // Include Name
  Bluefruit.Advertising.addName();
  
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

void SetupGATT::Config() {

  Serial.println("Bluefruit52 HRM Example");
  Serial.println("-----------------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  // off Blue LED for lowest power consumption
  //Bluefruit.autoConnLed(false);
  Bluefruit.setConnLedInterval(4000);
  Bluefruit.setTxPower( (int8_t)6 );    // Check bluefruit.h for supported values

  // Set the advertised device name (keep it short!)
  Serial.println("Setting Device Name to 'Feather52 HRM'");
  Bluefruit.setName("Kicker");
  Bluefruit.setAppearance(BLE_APPEARANCE_CYCLING_POWER_SENSOR);

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Wahoo");
  bledis.setModel("Keiser M0");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  
  blebas.write(pcnt);
  
  
}

void SetupGATT::connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
  is_connected = true;
}

void SetupGATT::disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Serial.println("Advertising!");

  is_connected = false;
}
         
void BLENotify::Battery() {
  pcnt = batteryclass.getPCNT();
  //Serial.print("Batt: ");Serial.println(pcnt);
  blebas.write(pcnt);
}

void BLENotify::Power(uint16_t pwr, uint16_t total_rev, uint16_t event_time) {
    
    uint16_t time_resolution = abs( event_time * 1.02  );
    uint16_t cpmdata[] = { 0b00100000, pwr, total_rev, time_resolution }; //%65536  }; // Set the characteristic to use 8-bit values, with the sensor connected and detected

    cpmc.notify(cpmdata, sizeof(cpmdata) );
    //cpmc.write(cpmdata, sizeof(cpmdata) );
    
}

void BLENotify::Gear(int gear, long pwr, long angle, int cad, int pcnt, int offset) {
    
    uint8_t cusgdata[] = { gear, pwr, pwr >> 8, angle, angle >> 8, cad, pcnt, offset }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
    
    cusg.notify(cusgdata, sizeof(cusgdata) );
    
}
/*
void BLENotify::Angle(int angle) {
    
    
    uint8_t cusadata[] = { angle }; // Set the characteristic to use 8-bit values, with the sensor connected and detected

    //cusa.notify(cusadata, sizeof(cusadata) );
    cusa.write(cusadata, sizeof(cusadata) );
    
}
*/
