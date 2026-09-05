#include <bluefruit.h>
#include "PMW3360.h"
//#include "BTfuncs.h"

// Pin definitions
#define NCS_pin 7 // NCS
#define MOT_pin 2 
// PMW3360 sensor object
PMW3360 sensor;
//Params
int CPI = 800;

//BLE init
BLEDis bledis; // device info 
BLEHidAdafruit blehid; // HID functions

//callbacks for bluetooth events
void connect_callback(uint16_t conn_handle){
  Serial.println("Connected!");
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  Serial.println("Disconnected!");
}


void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open
  Serial.println("Serial established.");

  //Init sensor
  if (!sensor.begin(NCS_pin, CPI)) {
    Serial.println("initialization failure. Check wiring.");
    while (1) {}// Halt
  } else {
    Serial.println("Initialization success.");
  }

  //Init bluetooth service
  Bluefruit.begin();
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.setName("NRF Mouse");

  bledis.begin();
  blehid.begin();
  Serial.println("BLE HID and DIS started");

  //Init advertising system - async after this (?)
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  Bluefruit.Advertising.start(0);


}

void loop() {
  // If the motion interrupt has fired, read the sensor and send movement

  if((digitalRead(MOT_pin) == LOW)&&(Bluefruit.connected())){
    PMW3360_DATA data = sensor.readBurst(); 
    if (data.isMotion && data.isOnSurface) {
      blehid.mouseMove(data.dx, data.dy);
    }
  }
}