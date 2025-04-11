#ifndef MAIN_H
#define MAIN_H

// Include necessary libraries
#include <Arduino.h>


struct HealthRegisterData {
    uint8_t sats_used;
    uint8_t sats_in_view;
    float HDOP;
    bool ovf;
    bool mg_n;
    bool acc_n;
    bool accel;
    bool gyro;
    bool mag;
    bool gps;
  };

// Function prototypes
void setup();
void loop();

#endif // MAIN_H