#include <Arduino.h>
#include <unity.h>
#include "FS.h"
#include "SD.h"

void test_sd_card_initialization() {
    bool initialized = SD.begin();
    TEST_ASSERT_TRUE(initialized);
}

void test_sd_card_file_write() {
    File file = SD.open("/test.txt", FILE_WRITE);
    TEST_ASSERT_NOT_NULL(file);
    file.println("Test data");
    file.close();

    file = SD.open("/test.txt", FILE_READ);
    TEST_ASSERT_NOT_NULL(file);
    String content = file.readString();
    file.close();

    TEST_ASSERT_EQUAL_STRING("Test data\n", content.c_str());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_sd_card_initialization);
    RUN_TEST(test_sd_card_file_write);
    UNITY_END();
}

void loop() {
    // Empty loop for testing
}
