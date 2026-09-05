#include "PMW3360.h"

// Pin definitions
#define NCS 7 // NCS
#define MOT 2 


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
  Serial.println("Starting, attempting SPI connections");

  // Initialize PMW3360 sensor with CPI = 1600
  // CPI can be adjusted: 800, 1200, 1600, 3200, etc.
  if (!sensor.begin(NCS, 1600)) {
    Serial.println("PMW3360 initialization failed! Check wiring.");
    while (1) {
      // Halt
    }
  }
  else{
    Serial.println("Sensor ready.");
  }


  
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