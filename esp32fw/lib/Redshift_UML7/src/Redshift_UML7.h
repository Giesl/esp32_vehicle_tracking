/**
 * @file Redshift_UML7.h
 * @brief Header file for the Redshift_UML7 class, which provides an interface to the UM7 sensor.
 * 
 * This file contains the definitions, constants, and declarations required to interact with the UM7 sensor.
 * The UM7 sensor is a high-performance orientation sensor that provides data such as gyroscope, accelerometer,
 * magnetometer, GPS, and orientation (yaw, pitch, roll). The Redshift_UML7 class facilitates communication
 * with the sensor over a serial interface and provides methods to retrieve and process sensor data.
 * 
 * @author Vojtěch Giesl
 * @date 03/2025
 * @version 1.0
 */

#ifndef REDSHIFT_UML7_H
#define REDSHIFT_UML7_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup ConfigurationRegisters Configuration Registers
 * @brief Definitions for configuration register addresses.
 * @{
 */
#define CREG_COM_SETTINGS 0x00 ///< Baud rates for reading over the UM7, default 115200 baud
#define CREG_COM_RATES1 0x01 ///< Individual raw data rate
#define CREG_COM_RATES2 0x02 ///< All raw data rate
#define CREG_COM_RATES3 0x03 ///< Individual processed data rate
#define CREG_COM_RATES4 0x04 ///< All processed data rate
#define CREG_COM_RATES5 0x05 ///< Quaternion, Euler, position, and velocity data rate
#define CREG_COM_RATES6 0x06 ///< Pose (Euler & position), health, and gyro bias estimate rates
#define CREG_COM_RATES7 0x07 ///< Data rate for CHR NMEA-style packets
#define CREG_MISC_SETTINGS 0x08 ///< Filter and sensor control options
#define CREG_HOME_NORTH 0x09 ///< Sets north from current position
#define CREG_HOME_EAST 0x0A ///< Sets east from current position
#define CREG_HOME_UP 0x0B ///< Sets home altitude in meters
#define CREG_GYRO_TRIM_X 0x0C ///< Gyroscope X-axis trim
#define CREG_GYRO_TRIM_Y 0x0D ///< Gyroscope Y-axis trim
#define CREG_GYRO_TRIM_Z 0x0E ///< Gyroscope Z-axis trim
#define CREG_MAG_CAL1_1 0x0F ///< Magnetometer calibration matrix element 1,1
#define CREG_MAG_CAL1_2 0x10 ///< Magnetometer calibration matrix element 1,2
#define CREG_MAG_CAL1_3 0x11 ///< Magnetometer calibration matrix element 1,3
#define CREG_MAG_CAL2_1 0x12 ///< Magnetometer calibration matrix element 2,1
#define CREG_MAG_CAL2_2 0x13 ///< Magnetometer calibration matrix element 2,2
#define CREG_MAG_CAL2_3 0x14 ///< Magnetometer calibration matrix element 2,3
#define CREG_MAG_CAL3_1 0x15 ///< Magnetometer calibration matrix element 3,1
#define CREG_MAG_CAL3_2 0x16 ///< Magnetometer calibration matrix element 3,2
#define CREG_MAG_CAL3_3 0x17 ///< Magnetometer calibration matrix element 3,3
#define CREG_MAG_BIAS_X 0x18 ///< Magnetometer X-axis bias
#define CREG_MAG_BIAS_Y 0x19 ///< Magnetometer Y-axis bias
#define CREG_MAG_BIAS_Z 0x1A ///< Magnetometer Z-axis bias
#define CREG_ACCEL_CAL1_1 0x1B ///< Accelerometer calibration matrix element 1,1
#define CREG_ACCEL_CAL1_2 0x1C ///< Accelerometer calibration matrix element 1,2
#define CREG_ACCEL_CAL1_3 0x1D ///< Accelerometer calibration matrix element 1,3
#define CREG_ACCEL_CAL2_1 0x1E ///< Accelerometer calibration matrix element 2,1
#define CREG_ACCEL_CAL2_2 0x1F ///< Accelerometer calibration matrix element 2,2
#define CREG_ACCEL_CAL2_3 0x20 ///< Accelerometer calibration matrix element 2,3
#define CREG_ACCEL_CAL3_1 0x21 ///< Accelerometer calibration matrix element 3,1
#define CREG_ACCEL_CAL3_2 0x22 ///< Accelerometer calibration matrix element 3,2
#define CREG_ACCEL_CAL3_3 0x23 ///< Accelerometer calibration matrix element 3,3
#define CREG_ACCEL_BIAS_X 0x24 ///< Accelerometer X-axis bias
#define CREG_ACCEL_BIAS_Y 0x25 ///< Accelerometer Y-axis bias
#define CREG_ACCEL_BIAS_Z 0x26 ///< Accelerometer Z-axis bias
/** @} */

/**
 * @defgroup DataRegisters Data Registers
 * @brief Definitions for data register addresses.
 * @{
 */
#define DREG_HEALTH 0x55 ///< Health register
#define DREG_GYRO_RAW_XY 0x56 ///< Raw gyroscope X and Y-axis data
#define DREG_GYRO_RAW_Z 0x57 ///< Raw gyroscope Z-axis data
#define DREG_GYRO_RAW_TIME 0x58 ///< Timestamp for raw gyroscope data
#define DREG_ACCEL_RAW_XY 0x59 ///< Raw accelerometer X and Y-axis data
#define DREG_ACCEL_RAW_Z 0x5A ///< Raw accelerometer Z-axis data
#define DREG_ACCEL_RAW_TIME 0x5B ///< Timestamp for raw accelerometer data
#define DREG_MAG_RAW_XY 0x5C ///< Raw magnetometer X and Y-axis data
#define DREG_MAG_RAW_Z 0x5D ///< Raw magnetometer Z-axis data
#define DREG_MAG_RAW_TIME 0x5E ///< Timestamp for raw magnetometer data
#define DREG_TEMPERATURE 0x5F ///< Temperature data
#define DREG_TEMPERATURE_TIME 0x60 ///< Timestamp for temperature data
#define DREG_GYRO_PROC_X 0x61 ///< Processed gyroscope X-axis data (deg/s)
#define DREG_GYRO_PROC_Y 0x62 ///< Processed gyroscope Y-axis data (deg/s)
#define DREG_GYRO_PROC_Z 0x63 ///< Processed gyroscope Z-axis data (deg/s)
#define DREG_GYRO_PROC_TIME 0x64 ///< Timestamp for processed gyroscope data
#define DREG_ACCEL_PROC_X 0x65 ///< Processed accelerometer X-axis data (m/s^2)
#define DREG_ACCEL_PROC_Y 0x66 ///< Processed accelerometer Y-axis data (m/s^2)
#define DREG_ACCEL_PROC_Z 0x67 ///< Processed accelerometer Z-axis data (m/s^2)
#define DREG_ACCEL_PROC_TIME 0x68 ///< Timestamp for processed accelerometer data
#define DREG_MAG_PROC_X 0x69 ///< Processed magnetometer X-axis data (T)
#define DREG_MAG_PROC_Y 0x6A ///< Processed magnetometer Y-axis data (T)
#define DREG_MAG_PROC_Z 0x6B ///< Processed magnetometer Z-axis data (T)
#define DREG_MAG_PROC_TIME 0x6C ///< Timestamp for processed magnetometer data
#define DREG_QUAT_AB 0x6D ///< Quaternion components A and B
#define DREG_QUAT_CD 0x6E ///< Quaternion components C and D
#define DREG_QUAT_TIME 0x6F ///< Timestamp for quaternion data
#define DREG_EULER_PHI_THETA 0x70 ///< Euler angles phi and theta (deg)
#define DREG_EULER_PSI 0x71 ///< Euler angle psi (deg)
#define DREG_EULER_PHI_THETA_DOT 0x72 ///< Euler angle rates phi and theta (deg/s)
#define DREG_EULER_PSI_DOT 0x73 ///< Euler angle rate psi (deg/s)
#define DREG_EULER_TIME 0x74 ///< Timestamp for Euler angle data
#define DREG_POSITION_N 0x75 ///< Position north (m)
#define DREG_POSITION_E 0x76 ///< Position east (m)
#define DREG_POSITION_UP 0x77 ///< Position up (m)
#define DREG_POSITION_TIME 0x78 ///< Timestamp for position data
#define DREG_VELOCITY_N 0x79 ///< Velocity north (m/s)
#define DREG_VELOCITY_E 0x7A ///< Velocity east (m/s)
#define DREG_VELOCITY_UP 0x7B ///< Velocity up (m/s)
#define DREG_VELOCITY_TIME 0x7C ///< Timestamp for velocity data
#define DREG_GPS_LATITUDE 0x7D ///< GPS latitude (deg)
#define DREG_GPS_LONGITUDE 0x7E ///< GPS longitude (deg)
#define DREG_GPS_ALTITUDE 0x7F ///< GPS altitude (m)
#define DREG_GPS_COURSE 0x80 ///< GPS course (deg)
#define DREG_GPS_SPEED 0x81 ///< GPS speed (m/s)
#define DREG_GPS_TIME 0x82 ///< GPS time
#define DREG_GPS_SAT_1_2 0x83 ///< GPS satellites 1 and 2
#define DREG_GPS_SAT_3_4 0x84 ///< GPS satellites 3 and 4
#define DREG_GPS_SAT_5_6 0x85 ///< GPS satellites 5 and 6
#define DREG_GPS_SAT_7_8 0x86 ///< GPS satellites 7 and 8
#define DREG_GPS_SAT_9_10 0x87 ///< GPS satellites 9 and 10
#define DREG_GPS_SAT_11_12 0x88 ///< GPS satellites 11 and 12
#define DREG_GYRO_BIAS_X 0x89 ///< Gyroscope X-axis bias
#define DREG_GYRO_BIAS_Y 0x8A ///< Gyroscope Y-axis bias
#define DREG_GYRO_BIAS_Z 0x8B ///< Gyroscope Z-axis bias
/** @} */

/**
 * @defgroup CommandRegisters Command Registers
 * @brief Definitions for command register addresses.
 * @{
 */
#define GET_FW_REVISION 0xAA ///< Get firmware revision
#define FLASH_COMMIT 0xAB ///< Write all configuration settings to FLASH
#define RESET_TO_FACTORY 0xAC ///< Reset to factory defaults
#define ZERO_GYROS 0xAD ///< Calibrate gyroscope, keep flat
#define SET_HOME_POSITION 0xAE ///< Set home position
#define SET_MAG_REFERENCE 0xB0 ///< Set magnetic reference
#define CALIBRATE_ACCELEROMETERS 0xB1 ///< Calibrate accelerometers (keep flat)
#define RESET_EKF 0xB3 ///< Reset the Extended Kalman Filter
/** @} */



#define READ_DELAY 100 ///< Delay in milliseconds for reading data
#define BUFFER_LENGTH 128 ///< Length of the buffer for reading data
#define GET_PACKET_ATTEMPS 5 ///< Number of attempts to get a packet

/**
 * @brief Structure for holding received packet information.
 */
typedef struct UM7_packet_struct {
    uint8_t Address;       ///< Packet address
    uint8_t PT;            ///< Packet type
    uint16_t Checksum;     ///< Packet checksum
    uint8_t data_length;   ///< Length of the data section
    uint8_t data[30];      ///< Data section (up to 30 bytes)
} UM7_packet;

/**
 * @brief Helper function to extract bits from a byte.
 * @param byte The byte to extract bits from.
 * @param start_bit The starting bit position.
 * @param num_bits The number of bits to extract.
 * @return Extracted bits as a uint8_t.
 */
uint8_t extract_bits(uint8_t byte, uint8_t start_bit, uint8_t num_bits);

/**
 * @brief Structure to hold gyro measurement data.
 */
typedef struct {
    float gyroX;   ///< Processed gyro X-axis data
    float gyroY;   ///< Processed gyro Y-axis data
    float gyroZ;   ///< Processed gyro Z-axis data
    float gyroTime; ///< Timestamp of the gyro data
} GyroMeasurement;

/**
 * @brief Structure to hold accelerometer measurement data.
 */
typedef struct {
    float accelX; ///< Raw accelerometer X-axis data
    float accelY; ///< Raw accelerometer Y-axis data
    float accelZ; ///< Raw accelerometer Z-axis data
    float accelTime; ///< Timestamp of the accelerometer data
} AccelMeasurement;

/**
 * @brief Structure to hold magnetometer measurement data.
 */
typedef struct {
    float magX; ///< Raw magnetometer X-axis data
    float magY; ///< Raw magnetometer Y-axis data
    float magZ; ///< Raw magnetometer Z-axis data
    float magTime; ///< Timestamp of the magnetometer data
} MagMeasurement;

typedef struct{
    float latitude; ///< GPS latitude
    float longitude; ///< GPS longitude
    float altitude; ///< GPS altitude
    float time; ///< GPS time
    float course; ///< GPS course
    float speed; ///< GPS speed
} GPSMeasurement;


/**
 * @brief Structure to hold all relevant measurements for car movement monitoring.
 */
typedef struct {
    float roll;          ///< Roll angle in degrees
    float pitch;         ///< Pitch angle in degrees
    float yaw;           ///< Yaw angle in degrees
    float accelX;        ///< Processed accelerometer X-axis data (m/s^2)
    float accelY;        ///< Processed accelerometer Y-axis data (m/s^2)
    float accelZ;        ///< Processed accelerometer Z-axis data (m/s^2)
    float accelTime;     ///< Accelerometer timestamp
    float gyroX;         ///< Processed gyroscope X-axis data (deg/s)
    float gyroY;         ///< Processed gyroscope Y-axis data (deg/s)
    float gyroZ;         ///< Processed gyroscope Z-axis data (deg/s)
    float gyroTime;      ///< Gyroscope timestamp
    float velocityN;     ///< Velocity north (m/s)
    float velocityE;     ///< Velocity east (m/s)
    float velocityUp;    ///< Velocity up (m/s)
    float positionN;     ///< Position north (m)
    float positionE;     ///< Position east (m)
    float positionUp;    ///< Position up (m)
    float gpsSpeed;      ///< GPS speed (m/s)
    float gpsCourse;     ///< GPS course (deg)
    float gpsTime;       ///< GPS time
    float gpsLatitude;   ///< GPS latitude
    float gpsLongitude;  ///< GPS longitude
    float gpsAltitude;   ///< GPS altitude
    float magX;          ///< magnetometer X-axis data
    float magY;          ///< magnetometer Y-axis data
    float magZ;          ///< magnetometer Z-axis data
    float magTime;       ///< magnetometer timestamp
} CarMovementMeasurements;

/**
 * @class Redshift_UML7
 * @brief Class to interface with the UM7 sensor.
 */
class Redshift_UML7 {
public:
    /**
     * @brief Constructor for the Redshift_UML7 class.
     * @param serial Reference to the serial stream used for communication.
     */
    Redshift_UML7(Stream &serial);

    /**
     * @brief Initializes the UM7 sensor.
     */
    void begin();

    /**
     * @brief Retrieves data from the UM7 sensor.
     * @return A string containing the data.
     */
    String getData();

    /**
     * @brief Sends a command to the UM7 sensor.
     * @param command The command string to send.
     */
    void sendCommand(const String &command);

    /**
     * @brief Sets the baud rate for communication.
     * @param baudRate The desired baud rate.
     */
    void setBaudRate(unsigned long baudRate);

    /**
     * @brief Parses an NMEA packet.
     * @param nmeaString The NMEA string to parse.
     * @return True if parsing was successful, false otherwise.
     */
    bool parseNmeaPacket(String nmeaString);

    /**
     * @brief Reads register data.
     * @param registerAddress The address of the register to read.
     * @param data Pointer to the buffer to store the read data.
     * @param length The length of the data to read.
     * @return True if the data was successfully read, false otherwise.
     */
    bool read_register_data(uint8_t registerAddress, uint8_t *data, size_t length);

    /**
     * @brief Sends a binary command to the UM7 sensor.
     * @param command The command to send.
     * @return True if the command was successfully sent, false otherwise.
     */
    bool sendBinaryCommand(uint8_t command);

    /**
     * @brief Reads a register as a float value.
     * @param data Pointer to the buffer containing the register data.
     * @return The float value read from the register.
     */
    float read_register_as_float(uint8_t *data);

    /**
     * @brief Reads a packet from the UM7 sensor.
     * @param packet Reference to the packet structure to store the read data.
     * @param address The address of the packet to read.
     * @return True if the packet was successfully read, false otherwise.
     */
    bool readPacket(UM7_packet &packet, u_int8_t address);

    /**
     * @brief Reads a packet from the UM7 sensor.
     * @param packet Reference to the packet structure to store the read data.
     * @param packet_size The size of the packet to read.
     * @param address The address of the packet to read.
     * @return True if the packet was successfully read, false otherwise.
     */
    bool readPacket(UM7_packet &packet, int packet_size, u_int8_t address);

    /**
     * @brief Retrieves the firmware version of the UM7 sensor.
     * @return A string containing the firmware version.
     */
    bool getFirmwareVersion(String &fw_version);

    /**
     * @brief Retrieves the temperature from the UM7 sensor.
     * @param temp Reference to store the temperature value.
     * @return True if the temperature was successfully retrieved, false otherwise.
     */
    bool getTemperature(float &temp);

    /**
     * @brief Retrieves the GPS time from the UM7 sensor.
     * @param gpsTime Reference to store the GPS time.
     * @return True if the GPS time was successfully retrieved, false otherwise.
     */
    bool getGpsTime(float &gpsTime);

    /**
     * @brief Retrieves health register data from the UM7 sensor.
     * @param time Reference to store the time.
     * @param sats_used Reference to store the number of satellites used.
     * @param sats_in_view Reference to store the number of satellites in view.
     * @param HDOP Reference to store the HDOP value.
     * @param mode Reference to store the mode.
     * @param COM Reference to store the COM value.
     * @param accel Reference to store the accelerometer status.
     * @param gyro Reference to store the gyroscope status.
     * @param mag Reference to store the magnetometer status.
     * @param GPS Reference to store the GPS status.
     * @return True if the health register data was successfully retrieved, false otherwise.
     */
    bool getHealthRegisterData(uint16_t &time, uint8_t &sats_used, uint8_t &sats_in_view, float &HDOP, uint8_t &mode, uint8_t &COM, uint8_t &accel, uint8_t &gyro, uint8_t &mag, uint8_t &GPS);

    /**
     * @brief Parses serial data received from the UM7 sensor.
     * @param rx_data Pointer to the buffer containing the received data.
     * @param rx_length The length of the received data.
     * @param packet Pointer to the packet structure to store the parsed data.
     * @param expectedAddress The expected address of the packet.
     * @return The number of bytes parsed.
     */
    uint8_t parse_serial_data(uint8_t *rx_data, uint8_t rx_length, UM7_packet *packet, uint8_t expectedAddress);

    /**
     * @brief Retrieves the roll angle from the UM7 sensor.
     * @return The roll angle in degrees.
     */
    float getRoll();

    /**
     * @brief Retrieves the pitch angle from the UM7 sensor.
     * @return The pitch angle in degrees.
     */
    float getPitch();

    /**
     * @brief Retrieves the yaw angle from the UM7 sensor.
     * @return The yaw angle in degrees.
     */
    float getYaw();

    /**
     * @brief Retrieves the orientation (yaw, pitch, roll) from the UM7 sensor.
     * @param yaw Reference to store the yaw angle.
     * @param pitch Reference to store the pitch angle.
     * @param roll Reference to store the roll angle.
     * @return True if the orientation was successfully retrieved, false otherwise.
     */
    bool getOrientation(float &yaw, float &pitch, float &roll);

    /**
     * @brief Retrieves the magnetometer X-axis data.
     * @return The magnetometer X-axis data.
     */
    float getMagX();

    /**
     * @brief Retrieves the magnetometer Y-axis data.
     * @return The magnetometer Y-axis data.
     */
    float getMagY();

    /**
     * @brief Retrieves the magnetometer Z-axis data.
     * @return The magnetometer Z-axis data.
     */
    float getMagZ();

    /**
     * @brief Retrieves the processed gyroscope Z-axis data.
     * @param gyroZ Reference to store the gyroscope Z-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getGyroProcZ(float &gyroZ);

    /**
     * @brief Resets the UM7 sensor to factory defaults.
     * @return True if the reset was successful, false otherwise.
     */
    bool resetToFactoryDefaults();

    /**
     * @brief Zeros the gyroscopes of the UM7 sensor.
     * @return True if the gyroscopes were successfully zeroed, false otherwise.
     */
    bool zeroGyroscopes();

    /**
     * @brief Sets the home position for the UM7 sensor.
     * @return True if the home position was successfully set, false otherwise.
     */
    bool setHomePosition();

    /**
     * @brief Resets the Extended Kalman Filter (EKF) of the UM7 sensor.
     * @return True if the EKF was successfully reset, false otherwise.
     */
    bool resetEKF();

    /**
     * @brief Commits the current configuration settings to FLASH.
     * @return True if the settings were successfully committed, false otherwise.
     */
    bool flashCommit();

    /**
     * @brief Reads data from a data register.
     * @param dregAddress The address of the data register to read.
     * @param data Pointer to the buffer to store the read data.
     * @param length The length of the data to read.
     * @return True if the data was successfully read, false otherwise.
     */
    bool readDregData(uint8_t dregAddress, uint8_t *data, size_t length);

    /**
     * @brief Retrieves raw gyroscope measurements.
     * @param gyroX Reference to store the raw gyroscope X-axis data.
     * @param gyroY Reference to store the raw gyroscope Y-axis data.
     * @param gyroZ Reference to store the raw gyroscope Z-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getGyroRawMeasurements(int16_t &gyroX, int16_t &gyroY, int16_t &gyroZ);

    /**
     * @brief Retrieves raw accelerometer X and Y-axis measurements.
     * @param accelX Reference to store the raw accelerometer X-axis data.
     * @param accelY Reference to store the raw accelerometer Y-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelRawXY(int16_t &accelX, int16_t &accelY);

    /**
     * @brief Retrieves raw accelerometer Z-axis measurements.
     * @param accelZ Reference to store the raw accelerometer Z-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelRawZ(int16_t &accelZ);

    /**
     * @brief Retrieves processed gyroscope measurements.
     * @param gyroData Reference to a GyroMeasurement structure to store the data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getGyroProcessedMeasurements(GyroMeasurement &gyroData);

    bool getGyroProcessedMeasurements(float &gyroX, float &gyroY, float &gyroZ, float &gyroTime);

    /**
     * @brief Retrieves raw accelerometer measurements.
     * @param accelX Reference to store the raw accelerometer X-axis data.
     * @param accelY Reference to store the raw accelerometer Y-axis data.
     * @param accelZ Reference to store the raw accelerometer Z-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelRawMeasurements(int16_t &accelX, int16_t &accelY, int16_t &accelZ);

    /**
     * @brief Retrieves processed accelerometer X-axis data.
     * @param accelX Reference to store the processed accelerometer X-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelProcX(float &accelX);

    /**
     * @brief Retrieves processed accelerometer Y-axis data.
     * @param accelY Reference to store the processed accelerometer Y-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelProcY(float &accelY);

    /**
     * @brief Retrieves processed accelerometer Z-axis data.
     * @param accelZ Reference to store the processed accelerometer Z-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelProcZ(float &accelZ);

    /**
     * @brief Retrieves processed gyroscope X-axis data.
     * @param gyroX Reference to store the processed gyroscope X-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getGyroProcX(float &gyroX);

    /**
     * @brief Retrieves processed gyroscope Y-axis data.
     * @param gyroY Reference to store the processed gyroscope Y-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getGyroProcY(float &gyroY);

    /**
     * @brief Retrieves processed accelerometer measurements.
     * @param accelX Reference to store the processed accelerometer X-axis data.
     * @param accelY Reference to store the processed accelerometer Y-axis data.
     * @param accelZ Reference to store the processed accelerometer Z-axis data.
     * @param accelTime Reference to store the timestamp of the accelerometer data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelProcessedMeasurements(float &accelX, float &accelY, float &accelZ, float &accelTime);

    bool getAccelProcessedMeasurements(AccelMeasurement &accelData);

    bool getGPSMeasurements(GPSMeasurement &gpsData);

    bool getGPSMeasurements(float &latitude, float &longitude, float &altitude, float &time, float &course, float &speed);

    /**
     * @brief Prints all measurements to the serial output.
     */
    void printAllMeasurements();

    /**
     * @brief Retrieves raw magnetometer X and Y-axis measurements.
     * @param magX Reference to store the raw magnetometer X-axis data.
     * @param magY Reference to store the raw magnetometer Y-axis data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getMagRawXY(int16_t &magX, int16_t &magY);

    /**
     * @brief Prints the contents of a packet to the serial output.
     * @param packet The packet to print.
     */
    void printPacket(const UM7_packet &packet);

    bool getHealthRegisterData(uint8_t &sats_used, uint8_t &sats_in_view, float &HDOP, bool &ovf, bool &mg_n, bool &acc_n, bool &accel, bool &gyro, bool &mag, bool &gps);

    /**
     * @brief Reads serial data from the UM7 sensor.
     * @param buffer Pointer to the buffer to store the read data.
     * @param bufferSize The size of the buffer.
     * @return The number of bytes read.
     */
    size_t readSerialData(uint8_t *buffer, size_t bufferSize);

    /**
     * @brief Parses a packet received from the UM7 sensor.
     * @param rx_data Pointer to the buffer containing the received data.
     * @param rx_length The length of the received data.
     * @param packet Reference to the packet structure to store the parsed data.
     * @param expectedAddress The expected address of the packet.
     * @return True if the packet was successfully parsed, false otherwise.
     */
    bool parsePacket(const uint8_t *rx_data, size_t rx_length, UM7_packet &packet, uint8_t expectedAddress);

    /**
     * @brief Retrieves magnetic data from the UM7 sensor.
     * @param magX Reference to store the magnetometer X-axis data.
     * @param magY Reference to store the magnetometer Y-axis data.
     * @param magZ Reference to store the magnetometer Z-axis data.
     * @param time Reference to store the timestamp of the magnetometer data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getMagneticData(float &magX, float &magY, float &magZ, float &time);

    /**
     * @brief Retrieves the gyroscope timestamp from the UM7 sensor.
     * @param time Reference to store the gyroscope timestamp.
     * @return True if the timestamp was successfully retrieved, false otherwise.
     */
    bool getGyroTime(float &time);

    /**
     * @brief Retrieves the accelerometer timestamp from the UM7 sensor.
     * @param time Reference to store the accelerometer timestamp.
     * @return True if the timestamp was successfully retrieved, false otherwise.
     */
    bool getAccelTime(float &time);

    /**
     * @brief Retrieves raw accelerometer measurements.
     * @param accelData Reference to an AccelMeasurement structure to store the data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getAccelRawMeasurements(AccelMeasurement &accelData);

    /**
     * @brief Retrieves raw magnetometer measurements.
     * @param magData Reference to a MagMeasurement structure to store the data.
     * @return True if the data was successfully retrieved, false otherwise.
     */
    bool getMagRawMeasurements(MagMeasurement &magData);


    /**
     * @brief Retrieves all car movement measurements and stores them in the corresponding struct.
     * @param measurements Reference to a CarMovementMeasurements structure to store the data.
     * @return True if all measurements were successfully retrieved, false otherwise.
     */
    bool getCarMovementMeasurements(CarMovementMeasurements &measurements);

private:
    Stream &serial; ///< Reference to the serial stream used for communication.

    float roll, pitch, yaw; ///< Orientation data.
    String firmwareVersion; ///< Firmware version string.

    // Health data variables
    uint16_t health_time; ///< Health data time
    uint8_t health_sats_used; ///< Number of satellites used
    uint8_t health_sats_in_view; ///< Number of satellites in view
    float health_HDOP; ///< HDOP value
    uint8_t health_mode; ///< Mode
    uint8_t health_COM; ///< COM value
    uint8_t health_accel; ///< Accelerometer status
    uint8_t health_gyro; ///< Gyroscope status
    uint8_t health_mag; ///< Magnetometer status
    uint8_t health_GPS; ///< GPS status

    // Magnetometer data variables
    float magX, magY, magZ; ///< Magnetometer data
};

#endif // REDSHIFT_UML7_H