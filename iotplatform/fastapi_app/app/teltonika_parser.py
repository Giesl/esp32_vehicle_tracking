import struct
import time

# Teltonika I/O Element configuration
IO_ELEMENT_CONFIG = {
    17: {"name": "acceleration_x", "type": "signed", "unit": "mG"},
    18: {"name": "acceleration_y", "type": "signed", "unit": "mG"},
    19: {"name": "acceleration_z", "type": "signed", "unit": "mG"},
    24: {"name": "speed", "type": "unsigned", "unit": "km/h"},
    16: {"name": "total_mileage", "type": "unsigned", "unit": "m"},
    48: {"name": "fuel_level", "type": "unsigned", "unit": "%"},
    759: {"name": "fuel_type", "type": "unsigned", "unit": ""},
    41: {"name": "throttle_position", "type": "unsigned", "unit": "%"},
    36: {"name": "engine_rpm", "type": "unsigned", "unit": "rpm"},
    31: {"name": "engine_load", "type": "unsigned", "unit": "%"},
    256: {"name": "vin", "type": "ascii", "unit": ""},
    239: {"name": "ignition", "type": "boolean", "unit": ""},
    240: {"name": "movement", "type": "boolean", "unit": ""},
    250: {"name": "trip_status", "type": "signed", "unit": ""},
    252: {"name": "unplug", "type": "signed", "unit": ""}
}


# Function to parse the incoming data and prepare it for InfluxDB
def parse_periodic_data(device_data) -> list:
    retval = []
    print("parsing periodic data")
    for key in IO_ELEMENT_CONFIG.keys():
        if str(key) in device_data.keys():
            retval.append(parse_event(key, device_data))

    return retval


def parse_event(event_id, device_data) -> dict:
    event_config = IO_ELEMENT_CONFIG.get(event_id, None)
    timestamp = int(time.time())  # Current Unix timestamp

    if event_config:
        event_name = event_config["name"]
        # Prepare the InfluxDB point
        influx_point = {
            "measurement": event_name,  # Use event name as measurement name
            "tags": {
                "device_IMEI": device_data.get("device_IMEI", None),
                "latitude": str(device_data.get("latitude", None)),
                "longitude": str(device_data.get("longitude", None)),
                "altitude": str(device_data.get("altitude", None)),
                "eventID": str(device_data.get("eventID", None)),
                "satelites": str(device_data.get("satelites", None)),
                "speed": float(device_data.get("speed", 0.0)),
                "angle": float(device_data.get("angle", 0.0)),
            },
            "fields": {},  # Fields will be dynamically added based on event config
            "timestamp": timestamp
        }

        # Based on eventID, we could also add specific fields (e.g., acceleration, speed, etc.)
        if event_id in IO_ELEMENT_CONFIG:
            field_name = event_config["name"]
            field_value = device_data.get(str(event_id))

            if field_value is not None:
                if event_config["type"] == "signed":
                    influx_point["fields"]["value"] = int(field_value)  # For signed types
                elif event_config["type"] == "unsigned":
                    influx_point["fields"]["value"] = int(field_value)  # For unsigned types
                elif event_config["type"] == "ascii":
                    influx_point["fields"]["value"] = 1
                    influx_point["tags"][field_value] = str(field_value)  # For ASCII types
                elif event_config["type"] == "boolean":
                    influx_point["fields"]["value"] = int(field_value)  # For ASCII types

        return influx_point


def teltonika_prepare_data_for_influxdb(device_data) -> list:
    influxdb_data = []

    # Get the event name using eventID
    event_id = device_data["eventID"]
    if event_id == 0:  # periodic dataf
        influxdb_data = parse_periodic_data(device_data)
    else:
        influxdb_data.append(parse_event(event_id, device_data))

    return influxdb_data




def parse_io_elements(data: bytes, offset: int, codec: str) -> tuple:
    """
    Parse I/O elements for both Codec 8 and Codec 8 Extended.
    """
    io_data = {}

    if codec == "extended":  # Codec 8 Extended
        total_io = struct.unpack(">H", data[offset:offset + 2])[0]
        offset += 2
    else:  # Codec 8
        total_io = data[offset]
        offset += 1

    for _ in range(total_io):
        # ID and Value sizes differ in Codec 8 Extended
        if codec == "extended":
            id_size = data[offset]
            io_id = int.from_bytes(data[offset + 1:offset + 1 + id_size], "big")
            offset += 1 + id_size
            value_size = data[offset]
            offset += 1
        else:
            io_id = data[offset]
            offset += 1
            value_size = data[offset]
            offset += 1

        value_data = data[offset:offset + value_size]
        offset += value_size

        if io_id in IO_ELEMENT_CONFIG:
            element = IO_ELEMENT_CONFIG[io_id]
            if element["type"] == "unsigned":
                value = int.from_bytes(value_data, "big")
            elif element["type"] == "signed":
                value = int.from_bytes(value_data, "big", signed=True)
            elif element["type"] == "ascii":
                value = value_data.decode("ascii").strip()
            elif element["type"] == "boolean":
                value = bool.from_bytes(value_data, "big")
            else:
                value = value_data.hex()
            io_data[element["name"]] = value

    return io_data, offset


def parse_teltonika_data(data: bytes) -> dict:
    """
    Parses binary data from a Teltonika FMB device.
    Supports Codec 8 and Codec 8 Extended. Logs unknown codec IDs.
    """
    try:
        # Check minimum packet length
        if len(data) < 8:
            raise ValueError("Invalid data length")

        # Read data length (first 4 bytes)
        data_length = struct.unpack(">I", data[:4])[0]
        codec_id = data[4]

        # Supported codec IDs
        if codec_id == 0x08:
            codec = "standard"
        elif codec_id == 0x8E:
            codec = "extended"
        else:
            raise ValueError(f"Unsupported codec ID: {codec_id}. Please verify the payload.")

        # Number of records
        records_count = data[5]
        parsed_data = {"codec_id": codec_id, "records_count": records_count, "records": []}

        # Rest of the parsing logic...
        # (same as before for timestamp, GPS, and I/O elements)

        return parsed_data

    except Exception as e:
        return {"error": str(e)}

# Example usage
if __name__ == "__main__":
    # Example binary payload for Codec 8 Extended (replace with actual payload data)
    payload = b'00000000000001598e03000001933f17e1b0000a65cd5a1d4065c000ce004a0c000000000013000700ef0100f00100150400c800004501007164012f00000900b5000a00b60008004234f80018000000430fcb00440000001100080012ffee0013ffde000300f1000059d900c7000006e900100000221c00000000000001933f1cb7f8000a65cd5a1d4065c000ce004a0d000000ef0013000700ef0000f00000150400c800004501007164012f00000900b5000c00b600070042338b0018000000430fcb00440000001100100012fff80013ffe4000300f1000059d900c7000006e900100000221c00000000000001933f1dae10000a65cd5a1d4065c000ce004a0e000000fa0014000800ef0000f00000150400c800004501007164012f0000fa00000900b5000c00b60007004233610018000000430fcb00440000001100170012fff50013ffe4000300f1000059d900c7000006e900100000221c00000000030000adde'  # Replace with real binary data
    result = parse_teltonika_data(payload)
    print(result)
