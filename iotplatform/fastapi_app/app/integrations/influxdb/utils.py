import logging
import os
import time

from influxdb_client import Point, InfluxDBClient
import asyncio
from influxdb_client.client.write_api import SYNCHRONOUS

influx_token = os.environ.get("INFLUX_TOKEN", "viGZA7zu-lkoFQpplbBZjTezXAKvmxRwziZFcU-xzF0Qq_wiERbCc6tbFVQ7cDW2WlHOunAALlUuyGdlgU0FDg==")
influx_org = os.environ.get("INFLUX_ORG", "9934126df83fdfe2")


def print_influxdb_points(points: list):
    point: Point
    for point in points:
        print(point)


def dict_to_influxdb_points(data: dict, tags: dict, prefix:str = "") -> list:
    points = []
    if data is None:
        logging.warning("Received None data")
        return points

    timestamp = int(time.time())
    for key, value in data.items():
        if type(value) is dict:
            logging.warn(f"Nested dict found: {value}")
            nested_points = dict_to_influxdb_points(value, tags, prefix=f"{prefix}_{key}")
            logging.warn(f"Nested points: {nested_points}")
            points += nested_points
        else:
            if type(value) is list:
                value = f"{value}"
            influx_point = {
                "measurement": f"{prefix}_{key}",  # Use event name as measurement name
                "tags": tags,
                "fields": {
                    "value": value
                },  # Fields will be dynamically added based on event config
                "timestamp": timestamp
            }
            points.append(influx_point)
    return points




async def write_to_influxdb( points: list, bucket: str = "main", client: InfluxDBClient = None):
    print("Points to write into influxdb")
    for point in points:
        print(point)
    if client is None:
        client = InfluxDBClient(url="http://localhost:8086", token=influx_token, org=influx_org)
        write_api = client.write_api(write_options=SYNCHRONOUS)
        write_api.write(org=influx_org, bucket=bucket, record=points)
        client.close()
    else:
        write_api = client.write_api(write_options=SYNCHRONOUS)
        write_api.write(org=influx_org, bucket=bucket, record=points)


if __name__ == "__main__":
    test_points = [
        {
            "measurement": "test_measurement",
            "tags": {"device": "test_device"},
            "fields": {"value": 1},
            "timestamp": int(time.time())
        }
    ]
    asyncio.run(write_to_influxdb(test_points))
