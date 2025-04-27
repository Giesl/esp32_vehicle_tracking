#include <esp32_can.h> /* https://github.com/collin80/esp32_can */
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include "sdcard_utils.h" // Include SD card utility functions
#include <HTTPClient.h>
#include "config.h"
#include "main.h"
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "Redshift_UML7.h" // Include Redshift_UML7 header

#define RXD2 22 // Define GPIO pin for UART1 RX
#define TXD2 21 // Define GPIO pin for UART1 TX
#define SHIELD_LED_PIN 26 // Define GPIO pin for CAN shield LED


HardwareSerial mySerial(1); // Use UART1 for communication
Redshift_UML7 uml7(mySerial);


// Create a list (vector) to store CAN messages using the existing CAN_FRAME structure
std::vector<CAN_FRAME> canMessages;




//const char* serverName = "http:/iot.giesl.cz/canbus_data";
const char* serverName = config.httpServer.c_str();


// Function prototypes
void printHealthData();

static bool lastButtonState = HIGH;

SemaphoreHandle_t sdCardMutex; // Mutex for SD card operations

void setupSerial() {
    mySerial.begin(115200, SERIAL_8N1, RXD2, TXD2); // Initialize UART1 with custom pins
}


void sendOBDRequests() {
  if (!config.canEnabled){
    return;
  }


    // Send standard OBD-II requests to CAN0
    CAN_FRAME requestFrame;
    requestFrame.id = 0x7DF; // Standard OBD-II request ID
    requestFrame.extended = false;
    requestFrame.length = 8;

    // Example: Request Engine RPM (PID 0x0C)
    requestFrame.data.byte[0] = 0x02; // Number of additional data bytes
    requestFrame.data.byte[1] = 0x01; // Service 01 (Current Data)
    requestFrame.data.byte[2] = 0x0C; // PID 0x0C (Engine RPM)
    requestFrame.data.byte[3] = 0x00;
    requestFrame.data.byte[4] = 0x00;
    requestFrame.data.byte[5] = 0x00;
    requestFrame.data.byte[6] = 0x00;
    requestFrame.data.byte[7] = 0x00;

    CAN0.sendFrame(requestFrame);
    delay(50); // Wait for a short time to allow the request to be processed
    Serial.println("Sent OBD-II request for Engine RPM.");

    // Example: Request Vehicle Speed (PID 0x0D)
    requestFrame.data.byte[2] = 0x0D; 
    CAN0.sendFrame(requestFrame);
    delay(50); // Wait for a short time to allow the request to be processed
    Serial.println("Sent OBD-II request for Vehicle Speed.");

    // Example: Request Throttle Position (PID 0x11)
    requestFrame.data.byte[2] = 0x11;
    CAN0.sendFrame(requestFrame);
    Serial.println("Sent OBD-II request for Throttle Position.");
}


void generateSampleCanFrames() {
  // Use the provided CAN message data to populate the vector
  struct {
    uint32_t id;
    uint8_t length;
    uint8_t data[8];
  } sampleMessages[] = {
    {0x100, 8, {0x02, 0x1A, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00}}, // Engine RPM
    {0x200, 8, {0x03, 0x41, 0x0C, 0x1A, 0xF8, 0x00, 0x00, 0x00}} // Vehicle Speed
  };

  for (const auto& msg : sampleMessages) {
    CAN_FRAME frame;
    frame.id = msg.id;
    frame.extended = false;
    frame.length = msg.length;
    memcpy(frame.data.byte, msg.data, msg.length);

    canMessages.push_back(frame); // Add the frame to the vector
  }

  Serial.println("Sample CAN frames added to the vector.");
}


void handle_can_message() {
    if (!config.canEnabled) {
        return;
    }

    CAN_FRAME can_message;
    if (!CAN0.read(can_message)) {
        //Serial.println("No CAN message received");
        //Serial.print("-");
        return;
    }

    // Apply CAN ID filter if enabled
    /*
    if (config.canFilterEnabled) {
        bool idAllowed = false;
        for (uint32_t id : config.filteredCanIds) {
            if (can_message.id == id) {
                idAllowed = true;
                break;
            }
        }
        if (!idAllowed) {
            Serial.println("Filtered out CAN message with ID: 0x" + String(can_message.id, HEX));
            return;
        }
    }
        */

    // Construct the CAN message string
    String message = "0x";
    message += String(can_message.id, HEX);
    message += " [";
    message += String(can_message.length, DEC);
    message += "] <";
    for (int i = 0; i < can_message.length; i++) {
        if (i != 0) message += ":";
        message += String(can_message.data.byte[i], HEX);
    }
    message += ">\n";
    // Print the message to the Serial Monitor
    Serial.println("CAN MESSAGE: " + message);

    canMessages.push_back(can_message); // Store the message in the vector
}

void sendPostRequest(const String& payload) {
  
  if (!config.wifiEnabled){
    return;
  }


  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, config.httpServer.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + config.authToken);
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println(response);
    } else {
      Serial.println("Error sending POST request");
    }
    http.end();
  } else {
    Serial.println("WiFi not connected, cannot send POST request");
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  IPAddress dnsServer(8, 8, 8, 8);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dnsServer);
  WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());

  unsigned long startTime = millis();
  unsigned long connectionTimeout = 20000; // 20 seconds

  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < connectionTimeout) {
    delay(500);
    Serial.print(".");
    digitalWrite(SHIELD_LED_PIN, !digitalRead(SHIELD_LED_PIN)); // Blink LED 26 while connecting to WiFi
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection timed out");
    digitalWrite(SHIELD_LED_PIN, LOW); // Turn off LED 26 if WiFi connection fails
  } else {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    digitalWrite(SHIELD_LED_PIN, HIGH); // Turn on LED 26 when WiFi is connected
  }
}



String getConfigJson(){
  String json = "{";
  json += "\"wifi_enabled\":" + String(config.wifiEnabled) + ",";
  json += "\"sd_card_enabled\":" + String(config.sdCardEnabled) + ",";
  json += "\"um7_enabled\":" + String(config.um7Enabled) + ",";
  json += "\"can_enabled\":" + String(config.canEnabled) + ",";
  json += "\"log_file\":\"" + config.logFile + "\",";
  json += "\"http_endpoint\":\"" + config.httpServer + "\",";
  json += "\"scrape_interval\":" + String(config.scrapeInterval) + ",";
  json += "\"obd_request_interval\":" + String(config.obdIIRequestInterval) + ",";
  json += "\"can_filter_enabled\":" + String(config.canFilterEnabled) + ",";
  json += "\"filtered_can_ids\":[";
  for (size_t i = 0; i < config.filteredCanIds.size(); i++) {
      json += "\"0x" + String(config.filteredCanIds[i], HEX);
      if (i < config.filteredCanIds.size() - 1) {
          json += "\",";
      }
      else {
          json += "\"";
      }
  }
  json += "]";
  json += "}";
  return json;
}

String getuml7MeasurementsJson(){

  String jsonPayload = "{";  
  HealthRegisterData healthData;
  if (uml7.getHealthRegisterData(healthData.sats_used, healthData.sats_in_view, healthData.HDOP,
    healthData.ovf, healthData.mg_n, healthData.acc_n,
    healthData.accel, healthData.gyro, healthData.mag, healthData.gps)) {
      jsonPayload += "\"health_data\":{";
      jsonPayload += "\"sats_used\":" + String(healthData.sats_used) + ",";
      jsonPayload += "\"sats_in_view\":" + String(healthData.sats_in_view) + ",";
      jsonPayload += "\"HDOP\":" + String(healthData.HDOP) + ",";
      jsonPayload += "\"ovf\":" + String(healthData.ovf) + ",";
      jsonPayload += "\"mg_n\":" + String(healthData.mg_n) + ",";
      jsonPayload += "\"acc_n\":" + String(healthData.acc_n) + ",";
      jsonPayload += "\"accel\":" + String(healthData.accel) + ",";
      jsonPayload += "\"gyro\":" + String(healthData.gyro) + ",";
      jsonPayload += "\"mag\":" + String(healthData.mag) + ",";
      jsonPayload += "\"gps\":" + String(healthData.gps);
      jsonPayload += "}";
    }
    else {
      jsonPayload += "\"health_data\":{}";
    }
    CarMovementMeasurements carMovementMeasurements;
    if (uml7.getCarMovementMeasurements(carMovementMeasurements)){
      //start of car_movement
      jsonPayload += ",\"car_movement\":{";
      jsonPayload += "\"accel_x\":" + String(carMovementMeasurements.accelX) + ",";
      jsonPayload += "\"accel_y\":" + String(carMovementMeasurements.accelY) + ",";
      jsonPayload += "\"accel_z\":" + String(carMovementMeasurements.accelZ) + ",";
      jsonPayload += "\"accel_time\":" + String(carMovementMeasurements.accelTime) + ",";
      jsonPayload += "\"gyro_x\":" + String(carMovementMeasurements.gyroX) + ",";
      jsonPayload += "\"gyro_y\":" + String(carMovementMeasurements.gyroY) + ",";
      jsonPayload += "\"gyro_z\":" + String(carMovementMeasurements.gyroZ) + ",";
      jsonPayload += "\"gyro_time\":" + String(carMovementMeasurements.gyroTime) + ",";
      jsonPayload += "\"mag_x\":" + String(carMovementMeasurements.magX) + ",";
      jsonPayload += "\"mag_y\":" + String(carMovementMeasurements.magY) + ",";
      jsonPayload += "\"mag_z\":" + String(carMovementMeasurements.magZ) + ",";
      jsonPayload += "\"mag_time\":" + String(carMovementMeasurements.magTime) + ",";
      jsonPayload += "\"roll\":" + String(carMovementMeasurements.roll) + ",";
      jsonPayload += "\"pitch\":" + String(carMovementMeasurements.pitch) + ",";
      jsonPayload += "\"yaw\":" + String(carMovementMeasurements.yaw) ;
      

      // GPS data
      jsonPayload += ",\"gps\":{";
      jsonPayload += "\"latitude\":" + String(carMovementMeasurements.gpsLatitude, 6) + ",";
      jsonPayload += "\"longitude\":" + String(carMovementMeasurements.gpsLongitude, 6) + ",";
      jsonPayload += "\"altitude\":" + String(carMovementMeasurements.gpsAltitude, 6) + ",";
      jsonPayload += "\"time\":" + String(carMovementMeasurements.gpsTime, 6) + ",";
      jsonPayload += "\"course\":" + String(carMovementMeasurements.gpsCourse, 6) + ",";
      jsonPayload += "\"speed\":" + String(carMovementMeasurements.gpsSpeed, 6);
      jsonPayload += "}";

      // end of car_movement
      jsonPayload += "}";
      }
    
    jsonPayload += "}";
    return jsonPayload;

}

String getCanMessagesJson(){

  generateSampleCanFrames(); // Generate sample CAN frames

  String jsonPayload = "[";

  for (size_t i = 0; i < canMessages.size(); i++) {
    CAN_FRAME can_message = canMessages[i];
    jsonPayload += "{";
    jsonPayload += "\"id\": \"0x" + String(can_message.id, HEX) + "\",";
    jsonPayload += "\"timestamp\":" + String(can_message.timestamp) + ",";
    jsonPayload += "\"data\":\"0x";
    for (int j = 0; j < can_message.length; j++) {
      jsonPayload += String(can_message.data.byte[j], HEX);
    }
    jsonPayload += "\"";
    jsonPayload += "}";
    if (i < canMessages.size() - 1) {
      jsonPayload += ","; // Add a comma between messages
    }
  }

  
  jsonPayload += "]";


  canMessages.clear(); // Clear the vector after sending the data
  return jsonPayload;
}

String construct_json_payload(){

  Serial.println("Constructing JSON payload...");
  unsigned long startTime = millis();
  String jsonPayload = "{";
  jsonPayload += "\"device_id\":\"" + WiFi.macAddress() + "\",";

  jsonPayload += "\"config\":" + getConfigJson();



  if (config.um7Enabled){
    jsonPayload += ",\"uml7_measurements\":" + getuml7MeasurementsJson();
  }
  

  if (config.canEnabled){
    jsonPayload += ",\"can_messages\":" + getCanMessagesJson();
  }

  jsonPayload += "}";    


  Serial.println("JSON payload constructed in " + String(millis() - startTime) + " ms");
  return jsonPayload;
}


void checkButtonPress() {
  bool currentButtonState = digitalRead(25);
  if (lastButtonState == HIGH && currentButtonState == LOW) {


    Serial.println("Button pressed!");
    Serial.println("Generating sample can frames");
    generateSampleCanFrames(); // Generate sample CAN frames

    
    Serial.println("Sent CAN request for Engine RPM.");
    // Send a CAN request to get engine RPM (PID 0x0C)
    CAN_FRAME requestFrame;
    requestFrame.id = 0x7DF; // Standard OBD-II request ID
    requestFrame.extended = false;
    requestFrame.length = 8;
    requestFrame.data.byte[0] = 0x02; // Number of additional data bytes
    requestFrame.data.byte[1] = 0x01; // Service 01 (Current Data)
    requestFrame.data.byte[2] = 0x0C; // PID 0x0C (Engine RPM)
    requestFrame.data.byte[3] = 0x00;
    requestFrame.data.byte[4] = 0x00;
    requestFrame.data.byte[5] = 0x00;
    requestFrame.data.byte[6] = 0x00;
    requestFrame.data.byte[7] = 0x00;

    CAN0.sendFrame(requestFrame);
   

    //construct_json_payload();

    // Button was just pressed
    //String jsonPayload = "{\"button_press\":\"true\"}";
    //sendPostRequest(jsonPayload);
    
    //float temp;
    //uml7.getTemperature(temp);
    //Serial.print("Temperature: ");
    //Serial.println(temp);

    //printHealthData();  // Print health data from UML7
    //float gpsTime;
    //uml7.getGpsTime(gpsTime);
    //Serial.print("GPS Time: ");
    //Serial.println(gpsTime);  



  }
  lastButtonState = currentButtonState;

}

  void printHealthData() {
    Serial.println("Fetching health data from UML7...");
    HealthRegisterData healthData;

    if (uml7.getHealthRegisterData(healthData.sats_used, healthData.sats_in_view, healthData.HDOP,
             healthData.ovf, healthData.mg_n, healthData.acc_n,
             healthData.accel, healthData.gyro, healthData.mag, healthData.gps)) {
      Serial.println("Health Register Data:");
      Serial.print("Satellites Used: ");
      Serial.println(healthData.sats_used);
      Serial.print("Satellites in View: ");
      Serial.println(healthData.sats_in_view);
      Serial.print("HDOP: ");
      Serial.println(healthData.HDOP);
      Serial.print("Overflow: ");
      Serial.println(healthData.ovf ? "Yes" : "No");
      Serial.print("Magnetometer Normal: ");
      Serial.println(healthData.mg_n ? "Yes" : "No");
      Serial.print("Accelerometer Normal: ");
      Serial.println(healthData.acc_n ? "Yes" : "No");
      Serial.print("Accelerometer Initialization Failed: ");
      Serial.println(healthData.accel ? "Yes" : "No");
      Serial.print("Gyroscope Initialization Failed: ");
      Serial.println(healthData.gyro ? "Yes" : "No");
      Serial.print("Magnetometer Initialization Failed: ");
      Serial.println(healthData.mag ? "Yes" : "No");
      Serial.print("GPS Packet Timeout: ");
      Serial.println(healthData.gps ? "Yes" : "No");
    } else {
      Serial.println("Failed to fetch health register data.");
    }
  }

bool append_measurement_to_file(const String& measurements){
  if (config.sdCardEnabled) {
    String filename = "/measurements_log.txt";
    String file_line = String(millis()) + ";" + measurements + "\n";

    Serial.println("Storing measurements to SD card");
    appendFile(SD, filename.c_str(), file_line.c_str()); // Append the measurements to the file
    //Serial.println("Stored measurements to SD card: " + file_line);
    return true; // Return true if SD card is enabled and file operation is successful
}
return false; // Return false if SD card is not enabled
}


void store_measurements(const String& measurements) {
    // Save measurements to SD card if enabled
    append_measurement_to_file(measurements);
    Serial.println("Stored measurements to SD card");

    // Send measurements to the server if WiFi is enabled
    if (config.wifiEnabled) {
        sendPostRequest(measurements);
    }
}


void setupPins(){
  pinMode(2, OUTPUT); // SHIELD LED
  pinMode(27, OUTPUT); // CAN RX LED
  pinMode(26, OUTPUT); // WiFi LED
  digitalWrite(2, LOW);
  digitalWrite(27, LOW);
  digitalWrite(26, LOW);
  pinMode(25, INPUT_PULLUP); // Configure GPIO 25 as input with pull-up
  pinMode(CS, OUTPUT); // Configure CS pin for SD card
  sdCardMutex = xSemaphoreCreateMutex(); // Initialize the mutex
  if (sdCardMutex == NULL) {
      Serial.println("Failed to create SD card mutex");
  }
}


void setup() {
  Serial.begin(115200);

  setupPins(); // Setup GPIO pins

  if (config.sdCardEnabled) {
    Serial.println("\nInitializing SD card...");
    test_sdcard(); // Test SD card functionality
    // Load configuration
    if (!load_config("/config.txt")) { // Ensure load_config returns a success flag
        Serial.println("Failed to load configuration from SD card!");
        return;
    }
  }
  
  Serial.println("------------------------");
  Serial.println("ESP 32 CAN SHIELD");
  Serial.println("------------------------");
  Serial.println("Firmware version: 1.3.0");

  // Initialize CAN
  Serial.println(" CAN...............INIT");
  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5); // see important note above
  CAN0.begin(500000); // 500Kbps
  CAN0.watchFor();
  Serial.println(" CAN............500Kbps");
  Serial.println("Ready to log CAN messages.");


  String fw_version;
  // Initialize UML7 module
  if (config.um7Enabled){
    Serial.println("UML7.....................INIT");
    setupSerial();
    uml7.begin();
    Serial.println("UML 7 begin");
    
    if(uml7.getFirmwareVersion(fw_version)){
      Serial.println("Firmware version: " + fw_version);
    } else {
      Serial.println("Failed to get firmware version");
    }
    Serial.println("Firmware version: " + fw_version);
  }


  String jsonPayload = "{\"message\":\"CAN Shield started\", \"device_id\":\"" + WiFi.macAddress() + "\"";
  // Connect to WiFi
  
 if (config.um7Enabled){
    jsonPayload += ",\"fw_version\":\"" + fw_version + "\"";
  }
  jsonPayload += "}";

  if (config.wifiEnabled){
    connectToWiFi();
    // Send the message to the server
  }

  store_measurements(jsonPayload); // Store the message to SD card if enabled



  
}


void loop() {
  
  static unsigned long lastBlinkTime = 0;
  static unsigned long lastCanRequestTime = 0;
  static unsigned long lastorentationTime = 0;
  static unsigned long lastHttpPostTime = 0;
  unsigned long currentTime = millis();

  

  checkButtonPress();
  handle_can_message(); // Handle CAN messages


  if (currentTime - lastBlinkTime >= 5000) {
    Serial.println("Blinking SHIELD LED");
    lastBlinkTime = currentTime;
    digitalWrite(SHIELD_LED_PIN, !digitalRead(SHIELD_LED_PIN));
  }


  if (currentTime - lastCanRequestTime >= config.scrapeInterval) { // Send CAN request every second
    
    sendOBDRequests(); // Send OBD-II requests to CAN0
    lastCanRequestTime = currentTime;
  }

  
 // if (currentTime - lastorentationTime >= config.uml7scrapeInterval) { // print orientation every 5 seconds
 //   //printHealthData();
 //   //uml7.printAllMeasurements();
 //   String content = construct_json_payload();
 //   //Serial.println(content);
 //   sendPostRequest(content);
 //   lastorentationTime = currentTime;
 // }

  if (currentTime - lastHttpPostTime >= config.scrapeInterval) { // Send HTTP POST at configured interval
    String content = construct_json_payload();
    store_measurements(content);
    Serial.println("Measurements stored and sent to server.");
    lastHttpPostTime = currentTime;
  }
  
  //if(config.canEnabled){
  
  //}

  // Handle web server requests
  //server.handleClient();
}
