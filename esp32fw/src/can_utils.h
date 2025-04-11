#ifndef CAN_UTILS_H
#define CAN_UTILS_H

#include <stdint.h>

// Simple CAN frame structure
typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
} CANFrame;

// Example: create an engine RPM frame
static inline CANFrame createEngineRPMFrame() {
    CANFrame frame = {0x100, 2, {0}};
    frame.data[0] = 0;
    frame.data[1] = 0;
    return frame;
}

// Example: create a vehicle speed frame
static inline CANFrame createVehicleSpeedFrame() {
    CANFrame frame = {0x101, 1, {0}};
    frame.data[0] = 0;
    return frame;
}

// Example: create an engine temperature frame
static inline CANFrame createEngineTempFrame() {
    CANFrame frame = {0x102, 1, {0}};
    frame.data[0] = 0;
    return frame;
}
static inline CANFrame createGetVinFrame() {
    CANFrame frame = {0x7DF, 3, {0}};
    frame.data[0] = 0x02; // Number of data bytes
    frame.data[1] = 0x09; // Service: "Request Vehicle Info"
    frame.data[2] = 0x02; // PID for VIN
    return frame;
}
#endif // CAN_UTILS_H

