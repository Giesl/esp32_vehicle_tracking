from __future__ import annotations

from typing import Optional, Dict, Any, Union # Union is useful for the extra values

from fastapi import UploadFile, File
from pydantic import BaseModel, Field, Extra


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





