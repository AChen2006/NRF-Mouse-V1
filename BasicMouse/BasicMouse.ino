#include <bluefruit.h>
#include "PMW3360.h"
//#include "BTfuncs.h"

// Pin definitions
#define NCS_pin 7 // NCS
#define MOT_pin 2 // Burst ready signal pin

#define Nswitches 0
//M1, M2, M3, LEFT, RIGHT, UP, DOWN
bool buttonState[7] = {0};
int8_t buttonMap[7] = {6,5,3,2,4,1,0};
unsigned long lockTime[7] = {0};
const unsigned long lockInterval = 10; //10ms
unsigned long scrollTime = 0;
unsigned long panTime = 0;
const unsigned long scrollInterval = 200; //200ms
unsigned long lastReportTime = 0;

// PMW3360 sensor object
PMW3360 sensor;
//Params
int CPI = 1600;

const unsigned long reportInterval = 15; // this will depends on specific handshake, set this for minimum
//BLE init
BLEDis bledis; // device info 
BLEHidAdafruit blehid; // HID functions

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

  for (int i = 0; i<Nswitches; i++){
   pinMode(buttonMap[i], INPUT_PULLUP);
  }

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

hid_mouse_report_t report{0,0,0,0,0};

void loop() {
  if(Bluefruit.connected()){

    //switch state updater
    //change a switch state IF the input is different, and past the change timer
    //if not past the change timer, set the change timer to now
    for(int i = 0; i<Nswitches; i++){
      if((digitalRead(buttonMap[i]) != buttonState[i])&&(millis()>lockTime[i])){
        lockTime[i] = millis()+lockInterval;
        buttonState[i] = !buttonState[i];
      }
    }

    //update M1,2,3 for report
    for (int i = 0; i<3; i++){
      if(buttonState[i] == 0){
        report.buttons |= (1<<i);
      }
      else{
        report.buttons &= ~(1<<i);
      }
    }

    //update report wheel state
    //just sums button presses in case rapid switch happens during one report
    if(millis() > scrollTime){
      if(buttonState[5] == LOW){
        report.wheel++;
        scrollTime = millis()+scrollInterval;
      }
      if(buttonState[6] == LOW){
        report.wheel--;
        scrollTime = millis()+scrollInterval;
      }
    }

    //update report pan state
    if(millis() > scrollTime){
      if(buttonState[3] == LOW){
        report.pan++;
        scrollTime = millis()+scrollInterval;
      }
      if(buttonState[4] == LOW){
        report.pan--;
        scrollTime = millis()+scrollInterval;
      }
    }

    //get sensor burst data frame and report x and y - use an interrupt later on
    //this may have to run a bit async as there may be several bursts per report.
    if(digitalRead(MOT_pin) == LOW){
      PMW3360_DATA data = sensor.readBurst(); 
      if (data.isMotion && data.isOnSurface) {
        report.x += data.dx;
        report.y += data.dy;
        //Serial.println(data.SQUAL);
      }
    }

    //UPDATE CONDITION LATER
    if(millis()>lastReportTime+reportInterval){
      blehid.mouseReport(&report);
      lastReportTime = millis();
      report.wheel = 0;
      report.pan = 0;
      report.x = 0;
      report.y = 0;
    }

    Serial.println(digitalRead(buttonMap[0]));
    
  }
}