import logging
import time

from fastapi_app.app.integrations.influxdb.utils import dict_to_influxdb_points


def get_id_label(message_id: str) -> str:
    labels = {
        "0x588": "Battery Voltage",
        "0x468": "Engine RPM",
        "0x123": "Oil Pressure",
        "0x456": "Coolant Temperature",
        "0x789": "Fuel Level",
        "0xABC": "Throttle Position",
        "0xDEF": "Vehicle Speed",
        "0x101": "Engine Load",
        "0x202": "Intake Air Temperature",
        "0x303": "Mass Air Flow",
        "0x404": "Oxygen Sensor",
        "0x505": "Transmission Temperature"
    }
    return labels.get(message_id, "Unknown")



def parse_uml7_sensor(data: dict, device_id: str, measurement_prefix:str ="") -> list:
    '''
    :param measurement_prefix:
    :param device_id:
    :param data:
    :return:
    '''
    points = []

    tags = {
        "device": device_id
    }
    '''
    Example of data:
    {
    "car_movement":
    {
        "accel_time": 545.11,
        "accel_x": -0.78,
        "accel_y": 0.39,
        "accel_z": -0.54,
        "gps":
        {
            "altitude": 0.0,
            "course": 0.0,
            "latitude": 85.07,
            "longitude": 18473052.0,
            "speed": 0.0,
            "time": 78664.0
        },
        "gyro_time": 546.79,
        "gyro_x": -0.41,
        "gyro_y": -0.6,
        "gyro_z": 0.21,
        "mag_time": 548.43,
        "mag_x": -0.2,
        "mag_y": -0.57,
        "mag_z": 0.56,
        "pitch": -49.84,
        "roll": -36.05,
        "yaw": -125.46
    }
}
    '''

    points = dict_to_influxdb_points(data, tags, prefix=measurement_prefix)
    return points


def parse_can_messages(messages:list, device_id: str):

    '''
    :param messages:
    :param device_id:
    :return:

    Example of data:
    [
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"},
        {"id": "0x468", "timestamp": 0, "data": "0x094000810"},
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"},
        {"id": "0x468", "timestamp": 0, "data": "0x094000810"},
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"},
        {"id": "0x468", "timestamp": 0, "data": "0x094000810"},
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"},
        {"id": "0x468", "timestamp": 0, "data": "0x094000810"},
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"},
        {"id": "0x468", "timestamp": 0, "data": "0x094000810"},
        {"id": "0x588", "timestamp": 0, "data": "0xfe010051206"}
    ]

    '''



    logging.info(f"[CAN MESSAGES] Received {len(messages)} CAN messages")
    points = []

    for message in messages:
        timestamp = int(time.time())
        message_id = message.get("id", -1)
        message_data = message.get("data", "")
        label = get_id_label(message_id)

        tags = {
            "device": device_id,
            "id": str(message_id),
            "can_timestamp": message.get("timestamp", -1),
            "label": label
        }
        point = {
            "measurement": f"can_message",  # Use event name as measurement name
            "tags": tags,
            "fields": {
                "value": str(message_data)
            },  # Fields will be dynamically added based on event config
            "timestamp": timestamp
        }
        points.append(point)

    logging.info(f"[CAN MESSAGES] Generated {len(points)} points")
    logging.info(f"[CAN MESSAGES] Points: {points}")
    return points


def parse_json_device_data(data: dict):
    print(f"Parsing data: {data}")
    points = []
    if "device_id" in data.keys():
        device_id = data.get("device_id", "undefined")
    if "config" in data.keys():
        logging.warning(f"Configuration data found: {data.get('config', {})}")
    points += (dict_to_influxdb_points(data.get("config", {}), tags={"device": device_id}, prefix="config"))
    if "can_messages" in data.keys():
        logging.warning(f"CAN messages found: {data.get('can_messages', {})}")
    points += parse_can_messages(data.get("can_messages", []), device_id)
    if "uml7_measurements" in data.keys():
        logging.warning(f"UML7 sensor data found: {data.get('uml7_measurements', {})}")
    points += parse_uml7_sensor(data.get("uml7_measurements", {}), device_id, measurement_prefix="uml7")

    return points