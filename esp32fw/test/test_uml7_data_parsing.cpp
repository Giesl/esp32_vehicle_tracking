#include <Arduino.h>
#include <unity.h>
#include "Redshift_UML7.h"
#include <main.h>

extern Redshift_UML7 uml7;

void test_uml7_health_data() {
    HealthRegisterData healthData;
    bool success = uml7.getHealthRegisterData(healthData.sats_used, healthData.sats_in_view, healthData.HDOP,
                                              healthData.ovf, healthData.mg_n, healthData.acc_n,
                                              healthData.accel, healthData.gyro, healthData.mag, healthData.gps);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_GREATER_OR_EQUAL(0, healthData.sats_used);
    TEST_ASSERT_GREATER_OR_EQUAL(0, healthData.sats_in_view);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_uml7_health_data);
    UNITY_END();
}

void loop() {
    // Empty loop for testing
}
