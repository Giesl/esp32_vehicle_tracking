#include "sdcard_utils.h"
#include "config.h"


SPIClass spi = SPIClass(HSPI);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);


    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);



    if (!SD.begin(CS, spi)) {
        Serial.println("SD card not mounted. Attempting to mount...");
        spi.begin(SCK, MISO, MOSI, CS);
        
        if (!SD.begin(CS, spi)) {
            Serial.println("Card Mount Failed");
            return;
        } else {
            Serial.println("SD card mounted successfully.");
        }
    }

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    Serial.printf("File size after appending: %u bytes\n", file.size());
    file.close();
    Serial.println("File closed after appending.");
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

bool load_config(const char* path) {
    Serial.println("Loading configuration from SD card...");

    
    spi.begin(SCK, MISO, MOSI, CS);
    
    if(!SD.begin(CS, spi)){
        Serial.println("Card Mount Failed");
        return false;
    }

    Serial.println("Opening file: " + String(path)); // Debugging line
    
    File file = SD.open(path, FILE_READ);
    

    if (!file) {
        Serial.println("Failed to open config file, using defaults.");
        return false;
    }
    Serial.println("Config file opened successfully.");

    while (file.available()) {
        // Read each line and parse the configuration
        String line = file.readStringUntil('\n');
        Serial.println("Line: " + line); // Debugging line
        line.trim();

        if (line.startsWith("WIFISSID=")) {
            config.wifiSSID = line.substring(9);
        } else if (line.startsWith("WIFIPASSWORD=")) {
            config.wifiPassword = line.substring(13);
        } else if (line.startsWith("WIFIENABLED=")) {
            Serial.println("FOUND WIFIENABLED: " + line.substring(12)); // Debugging line
            config.wifiEnabled = (line.substring(12) == "1");
        } else if (line.startsWith("HTTPSERVER=")) {
            config.httpServer = line.substring(11);
        } else if (line.startsWith("AUTHTOKEN=")) {
            config.authToken = line.substring(10);
        } else if (line.startsWith("SCRAPEINTERVAL=")) { // Parse HTTPSENDINTERVAL
            config.scrapeInterval = line.substring(17).toInt();
        } else if (line.startsWith("SDCARDENABLED=")) {
            config.sdCardEnabled = (line.substring(14) == "1");
        } else if (line.startsWith("OBDIISCRAPEINTERVAL=")) {
            config.obdIIRequestInterval = line.substring(18).toInt();
        } else if (line.startsWith("UM7ENABLED=")) {
            config.um7Enabled = (line.substring(11) == "1");
        } else if (line.startsWith("CANENABLED=")) {
            config.canEnabled = (line.substring(11) == "1");
        } else if (line.startsWith("CANFILTERENABLED=")) {
            config.canFilterEnabled = (line.substring(17) == "1");
        } else if (line.startsWith("FILTEREDCANIDS=")) { // New field for filtered IDs
            config.filteredCanIds.clear();
            String ids = line.substring(15);
            int start = 0, end;
            while ((end = ids.indexOf(',', start)) != -1) {
                config.filteredCanIds.push_back(ids.substring(start, end).toInt());
                start = end + 1;
            }
            if (start < ids.length()) {
                config.filteredCanIds.push_back(ids.substring(start).toInt());
            }
        } else if (line.startsWith("LOGFILE=")) { // Parse LOGFILE
            config.logFile = line.substring(8);
        }
    }

    file.close();

    Serial.println("Configuration loaded:");
    Serial.println("WiFi SSID: " + config.wifiSSID);
    Serial.println("WiFi Password: " + config.wifiPassword);
    Serial.println("WiFi Enabled: " + String(config.wifiEnabled));
    Serial.println("HTTP Server: " + config.httpServer);
    Serial.println("SD Card Enabled: " + String(config.sdCardEnabled));
    Serial.println("OBD II request interval: " + String(config.obdIIRequestInterval));
    Serial.println("UM7 Enabled: " + String(config.um7Enabled));
    Serial.println("CAN Enabled: " + String(config.canEnabled));
    Serial.println("CAN Filter Enabled: " + String(config.canFilterEnabled)); // Log new field
    Serial.println("Log File: " + config.logFile); // Log the new field
    Serial.print("Filtered CAN IDs: ");
    for (uint32_t id : config.filteredCanIds) {
        Serial.print("0x" + String(id, HEX) + " ");
    }
    Serial.println();

    return true;
}

void test_sdcard() {
    Serial.println("Initializing SD card...");
    spi.begin(SCK, MISO, MOSI, CS);
    
    if(!SD.begin(CS, spi)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);


    listDir(SD, "/", 0);
    
    //testFileIO(SD, "/config.txt");
    ////readFile(SD, "/config.txt");
    //
    //createDir(SD, "/mydir");
    //listDir(SD, "/", 0);
    //removeDir(SD, "/mydir");
    //listDir(SD, "/", 2);
    //writeFile(SD, "/hello.txt", "Hello ");
    //appendFile(SD, "/hello.txt", "World!\n");
    //readFile(SD, "/hello.txt");
    //renameFile(SD, "/hello.txt", "/foo.txt");
    //readFile(SD, "/foo.txt");
    //deleteFile(SD, "/foo.txt");
    //testFileIO(SD, "/test.txt");


}