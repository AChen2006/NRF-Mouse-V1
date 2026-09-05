#include <bluefruit.h>
#include "PMW3360.h"
//#include "BTfuncs.h"

//TODO redesign mount for better optics, and do electronics

// Pin definitions
#define NCS_pin 7 // NCS
#define MOT_pin 2 

#define M1 10
#define M2 0
#define SW_LT 9
#define SW_RT 1
#define SW_UP 2
#define SW_DN 3
#define SW_IN 4

// PMW3360 sensor object
PMW3360 sensor;
long dataX = 0;
long dataY = 0;
//Params
int CPI = 1600;

//BLE init
BLEDis bledis; // device info 
BLEHidAdafruit blehid; // HID functions

unsigned long timesince = 0;

//callbacks for bluetooth events
void connect_callback(uint16_t conn_handle){
  Serial.println("Connected!");

  // query connection interval
  BLEConnection* connection = Bluefruit.Connection(conn_handle);  
  if (connection) {
    // BLE units are 1.25ms
    uint16_t interval_units = connection->getConnectionInterval();
    // Convert to milliseconds
    float interval_ms = interval_units * 1.25f; // /?
    Serial.print("interval: ");
    Serial.println(interval_ms);
  }
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
    Serial.println("initialization failure.");
    while (1) {}// Halt
  } else {
    Serial.println("Initialization success.");
  }

  //Init bluetooth service
  Bluefruit.begin();
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.Periph.setConnInterval(6, 6);
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

  //check connection interval

}

int count = 0;
void loop() {
  // If the motion interrupt has fired, read the sensor and send movement
   
  if((digitalRead(MOT_pin) == LOW)&&(Bluefruit.connected())){
    PMW3360_DATA data = sensor.readBurst(); 
    if (data.isMotion && data.isOnSurface) {
      dataX += data.dx;
      dataY += data.dy;
      //Serial.println(data.SQUAL);
      count++;
    }
  }
  if(millis()>timesince + 30){
    blehid.mouseMove(dataX, dataY);
    dataX = 0;
    dataY = 0;
    Serial.println(count);
    count = 0;
  }
}