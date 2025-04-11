#include <Arduino.h>
#include <unity.h>
#include <WiFi.h>
#include "config.h"

void test_wifi_connection() {
    WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
    }
    TEST_ASSERT_EQUAL(WL_CONNECTED, WiFi.status());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_wifi_connection);
    UNITY_END();
}

void loop() {
    // Empty loop for testing
}
