#include "Redshift_UML7.h"

Redshift_UML7::Redshift_UML7(Stream &serial) : serial(serial) {}

void Redshift_UML7::begin() {
    // Initialization code for the UML7 module
    Serial.println("Initializing Redshift UML7...");
    // Add any specific UM7 initialization here (if needed)
}


void Redshift_UML7::sendCommand(const String &command) {
    // Send a command to the module
    serial.println(command);
}

void Redshift_UML7::setBaudRate(unsigned long baudRate) {
    //TODO Implement this method
}

bool Redshift_UML7::parseNmeaPacket(String nmeaString) {
    //  Parse an NMEA packet and extract data.
    //  This is a basic example, you'll need to expand it
    //  to handle different NMEA packet types ($PCHRH,
    //  $PCHRP, $PCHRA, etc.) and extract the specific
    //  data you need.
    if (nmeaString.startsWith("$PCHRA")) {
        // Example: Parsing a $PCHRA (Attitude) packet
        int comma1 = nmeaString.indexOf(',');
        int comma2 = nmeaString.indexOf(',', comma1 + 1);
        int comma3 = nmeaString.indexOf(',', comma2 + 1);
        int comma4 = nmeaString.indexOf(',', comma3 + 1);

        if (comma1 != -1 && comma2 != -1 && comma3 != -1 && comma4 != -1) {
            // Extract data (example: roll, pitch, yaw)
            roll = nmeaString.substring(comma1 + 1, comma2).toFloat();
            pitch = nmeaString.substring(comma2 + 1, comma3).toFloat();
            yaw = nmeaString.substring(comma3 + 1, comma4).toFloat();
            return true;
        }
    }
    // New: Parsing $PCHRH packet
    else if (nmeaString.startsWith("$PCHRH")) {
        // $PCHRH, time, sats_used, sats_in_view, HDOP, mode, COM, accel, gyro, mag, GPS, res, res, res,*checksum
        int comma1 = nmeaString.indexOf(',');
        int comma2 = nmeaString.indexOf(',', comma1 + 1);
        int comma3 = nmeaString.indexOf(',', comma2 + 1);
        int comma4 = nmeaString.indexOf(',', comma3 + 1);
        int comma5 = nmeaString.indexOf(',', comma4 + 1);
        int comma6 = nmeaString.indexOf(',', comma5 + 1);
        int comma7 = nmeaString.indexOf(',', comma6 + 1);
        int comma8 = nmeaString.indexOf(',', comma7 + 1);
        int comma9 = nmeaString.indexOf(',', comma8 + 1);
        int comma10 = nmeaString.indexOf(',', comma9 + 1);

        if (comma1 != -1 && comma2 != -1 && comma3 != -1 && comma4 != -1 &&
            comma5 != -1 && comma6 != -1 && comma7 != -1 && comma8 != -1 &&
            comma9 != -1 && comma10 != -1) {
            health_time = nmeaString.substring(comma1 + 1, comma2).toFloat();
            health_sats_used = nmeaString.substring(comma2 + 1, comma3).toInt();
            health_sats_in_view = nmeaString.substring(comma3 + 1, comma4).toInt();
            health_HDOP = nmeaString.substring(comma4 + 1, comma5).toFloat();
            health_mode = nmeaString.substring(comma5 + 1, comma6).toInt();
            health_COM = nmeaString.substring(comma6 + 1, comma7).toInt();
            health_accel = nmeaString.substring(comma7 + 1, comma8).toInt();
            health_gyro = nmeaString.substring(comma8 + 1, comma9).toInt();
            health_mag = nmeaString.substring(comma9 + 1, comma10).toInt();
            health_GPS = nmeaString.substring(comma10 + 1).toInt();
            return true;
        }
    }
    return false; // Packet not recognized or parsing failed
}

bool Redshift_UML7::read_register_data(uint8_t registerAddress, uint8_t *data, size_t length) {
    // Prepare the read command packet
    uint8_t tx_data[5];
    tx_data[0] = 's';
    tx_data[1] = 'n';
    tx_data[2] = 'p';
    tx_data[3] = 0x00; // Packet Type: "Has Data" bit cleared for read operation
    tx_data[4] = registerAddress; // Address of the register to read

    // Calculate checksum
    uint16_t checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum += tx_data[i];
    }

    // Append checksum to the packet
    uint8_t tx_packet[7];
    memcpy(tx_packet, tx_data, 5);
    tx_packet[5] = (checksum >> 8) & 0xFF; // High byte
    tx_packet[6] = checksum & 0xFF;        // Low byte

    // Send the read command packet
    serial.write(tx_packet, sizeof(tx_packet));

    delay(READ_DELAY); // Adjust delay based on module response time
    // Read the response packet
    UM7_packet packet;
    for (int attempt = 0; attempt < GET_PACKET_ATTEMPS; attempt++) {
        delay(int(READ_DELAY / 2)); // Adjust delay based on module response time
        if (readPacket(packet, registerAddress)) {
            break; // Exit loop if packet is successfully read
        }
    
        Serial.println("Retrying to read response packet...");
    }

    if (packet.Address != registerAddress) {
        Serial.printf("Failed to read response packet after %d attempts\n", GET_PACKET_ATTEMPS);
        return false;
    }

    // Verify the response packet address
    if (packet.Address != registerAddress) {
        Serial.println("Unexpected response address");
        return false;
    }

    // Verify the response packet length
    if (packet.data_length < length) {
        Serial.println("Insufficient data in response");
        return false;
    }

    // Copy the data from the response packet
    memcpy(data, packet.data, length);
    return true;
}

bool Redshift_UML7::sendBinaryCommand(uint8_t command) {
    uint8_t tx_data[7];
    memset(tx_data, 0, sizeof(tx_data));
    tx_data[0] = 's';
    tx_data[1] = 'n';
    tx_data[2] = 'p';
    tx_data[3] = 0x00; // Packet Type byte
    tx_data[4] = command; // Address of the command

    // Correct checksum calculation
    uint16_t computed_checksum = 0;
    for (int i = 0; i < 5; i++) { // Only sum the first 5 bytes
        computed_checksum += tx_data[i];
    }

    // Append the checksum to the packet
    tx_data[5] = (computed_checksum >> 8) & 0xFF; // High byte
    tx_data[6] = computed_checksum & 0xFF;        // Low byte

    // Debug: Print the packet being sent
    Serial.print("Sending binary command: 0x");
    for (int i = 0; i < sizeof(tx_data); i++) {
        Serial.print(tx_data[i], HEX);
    }
    Serial.println();

    // Send the packet
    serial.write(tx_data, sizeof(tx_data));
    return true; // Or return false if sending fails
}

float Redshift_UML7::read_register_as_float(uint8_t* data) { // For one register as an IEEE floatpoint
    
    
    union {
        uint8_t bytes[4];
        float value;
    } temp;

    temp.bytes[3] = data[0];
    temp.bytes[2] = data[1];
    temp.bytes[1] = data[2];
    temp.bytes[0] = data[3];

    
    // Serial.println("Reading data as float");
    // Serial.print("Data: ");
    // for (int i = 0; i < 4; i++) {
    //     Serial.print(data[i], HEX);
    // }
    // Serial.print(" -> ");
    // Serial.println(temp.value);
    

    return temp.value;
}

bool Redshift_UML7::readPacket(UM7_packet &packet, u_int8_t address) { 
    return readPacket(packet, BUFFER_LENGTH, address); // Default packet size
}

bool Redshift_UML7::readPacket(UM7_packet &packet, int packet_size, u_int8_t address) {
    uint8_t buffer[packet_size]; // Temporary buffer to hold incoming data
    size_t length = readSerialData(buffer, sizeof(buffer)); // Read data from serial

    if (length == 0) {
        Serial.println("No data received from serial");
        return false;
    }

    // Parse the received data into a UM7_packet structure
    uint8_t parse_result = parse_serial_data(buffer, length, &packet, address);
    if (parse_result != 0) {
        Serial.print("Failed to parse packet, error code: ");
        Serial.println(parse_result);
        return false;
    }
    
    return true;
}


bool Redshift_UML7::getFirmwareVersion(String &fw_version) {
    
    // Send the GET_FW_REVISION command (0xAA)
    if (!sendBinaryCommand(GET_FW_REVISION)) {
        Serial.println("Failed to send GET_FW_REVISION command");
        return "Error: Command failed";
    }

    // Wait for the response
    delay(50); // Adjust delay based on module response time
    UM7_packet packet;
    readPacket(packet, GET_FW_REVISION);
    if (packet.Address == GET_FW_REVISION) {
        // Extract firmware version from packet data
        char fw_version_tmp[5];
        fw_version_tmp[0] = packet.data[0];
        fw_version_tmp[1] = packet.data[1];
        fw_version_tmp[2] = packet.data[2];
        fw_version_tmp[3] = packet.data[3];
        fw_version_tmp[4] = '\0'; // Null-terminate the string
        fw_version = String(fw_version_tmp);
        return true;
    } else {
        return false;
    }
}

size_t Redshift_UML7::readSerialData(uint8_t *buffer, size_t bufferSize) {
    size_t length = 0;
    unsigned long startTime = millis();
    while (millis() - startTime < 500 && length < bufferSize) {
        if (serial.available()) {
            buffer[length++] = serial.read();
        }
    }

    // Debug: Print the received data
    //Serial.print("Received data: 0x");
    //for (size_t i = 0; i <= length; i++) {
    //    Serial.print(buffer[i], HEX);
    //}
    //Serial.println();

    return length;
}

bool Redshift_UML7::parsePacket(const uint8_t *rx_data, size_t rx_length, UM7_packet &packet, uint8_t expectedAddress) {
    uint8_t parse_result = parse_serial_data(const_cast<uint8_t *>(rx_data), rx_length, &packet, expectedAddress);
    return parse_result == 0 && packet.Address == expectedAddress;
}

// New: Get Raw Magnetometer Data
bool Redshift_UML7::getMagRawXY(int16_t &magX, int16_t &magY) {
    uint8_t tmpdata[4];
    if (read_register_data(DREG_MAG_RAW_XY, tmpdata, sizeof(tmpdata))) {
        magX = (tmpdata[0] << 8) | tmpdata[1]; // Combine high and low bytes for X-axis
        magY = (tmpdata[2] << 8) | tmpdata[3]; // Combine high and low bytes for Y-axis
        return true;
    }
    return false;
}

void Redshift_UML7::printPacket(const UM7_packet &packet) {
    Serial.println("-----------------------------------");
    Serial.println("Packet Details:");
    Serial.print("Address: 0x");
    Serial.println(packet.Address, HEX);
    Serial.print("Packet Type: 0x");
    Serial.println(packet.PT, HEX);
    Serial.print("Data Length: ");
    Serial.println(packet.data_length);
    Serial.print("Data: 0x");
    for (size_t i = 0; i <= packet.data_length; i++) {
        Serial.print(packet.data[i], HEX);
        }
    Serial.println();
    Serial.println("-----------------------------------");
}

bool Redshift_UML7::getHealthRegisterData(uint8_t &sats_used, uint8_t &sats_in_view, float &HDOP,bool &ovf, bool &mg_n, bool &acc_n, bool &accel, bool &gyro, bool &mag, bool &gps) {
    uint8_t tmpdata[4];
    if (read_register_data(DREG_HEALTH, tmpdata, sizeof(tmpdata))) {
        uint8_t data[4];
        data[0] = tmpdata[3];
        data[1] = tmpdata[2];
        data[2] = tmpdata[1];
        data[3] = tmpdata[0];
        


        // Parse the data into the HealthData structure
        sats_used = (data[3] >> 2) & 0x3F; // Extract SATS_USED from B3 (bits 31-26)
        HDOP = static_cast<float>(((data[3] & 0x03) << 8) | data[2]) / 10.0f; // Extract HDOP from B3:B2 (bits 25-16)
        sats_in_view = (data[1] >> 2) & 0x3F; // Extract SATS_IN_VIEW from B2 (bits 15-10)
        (data[1]) & 0x0002; // reserver bit B2 bit 9
        ovf = data[1] & 0x01; // Extract OVF from B2 (bit 8)
        mg_n = data[0] & 0x20; // Extract MG_N from B1 (bit 5)
        acc_n = data[0] & 0x10; // Extract ACC_N from B1 (bit 4)
        accel = data[0] & 0x08; // Extract ACCEL from B1 (bit 3)
        gyro = data[0] & 0x04; // Extract GYRO from B1 (bit 2)
        mag = data[0] & 0x02; // Extract MAG from B1 (bit 1)
        gps = data[0] & 0x01; // Extract GPS from B1 (bit 0)

        return true;
    }
    return false;
}

uint8_t Redshift_UML7::parse_serial_data(uint8_t* rx_data, uint8_t rx_length, UM7_packet* packet, u_int8_t expectedAddress) {
    uint8_t index = 0;

    while (index < rx_length - 2) {
        // Ensure the data buffer is long enough for a full packet
        if (rx_length - index < 7) {
            return 1; // Not enough data for a full packet
        }

        // Find the 'snp' start sequence
        while (index < (rx_length - 2) && !(rx_data[index] == 's' && rx_data[index + 1] == 'n' && rx_data[index + 2] == 'p')) {
            index++;
        }

        // Check if the header was not found
        if (index >= (rx_length - 2)) {
            return 2; // No valid packet found
        }

        uint8_t packet_index = index;

        // Verify enough space for a full packet
        if ((rx_length - packet_index) < 7) {
            return 3; // Not enough data for a full packet
        }

        // Extract packet type and determine length
        uint8_t PT = rx_data[packet_index + 3];
        uint8_t packet_has_data = (PT & 0x80) >> 7;
        uint8_t packet_is_batch = (PT & 0x40) >> 6;
        uint8_t batch_length = (PT & 0x3C) >> 2;

        uint8_t data_length = 0;
        if (packet_has_data) {
            data_length = packet_is_batch ? 4 * batch_length : 4;
        }

        // Verify enough data for the full packet
        if ((rx_length - packet_index) < (data_length + 7)) {
            return 3; // Not enough data for a full packet
        }

        // Extract data and compute checksum
        packet->Address = rx_data[packet_index + 4];
        packet->PT = PT;
        packet->data_length = data_length;

        // Ensure data_length does not exceed the packet's data buffer size
        if (data_length > sizeof(packet->data)) {
            return 5; // Data length exceeds buffer size
        }

        uint16_t computed_checksum = 's' + 'n' + 'p' + packet->PT + packet->Address;
        for (uint8_t i = 0; i < data_length; i++) {
            packet->data[i] = rx_data[packet_index + 5 + i];
            computed_checksum += packet->data[i];
        }

        // Verify checksum
        uint16_t received_checksum = (rx_data[packet_index + 5 + data_length] << 8) |
                                      rx_data[packet_index + 6 + data_length];

        if (received_checksum != computed_checksum) {
            return 4; // Checksum mismatch
        }

        if (packet->Address == expectedAddress) {
            // Debug: Print parsed packet details
            //Serial.println("###### Expected packet found ######");
            //printPacket(*packet); // Print the packet details
            return 0; // Packet successfully parsed
        }

        // Move to the next packet
        index = packet_index + 7 + data_length;
    }

    Serial.println("No more packets found");
    return 6; // All packets successfully parsed
}

// ---------------------- Data Access Methods ----------------------

float Redshift_UML7::getRoll() {
    uint8_t data[4];
    if (read_register_data(DREG_EULER_PHI_THETA, data, 4)) {
        int16_t rawRoll = (data[0] << 8) | data[1];
        return static_cast<float>(rawRoll) / 91.02222f; // Scale factor for roll
    }
    return 0.0f; // Return 0 if reading fails
}

float Redshift_UML7::getPitch() {
    uint8_t data[4];
    if (read_register_data(DREG_EULER_PHI_THETA, data, 4)) {
        int16_t rawPitch = (data[2] << 8) | data[3];
        return static_cast<float>(rawPitch) / 91.02222f; // Scale factor for pitch
    }
    return 0.0f; // Return 0 if reading fails
}

float Redshift_UML7::getYaw() {
    uint8_t data[4]; 
    if (read_register_data(DREG_EULER_PSI, data, 4)) {
        // Extract yaw from the 1st and 2nd bytes
        int16_t rawYaw = (data[0] << 8) | data[1];
        return static_cast<float>(rawYaw) / 91.02222f; // Scale factor for yaw
    }
    return 0.0f; // Return 0 if reading fails
}

bool Redshift_UML7::getOrientation(float &yaw, float &pitch, float &roll){

    uint8_t data[4];
    if (read_register_data(DREG_EULER_PHI_THETA, data, 4)) {
        int16_t rawRoll = (data[0] << 8) | data[1];
        int16_t rawPitch = (data[2] << 8) | data[3];
        roll = static_cast<float>(rawRoll) / 91.02222f; // Scale factor for roll
        pitch = static_cast<float>(rawPitch) / 91.02222f; // Scale factor for pitch
        yaw = getYaw();
        return true;
    }
    
    return false; // Return false if reading fails
}

float Redshift_UML7::getMagX() {
    uint8_t data[4];
    if (read_register_data(DREG_MAG_PROC_X, data, 4)) {
        return read_register_as_float(data);
    }
    return 0.0f; // Return 0 if reading fails
}

float Redshift_UML7::getMagY() {
    uint8_t data[4];
    if (read_register_data(DREG_MAG_PROC_Y, data, 4)) {
        return read_register_as_float(data);
    }
    return 0.0f; // Return 0 if reading fails
}

float Redshift_UML7::getMagZ() {
    uint8_t data[4];
    if (read_register_data(DREG_MAG_PROC_Z, data, 4)) {
        return read_register_as_float(data);
    }
    return 0.0f; // Return 0 if reading fails
}

bool Redshift_UML7::getTemperature(float &temp) {
    uint8_t data[4];
    if (read_register_data(DREG_TEMPERATURE, data, 4)) {
        temp = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getGpsTime(float &gpsTime) {
    uint8_t data[4];
    if (read_register_data(DREG_GPS_TIME, data, 4)) {
        gpsTime = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getGyroRawMeasurements(int16_t &gyroX, int16_t &gyroY, int16_t &gyroZ) {
    uint8_t data[6];
    if (read_register_data(DREG_GYRO_RAW_XY, data, 6)) {
        gyroX = (data[0] << 8) | data[1];
        gyroY = (data[2] << 8) | data[3];
        gyroZ = (data[4] << 8) | data[5];
        return true;
    }
    return false;
}




bool Redshift_UML7::getAccelRawXY(int16_t &accelX, int16_t &accelY) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_RAW_XY, data, 4)) {
        int16_t rawAccelX = (data[0] << 8) | data[1];
        int16_t rawAccelY = (data[2] << 8) | data[3];
        accelX = static_cast<float>(accelX); 
        accelY = static_cast<float>(accelY); 
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelRawZ(int16_t &accelZ) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_RAW_Z, data, 4)) {
        int16_t rawAccelX = (data[0] << 8) | data[1];
        accelZ = static_cast<float>(accelZ); 
        return true;
    }
    return false;
}


bool Redshift_UML7::getAccelRawMeasurements(int16_t &accelX, int16_t &accelY, int16_t &accelZ) {
    float accelXf, accelYf, accelZf;
    if (getAccelRawXY(accelX, accelY) && getAccelRawZ(accelZ)) {
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelRawMeasurements(AccelMeasurement &accelData) {
    uint8_t data[6];
    if (read_register_data(DREG_ACCEL_RAW_XY, data, 6)) {
        accelData.accelX = (data[0] << 8) | data[1];
        accelData.accelY = (data[2] << 8) | data[3];
        accelData.accelZ = (data[4] << 8) | data[5];
        return true;
    }
    return false;
}

bool Redshift_UML7::getMagRawMeasurements(MagMeasurement &magData) {
    uint8_t data[6];
    if (read_register_data(DREG_MAG_RAW_XY, data, 6)) {
        magData.magX = (data[0] << 8) | data[1];
        magData.magY = (data[2] << 8) | data[3];
        magData.magZ = (data[4] << 8) | data[5];
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelProcX(float &accelX) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_PROC_X, data, 4)) {
        accelX = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelProcY(float &accelY) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_PROC_Y, data, 4)) {
        accelY = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelProcZ(float &accelZ) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_PROC_Z, data, 4)) {
        accelZ = read_register_as_float(data);
        return true;
    }
    return false;
}


bool Redshift_UML7::getAccelProcessedMeasurements(float &accelX, float &accelY, float &accelZ, float &accelTime) {
    if (getAccelProcX(accelX) && getAccelProcY(accelY) && getAccelProcZ(accelZ) && getAccelTime(accelTime)) {
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelProcessedMeasurements(AccelMeasurement &accelData) {
    // Get processed X acceleration
    if (!getAccelProcX(accelData.accelX)) {
        return false;
    }

    // Get processed Y acceleration
    if (!getAccelProcY(accelData.accelY)) {
        return false;
    }

    // Get processed Z acceleration
    if (!getAccelProcZ(accelData.accelZ)) {
        return false;
    }

    // Get time of measurement
    if (!getAccelTime(accelData.accelTime)) {
        return false;
    }

    return true;
}



bool Redshift_UML7::getGyroProcX(float &gyroX) {
    uint8_t data[4];
    if (read_register_data(DREG_GYRO_PROC_X, data, 4)) {
        gyroX = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getGyroProcY(float &gyroY) {
    uint8_t data[4];
    if (read_register_data(DREG_GYRO_PROC_Y, data, 4)) {
        gyroY = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getGyroProcZ(float &gyroZ) {
    uint8_t data[4];
    if (read_register_data(DREG_GYRO_PROC_Z, data, 4)) {
        gyroZ = read_register_as_float(data);
        return true;
    }
    return false;
}


bool Redshift_UML7::getGyroProcessedMeasurements(GyroMeasurement &gyroData) {
    return getGyroProcessedMeasurements(gyroData.gyroX, gyroData.gyroY, gyroData.gyroZ, gyroData.gyroTime);
}

bool Redshift_UML7::getGyroProcessedMeasurements(float &gyroX, float &gyroY, float &gyroZ, float &gyroTime) {
    if (getGyroProcX(gyroX) && getGyroProcY(gyroY) && getGyroProcZ(gyroZ) && getGyroTime(gyroTime)) {
        return true;
    }
    return false;
}

bool Redshift_UML7::getMagneticData(float &magX, float &magY, float &magZ, float &time) {
    uint8_t data[4];

    // Get magnetic X
    if (!read_register_data(DREG_MAG_PROC_X, data, 4)) {
        return false;
    }
    magX = read_register_as_float(data);

    // Get magnetic Y
    if (!read_register_data(DREG_MAG_PROC_Y, data, 4)) {
        return false;
    }
    magY = read_register_as_float(data);

    // Get magnetic Z
    if (!read_register_data(DREG_MAG_PROC_Z, data, 4)) {
        return false;
    }
    magZ = read_register_as_float(data);

    // Get time of measurement
    if (!read_register_data(DREG_MAG_PROC_TIME, data, 4)) {
        return false;
    }
    time = read_register_as_float(data);

    return true;
}

bool Redshift_UML7::getGyroTime(float &time) {
    uint8_t data[4];
    if (read_register_data(DREG_GYRO_PROC_TIME, data, 4)) {
        time = read_register_as_float(data);
        return true;
    }
    return false;
}

bool Redshift_UML7::getAccelTime(float &time) {
    uint8_t data[4];
    if (read_register_data(DREG_ACCEL_PROC_TIME, data, 4)) {
        time = read_register_as_float(data);
        return true;
    }
    return false;
}

// ---------------------- Command Functions (Page 86) ----------------------

bool Redshift_UML7::resetToFactoryDefaults() {
    // Command: RESET_TO_FACTORY (0xAA)
    return sendBinaryCommand(0xAA);
}

bool Redshift_UML7::zeroGyroscopes() {
    // Command: ZERO_GYROS (0xAC)
    return sendBinaryCommand(0xAC);
}

bool Redshift_UML7::setHomePosition() {
    // Command: SET_HOME_POSITION (0xAD)
    return sendBinaryCommand(0xAD);
}

bool Redshift_UML7::resetEKF() {
    // Command: RESET_EKF (0xA9)
    return sendBinaryCommand(0xA9);
}

bool Redshift_UML7::flashCommit() {
    // Command: FLASH_COMMIT (0xAB)
    return sendBinaryCommand(0xAB);
}

bool Redshift_UML7::readDregData(uint8_t dregAddress, uint8_t *data, size_t length) {
    // Send the DREG read command
    if (!sendBinaryCommand(dregAddress)) {
        Serial.println("Failed to send DREG read command");
        return false;
    }

    // Wait for the response
    delay(READ_DELAY); // Adjust delay based on module response time

    UM7_packet packet;
    bool found = false;

    // Read packets until the desired one is found
    while (readPacket(packet, dregAddress)) {
        if (packet.Address == dregAddress) {
            // Verify the response packet length
            if (packet.data_length < length) {
                //Serial.println("Insufficient data in response");
                return false;
            }

            // Copy the data from the response packet
            memcpy(data, packet.data, length);
            found = true;
            break;
        }
    }

    if (!found) {
        Serial.println("Desired packet not found in response");
        return false;
    }

    return true;
}

bool Redshift_UML7::getCarMovementMeasurements(CarMovementMeasurements &measurements){
    // Retrieve accelerometer processed measurements
    if(!getAccelProcessedMeasurements(measurements.accelX, measurements.accelY, measurements.accelZ, measurements.accelTime)){
        return false;
    }

    // Retrieve gyro processed measurements
    if(!getGyroProcessedMeasurements(measurements.gyroX, measurements.gyroY, measurements.gyroZ, measurements.gyroTime)){
        return false;
    }

    // Retrieve magnetometer processed measurements
    if(!getMagneticData(measurements.magX, measurements.magY, measurements.magZ, measurements.magTime)){
        return false;
    }

    if (!getOrientation(measurements.yaw, measurements.pitch, measurements.roll)) {
        return false;
    }

    // Retrieve GPS measurements
    if (!getGPSMeasurements(measurements.gpsLatitude, measurements.gpsLongitude, measurements.gpsAltitude, measurements.gpsTime, measurements.gpsCourse, measurements.gpsSpeed)) {
        return false;
    }



    return true;
}

bool Redshift_UML7::getGPSMeasurements(GPSMeasurement &gpsData){
    // Retrieve GPS measurements
    if (!getGPSMeasurements(gpsData.latitude, gpsData.longitude, gpsData.altitude, gpsData.time, gpsData.course, gpsData.speed)) {
        return false;
    }

    return true;
}


bool Redshift_UML7::getGPSMeasurements(float &latitude, float &longitude, float &altitude, float &time, float &course, float &speed) {
    //TODO Implement this method
    uint8_t data[4];
    if (read_register_data(DREG_GPS_LATITUDE, data, 4)) {
        latitude = read_register_as_float(data);
    }
    else {
        return false;
    }
    if (read_register_data(DREG_GPS_LONGITUDE, data, 4)) {
        longitude = read_register_as_float(data);
    }
    else {
        return false;
    }
    if (read_register_data(DREG_GPS_ALTITUDE, data, 4)) {
        altitude = read_register_as_float(data);
    }
    else {
        return false;
    }
    if (read_register_data(DREG_GPS_TIME, data, 4)) {
        time = read_register_as_float(data);
    }
    else {
        return false;
    }
    if (read_register_data(DREG_GPS_COURSE, data, 4)) {
        course = read_register_as_float(data);
    }
    else {
        return false;
    }
    if (read_register_data(DREG_GPS_SPEED, data, 4)) {
        speed = read_register_as_float(data);
    }
    else {
        return false;
    }

    //Serial.println("------ GPS Measurements ------");
    //Serial.printf("Latitude: %f\n", latitude);
    //Serial.printf("Longitude: %f\n", longitude);
    //Serial.printf("Altitude: %f\n", altitude);
    //Serial.printf("Time: %f\n", time);
    //Serial.printf("Course: %f\n", course);
    //Serial.printf("Speed: %f\n", speed);
    //Serial.printf("---------------------------------------\n");
    return true;
    


}

void Redshift_UML7::printAllMeasurements() {
    Serial.println("------ All Relevant Measurements ------");


    float roll, pitch, yaw;
    if (getOrientation(yaw, pitch, roll)) {
        Serial.print("Roll: ");
        Serial.println(roll);
        Serial.print("Pitch: ");
        Serial.println(pitch);
        Serial.print("Yaw: ");
        Serial.println(yaw);
    } else {
        Serial.println("Orientation: Error reading");
    }

    // Print magnetometer data
    Serial.print("MagX: ");
    Serial.println(getMagX());
    Serial.print("MagY: ");
    Serial.println(getMagY());
    Serial.print("MagZ: ");
    Serial.println(getMagZ());

    // Print temperature
    float temp;
    if (getTemperature(temp)) {
        Serial.print("Temperature: ");
        Serial.println(temp);
    } else {
        Serial.println("Temperature: Error reading");
    }

    // Print GPS time
    float gpsTime;
    if (getGpsTime(gpsTime)) {
        Serial.print("GPS Time: ");
        Serial.println(gpsTime);
    } else {
        Serial.println("GPS Time: Error reading");
    }

    // Print gyro processed measurements
    GyroMeasurement gyroData;
    if (getGyroProcessedMeasurements(gyroData)) {
        Serial.print("Gyro X: ");
        Serial.println(gyroData.gyroX);
        Serial.print("Gyro Y: ");
        Serial.println(gyroData.gyroY);
        Serial.print("Gyro Z: ");
        Serial.println(gyroData.gyroZ);
        Serial.print("Gyro Time: ");
        Serial.println(gyroData.gyroTime);
    } else {
        Serial.println("Gyro Measurements: Error reading");
    }

    // Print accelerometer raw measurements
    float accelX, accelY, accelZ, accelTime;
    if (getAccelProcessedMeasurements(accelX, accelY, accelZ, accelTime)) {
        Serial.print("Accel Time: ");
        Serial.println(accelTime);
        Serial.print("Accel X: ");
        Serial.println(accelX);
        Serial.print("Accel Y: ");
        Serial.println(accelY);
        Serial.print("Accel Z: ");
        Serial.println(accelZ);
    } else {
        Serial.println("Accel Measurements: Error reading");
    }

    // Print raw accelerometer measurements
    AccelMeasurement accelData;
    if (getAccelRawMeasurements(accelData)) {
        Serial.print("Raw Accel X: ");
        Serial.println(accelData.accelX);
        Serial.print("Raw Accel Y: ");
        Serial.println(accelData.accelY);
        Serial.print("Raw Accel Z: ");
        Serial.println(accelData.accelZ);
    } else {
        Serial.println("Raw Accelerometer Measurements: Error reading");
    }

    // Print raw magnetometer measurements
    MagMeasurement magData;
    if (getMagRawMeasurements(magData)) {
        Serial.print("Raw Mag X: ");
        Serial.println(magData.magX);
        Serial.print("Raw Mag Y: ");
        Serial.println(magData.magY);
        Serial.print("Raw Mag Z: ");
        Serial.println(magData.magZ);
    } else {
        Serial.println("Raw Magnetometer Measurements: Error reading");
    }

    Serial.println("---------------------------------------");
}