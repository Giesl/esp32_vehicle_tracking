#include <Arduino.h>
#include <unity.h>
#include <vector>
#include "esp32_can.h"

extern std::vector<CAN_FRAME> canMessages;

void test_can_message_storage() {
    CAN_FRAME testFrame;
    testFrame.id = 0x123;
    testFrame.length = 8;
    memset(testFrame.data.byte, 0xAA, 8);

    canMessages.push_back(testFrame);

    TEST_ASSERT_EQUAL(1, canMessages.size());
    TEST_ASSERT_EQUAL(0x123, canMessages[0].id);
    TEST_ASSERT_EQUAL(8, canMessages[0].length);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL(0xAA, canMessages[0].data.byte[i]);
    }
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_can_message_storage);
    UNITY_END();
}

void loop() {
    // Empty loop for testing
}
