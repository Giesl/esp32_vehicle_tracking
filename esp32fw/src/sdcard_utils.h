#ifndef SDCARD_UTILS_H
#define SDCARD_UTILS_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"


#define SCK  14
#define MISO  12
#define MOSI  13
#define CS  23 //15



//#define TEST_FILE_IO 1


void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void testFileIO(fs::FS &fs, const char * path);
bool load_config(const char * path);
void test_sdcard();

#endif // SDCARD_UTILS_H