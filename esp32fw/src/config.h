#ifndef CONFIG_H
#define CONFIG_H
#define DEBUG 1

#include <Arduino.h>
#include <vector> // Include vector for the list of filtered IDs
#include <SD.h>

struct Config {
    bool wifiEnabled = true;
    String wifiSSID = "VG";
    String wifiPassword = "1111122222";
    String logFile = "/can_log.txt";
    String httpServer = "http://iot.giesl.cz/canbus_data";
    bool sdCardEnabled = true;
    int obdIIRequestInterval = 5000;
    int scrapeInterval = 10000;
    bool um7Enabled = true;
    bool canEnabled = true;
    bool canFilterEnabled = false; // Enable filtering
    std::vector<uint32_t> filteredCanIds = {0x100, 0x200, 0x300}; // Default filtered CAN IDs
};

extern Config config;


#endif // CONFIG_H

