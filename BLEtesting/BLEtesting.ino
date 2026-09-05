#include "PMW3360.h"
#include <ArduinoBLE.h>

// Pin definitions
#define NCS_pin 7 // NCS
#define MOT_pin 2 

//Params
int CPI = 1600;

// PMW3360 sensor object
PMW3360 sensor;
// Flag set by motion interrupt
volatile bool motionDetected = false;

// Interrupt service routine for PMW3360 motion pin (falling edge)
// void motionISR() {
//   motionDetected = true;
// }

void setup() {
  delay(2000);
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open

  // Initialize PMW3360 sensor with CPI
  if (!sensor.begin(NCS_pin, CPI)) {
    Serial.println("initialization failure. Check wiring.");
    while (1) {}// Halt
  } else {
    Serial.println("Initialization success.");
  }

  //Init ble 

}

void loop() {
  // If the motion interrupt has fired, read the sensor and send movement
  if(digitalRead(MOT) == LOW){
    PMW3360_DATA data = sensor.readBurst(); 
    if (data.isMotion && data.isOnSurface) {
      Serial.print("X-Movement (dx): ");
      Serial.print(data.dx);
      Serial.print("Y-Movement (dy): ");
      Serial.println(data.dy);
    }
  }
  delay(10);

}