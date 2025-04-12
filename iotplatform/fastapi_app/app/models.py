from __future__ import annotations

from typing import Optional, Dict, Any, Union, List, Literal  # Union is useful for the extra values

from fastapi import UploadFile, File
from pydantic import BaseModel, Field, Extra

# --- TELTONIKA DATA MODEL ---
class TeltonikaData(BaseModel):
    """
    Represents data from a Teltonika device.

    Includes fixed required fields and allows for additional, arbitrary key-value pairs
    (often representing Teltonika IO element IDs).
    """

    # --- Required Fields ---
    device_IMEI: str = Field(..., example="123456789012345", description="IMEI of the Teltonika device")
    server_time: str = Field(..., example="13:28:51 18-11-2024 (local) / 12:28:51 18-11-2024 (utc)", description="Timestamp from the receiving server")
    _timestamp_: str = Field(..., alias="_timestamp_", example="12:51:07 18-11-2024 (local) / 11:51:07 18-11-2024 (utc)", description="Timestamp from the device record")
    _rec_delay_: str = Field(..., alias="_rec_delay_", example="2264 seconds", description="Delay between record creation and server reception")
    priority: int = Field(..., example=0, description="Record priority level")
    longitude: float = Field(..., example=17.4443866)
    latitude: float = Field(..., example=49.0759616)
    altitude: int = Field(..., example=206, description="Altitude in meters")
    angle: int = Field(..., example=74, description="Course angle in degrees")
    satelites: int = Field(..., example=13, description="Number of visible satellites (Note: JSON key typo 'satelites' must be matched)") # Match the key typo in JSON
    speed: int = Field(..., example=0, description="Vehicle speed in km/h")
    eventID: int = Field(..., example=239, description="ID of the I/O element that triggered the record")

    # --- Handling Variable/Optional Fields (Pydantic V1) ---
    class Config:
        # Allow extra fields not explicitly defined in the model
        extra = Extra.allow
        # If you used aliases like timestamp_ for _timestamp_, tell Pydantic V1
        allow_population_by_field_name = True  # Allows using both alias and field name if needed

# --- TELTONIKA DATA MODEL END ---


# --- ESP DATA MODEL ---
class CanMessageItem(BaseModel):
    """Model for a single message within the 'can_messages' list"""
    data: str = Field(..., example="0x21af00000", description="CAN message data payload (hex string)")
    id: str = Field(..., example="0x100", description="CAN message ID (hex string)")
    timestamp: int = Field(..., example=0, description="Timestamp relative to message sequence start (integer ms or similar)")

class ConfigurationModel(BaseModel):
    """Model for the nested 'configuration' object (in status messages)"""
    http_endpoint: Optional[str] = Field(default=None, example="http://iot.giesl.cz/canbus_data", description="HTTP endpoint URL")
    sd_card_enabled: Optional[int] = Field(default=None, example=0, description="SD card status (0=disabled, 1=enabled)") # Or bool?
    wifi_enabled: Optional[int] = Field(default=None, example=1, description="WiFi status (0=disabled, 1=enabled)") # Or bool?

class ConfigModel(BaseModel):
    """Model for the nested 'config' object (in data messages - different structure than 'configuration')"""
    can_enabled: Optional[int] = Field(default=None, example=1)
    can_filter_enabled: Optional[int] = Field(default=None, example=0)
    can_scrape_interval: Optional[int] = Field(default=None, example=15000, description="CAN scraping interval in ms")
    filtered_can_ids: Optional[List[str]] = Field(default=None, example=["0x100", "0x200"], description="List of filtered CAN IDs")
    http_endpoint: Optional[str] = Field(default=None, example="http://iot.giesl.cz/canbus_data", description="HTTP endpoint URL")
    http_send_interval: Optional[int] = Field(default=None, example=30000, description="HTTP sending interval in ms")
    log_file: Optional[str] = Field(default=None, example="/can_log.txt", description="Log file path on device")
    sd_card_enabled: Optional[int] = Field(default=None, example=1)
    um7_enabled: Optional[int] = Field(default=None, example=1, description="UM7 sensor status")
    uml7_scrape_interval: Optional[int] = Field(default=None, example=30000, description="UM7 sensor scraping interval in ms")
    wifi_enabled: Optional[int] = Field(default=None, example=1)

class Uml7GpsModel(BaseModel):
    """Model for GPS data within UM7 measurements"""
    altitude: Optional[float] = Field(default=None)
    course: Optional[float] = Field(default=None)
    latitude: Optional[float] = Field(default=None)
    longitude: Optional[float] = Field(default=None)
    speed: Optional[float] = Field(default=None)
    time: Optional[float] = Field(default=None, description="GPS time data")

class Uml7CarMovementModel(BaseModel):
    """Model for car movement data within UM7 measurements"""
    accel_time: Optional[float] = Field(default=None)
    accel_x: Optional[float] = Field(default=None)
    accel_y: Optional[float] = Field(default=None)
    accel_z: Optional[float] = Field(default=None)
    gps: Optional[Uml7GpsModel] = Field(default=None)
    gyro_time: Optional[float] = Field(default=None)
    gyro_x: Optional[float] = Field(default=None)
    gyro_y: Optional[float] = Field(default=None)
    gyro_z: Optional[float] = Field(default=None)
    mag_time: Optional[float] = Field(default=None)
    mag_x: Optional[float] = Field(default=None)
    mag_y: Optional[float] = Field(default=None)
    mag_z: Optional[float] = Field(default=None)
    pitch: Optional[float] = Field(default=None)
    roll: Optional[float] = Field(default=None)
    yaw: Optional[float] = Field(default=None)

class Uml7MeasurementsModel(BaseModel):
    """Model for the nested 'uml7_measurements' object"""
    # If these keys might be absent even in a data message, mark them Optional
    car_movement: Optional[Uml7CarMovementModel] = Field(default=None)
    # Using a generic dict for health_data as its structure isn't defined in examples
    health_data: Optional[Dict[str, Any]] = Field(default=None, description="UM7 health data")

# Example STARTUP JSON:
"""
{
    "device_id": "14:2B:2F:C5:D3:E8",
    "fw_version": "U72B",
    "message": "CAN Shield started"
}
"""
class StartupMessageModel(BaseModel):
    """Model for a startup message"""
    message_type: Literal["startup"] = Field("startup", description="Internal type identifier", exclude=True)
    device_id: str = Field(..., description="Unique device identifier")
    fw_version: Optional[str] = Field(default=None, description="Device firmware version")
    message: Optional[str] = Field(default=None, description="Startup or informational message") # Key is "message" here


# Example STATUS JSON:
"""
{
    "can_messages": [{"data": "0x21af00000", "id": "0x100", "timestamp": 0}, {"data": "0x341c1af8000", "id": "0x200", "timestamp": 0}],
    "config": {"can_enabled": 1, "can_filter_enabled": 0, "can_scrape_interval": 15000, "filtered_can_ids": ["0x100", "0x200", "0x300"], "http_endpoint": "http://iot.giesl.cz/canbus_data", "http_send_interval": 30000, "log_file": "/can_log.txt", "sd_card_enabled": 1, "um7_enabled": 1, "uml7_scrape_interval": 30000, "wifi_enabled": 1},
    "device_id": "14:2B:2F:C5:D3:E8",
    "uml7_measurements": {"car_movement": {"accel_time": 62.16, "accel_x": 0.01, "accel_y": 0.03, "accel_z": -1.05, "gps": {"altitude": 0.0, "course": 0.0, "latitude": 0.0, "longitude": 0.0, "speed": 0.0, "time": 0.0}, "gyro_time": 63.85, "gyro_x": 0.02, "gyro_y": 0.22, "gyro_z": 0.24, "mag_time": 65.49, "mag_x": -0.76, "mag_y": -0.17, "mag_z": 0.32, "pitch": 0.87, "roll": -1.58, "yaw": -126.19}, "health_data": {}}
}
"""
class DataMessageModel(BaseModel):
    """Model for a data message with measurements"""
    message_type: Literal["data"] = Field("data", description="Internal type identifier", exclude=True)
    device_id: str = Field(..., description="Unique device identifier")
    config: Optional[ConfigModel] = Field(default=None, description="Device operational configuration")
    can_messages: Optional[List[CanMessageItem]] = Field(default=None, description="List of collected CAN messages")
    uml7_measurements: Optional[Uml7MeasurementsModel] = Field(default=None, description="Measurements from UM7 sensor")


# --- Union of all possible message models ---
CombinedESPMessage = Union[DataMessageModel, StartupMessageModel]
# --- ESP DATA MODEL END ---

