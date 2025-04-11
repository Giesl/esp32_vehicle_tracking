import asyncio
import json
import logging
import os
import time
from datetime import datetime, timedelta
from typing import Dict
from urllib.request import Request
import base64
from influxdb_client import Point, WritePrecision

from pydantic import Field, BaseModel
from fastapi import FastAPI, HTTPException, Depends, UploadFile, File, Form, Body
from fastapi.exceptions import RequestValidationError
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from influxdb_client.client.influxdb_client import InfluxDBClient
from pydantic import BaseModel
from starlette import status
from starlette.middleware.cors import CORSMiddleware

from fastapi_app.app.esp32_parser import parse_json_device_data
from fastapi_app.app.integrations.influxdb.utils import write_to_influxdb
from starlette.responses import JSONResponse

from fastapi_app.app.models import TeltonikaData
from fastapi_app.app.teltonika_parser import teltonika_prepare_data_for_influxdb

# InfluxDB configuration
influx_token = os.environ.get("INFLUX_TOKEN", "viGZA7zu-lkoFQpplbBZjTezXAKvmxRwziZFcU-xzF0Qq_wiERbCc6tbFVQ7cDW2WlHOunAALlUuyGdlgU0FDg==")
influx_org = os.environ.get("INFLUX_ORG", "9934126df83fdfe2")
influx_client = InfluxDBClient(url="http://influxdb:8086", token=influx_token, org=influx_org)


app = FastAPI(
    title="API for vehicle tracking",
    description="API for esp32 and teltonika devices",
    version="1.0",
    docs_url="/docs",  # URL for the Swagger UI
    redoc_url="/redoc",  # URL for the ReDoc UI
    openapi_url="/openapi.json",  # URL for the OpenAPI schema
    default_response_class=JSONResponse,  # Default response class
    default_status_code=status.HTTP_200_OK,  # Default status code
    default_response_description="Success",  # Default response description
    title_format="Vehicle Tracking API",  # Title format
    description_format="API for esp32 and teltonika devices",  # Description format

)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Replace "*" with specific origins if needed
    allow_credentials=True,
    allow_methods=["*"],  # Allow all HTTP methods
    allow_headers=["*"],  # Allow all headers
)

# List of valid API tokens
VALID_API_TOKENS = ["token1", "token2", "token3"]
#### postgres configuration
postgres_user = "postgres"
postgres_password = "eQG7hSXiZTn7e0E7aDkcSec3"
postgres_database = "postgres"
postgres_host = "postgres"


class SuccessMessage(BaseModel):
    message: str = Field(..., example="Operation successful")


# --- Authentication ---
bearer_scheme = HTTPBearer() # Define the scheme
# Simple authentication middleware
async def authenticate_api_token(credentials: HTTPAuthorizationCredentials = Depends(bearer_scheme)):
    """
    Validates the Bearer token provided in the Authorization header.

    Raises:
        HTTPException(401): If the scheme is not 'bearer'.
        HTTPException(401): If the token is not in the VALID_API_TOKENS list.
    """
    if credentials.scheme.lower() != "bearer":
        logging.warning(f"Invalid authentication scheme: {credentials.scheme}")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid authentication scheme. Use Bearer.",
            headers={"WWW-Authenticate": "Bearer"},
        )
    if credentials.credentials not in VALID_API_TOKENS:
        logging.warning(f"Invalid API token received: {credentials.credentials[:5]}...") # Log prefix only
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid or expired API token",
            headers={"WWW-Authenticate": "Bearer"},
        )
    # Log successful authentication attempt if needed
    # logging.info(f"Authentication successful for token: {credentials.credentials[:5]}...")
    return True # Or return user info/token data if needed later


# debugging validation error
@app.exception_handler(RequestValidationError)
async def validation_exception_handler(request: Request, exc: RequestValidationError):
    exc_str = f'{exc}'.replace('\n', ' ').replace('   ', ' ')
    content = {'status_code': 422, 'message': exc_str, 'data': None}
    print(f"{request.url}({request.base_url}) - {request.query_params}: {exc_str}")
    return JSONResponse(content=content, status_code=status.HTTP_422_UNPROCESSABLE_ENTITY)

# --- API Endpoints ---

@app.post(
    "/canbus_data",
    tags=["ESP32"], # Group in Swagger UI
    summary="Receive and process CAN bus data from ESP32 devices",
    description="Receives JSON data typically sent from an ESP32 device reading a vehicle's CAN bus. Parses the data and writes relevant points to InfluxDB.",
    response_model=SuccessMessage, # Model for the success response body
    status_code=status.HTTP_200_OK, # Default success status code
    responses={ # Define possible responses
        200: {"description": "Data received and queued for processing.", "model": SuccessMessage},
        401: {"description": "Authentication required or invalid token."},
        422: {"description": "Validation Error (e.g., malformed JSON or incorrect data types)."},
        500: {"description": "Internal server error during processing or InfluxDB write."},
    },
    dependencies=[Depends(authenticate_api_token)] # Apply authentication
)
async def receive_canbus_data(data: dict):
    """
        Endpoint to receive CAN bus data from ESP32.

        - **data**: The main JSON payload containing device ID, timestamp, and CAN readings.
        """
    # Process the received data here
    print(f'Received CAN bus data: {data}')

    # Write data to a file
    with open("/app/data/canbus_data.json", "a") as file:
        line = f"{datetime.now().timestamp()} {json.dumps(data, sort_keys=True)}"
        file.write(f"{line}\n")

    logging.warning(f"Received keys in data: {data.keys()}")

    points = parse_json_device_data(data)
    if len(points) > 0:
        logging.info(f"Writting {len(points)} points to InfluxDB")
        await asyncio.gather(write_to_influxdb(points, client=influx_client))

    return {"message": "Data received successfully"}

@app.post(
    "/receive_data",
    tags=["Teltonika"], # Group in Swagger UI
    summary="Receive and process data from Teltonika devices",
    description="Receives JSON data typically sent from Teltonika tracking devices. Parses the data and writes relevant points to InfluxDB.",
    response_model=SuccessMessage,
    status_code=status.HTTP_200_OK,
    responses={
        200: {"description": "Data received and queued for processing.", "model": SuccessMessage},
        401: {"description": "Authentication required or invalid token."},
        422: {"description": "Validation Error (e.g., malformed JSON or incorrect data types)."},
        500: {"description": "Internal server error during processing or InfluxDB write."},
    },
    dependencies=[Depends(authenticate_api_token)] # Apply authentication
)
async def receive_data(data: TeltonikaData = Body(...)):
    """
        Endpoint to receive data payloads from Teltonika devices.

        - **data**: The main JSON payload containing device IMEI and data records.
        """
    # Process the received data here
    print(f'Received data for device IMEI {data.device_IMEI}: {data}')
    data_dict = data.model_dump()
    influx_data = teltonika_prepare_data_for_influxdb(data_dict)
    # print(json.dumps(influx_data, indent=4))
    points = influx_data
    await asyncio.gather(write_to_influxdb(points))
    return {"message": "Data received successfully"}

@app.post(
    "/upload_file",
    tags=["Import"], # Group in Swagger UI
    summary="Upload and process base64 encoded CAN log files",
    description="Receives a JSON payload containing a start timestamp and a list of base64 encoded strings (representing log file content from ESP32 devices). Decodes the data, parses CAN messages relative to the start time, and writes points to InfluxDB. Processes only the *first* route found in the file.",
    response_model=SuccessMessage,
    status_code=status.HTTP_200_OK,
    responses={
        200: {"description": "File processed successfully and data written to InfluxDB.", "model": SuccessMessage},
        400: {"description": "Bad Request: Missing 'file' or 'start_time', invalid types, error decoding base64/JSON, or invalid start time format."},
        401: {"description": "Authentication required or invalid token."},
        422: {"description": "Validation Error (request body doesn't match expected format)."},
        500: {"description": "Internal server error during processing or InfluxDB write."},
    },
    dependencies=[Depends(authenticate_api_token)] # Apply authentication
)
async def upload_file(data: dict):
    """
    Endpoint to upload and process historical CAN data from log files.

    Expects a JSON body with:
    - **file**: A list of strings, where each string is base64 encoded text data from a log file.
    - **start_time**: An ISO 8601 formatted string representing the timestamp of the first log entry.

    *Note:* This endpoint currently processes only the data corresponding to the first detected 'CAN Shield started' message in the decoded content.
    """

    if not "file" in data.keys():
        raise HTTPException(status_code=400, detail="File not provided")
    if not "start_time" in data.keys():
        raise HTTPException(status_code=400, detail="Start time not provided")

    if not isinstance(data["file"], list):
        raise HTTPException(status_code=400, detail="File is not a list")

    if not isinstance(data["start_time"], str):
        raise HTTPException(status_code=400, detail="Start time is not a string")

    # Process the received data here
    data_str: str
    routes = 0
    total_points = 0
    routes_data = []
    for data_str in data["file"]:
        # Extract the Base64-encoded portion
        if data_str.startswith("data:text/plain;base64,"):
            base64_data = data_str.split(",", 1)[1]  # Get the part after 'base64,'
            try:
                decoded_data = base64.b64decode(base64_data).decode("utf-8")  # Decode Base64 and convert to string
                print(f"Decoded data: {decoded_data}")

                with open(f"/app/data/imported/decoded_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json", "a") as decoded_file:
                    decoded_file.write("start_time=" + data["start_time"] + "\n")
                    decoded_file.write(decoded_data)


                # Process the decoded data as needed
                lines = decoded_data.splitlines()
                last_millis = 0
                route_data = []
                influx_points = []
                for line in lines:
                    try:
                        json_data = json.loads(line.split(";")[1])
                        if json_data.get("can_message", "") == "CAN Shield started":
                            if routes == 1:
                                logging.warning(f"Only one route is processed, please adjust the file")
                                break

                            # {"can_message": "CAN Shield started", "device_id": "14:2B:2F:C5:D3:E8", "fw_version": "U72B"}
                            # new route
                            if len(route_data) > 0:
                                routes_data.append(route_data)
                            print("NEW ROUTE")
                            last_millis = 0
                            routes += 1
                            route_data = []


                        else:
                            millis: int = int(line.split(";")[0])
                            if last_millis > millis:
                                continue
                            last_millis = millis
                            # Process the JSON data as needed
                            #print(f"JSON data: {json_data}")
                            route_data.append((millis, json_data))
                            total_points += 1

                    except json.JSONDecodeError as e:
                        raise HTTPException(status_code=400, detail=f"Error decoding data in file: {str(e)}")

                if len(route_data) > 0:
                    routes_data.append(route_data)
                print(f"File has {routes} routes and {total_points} data points")
                print(f"Start time: {data['start_time']}")
                print(f"Routes data({len(routes_data)}): {routes_data}")\

                start_str = data.get("start_time", "")
                start_dt = None
                if len(start_str) > 0:
                    try:
                        start_dt = datetime.strptime(start_str, "%Y-%m-%dT%H:%M:%S.%fZ")
                    except ValueError as e:
                        raise HTTPException(status_code=400, detail=f"Error parsing start time: {str(e)}")
                for route in routes_data:
                    print("START ROUTE")
                    # print(route)
                    for millis, point in route:
                        # Convert milliseconds to datetime
                        point_time = start_dt + timedelta(milliseconds=millis)
                        data_points = parse_json_device_data(point)
                        for data_point in data_points:
                            # Add the timestamp to the point
                            data_point["timestamp"] = int(point_time.timestamp() * 1e9)  # Convert to nanoseconds
                            data_point["tags"]["inserted"] = 1
                            point = Point(data_point["measurement"])
                            for tag_key, tag_value in data_point["tags"].items():
                                point = point.tag(tag_key, tag_value)
                            for field_key, field_value in data_point["fields"].items():
                                point = point.field(field_key, field_value)
                            point = point.time(data_point["timestamp"], WritePrecision.NS)
                            influx_points.append(point)

                    print("END ROUTE")
                    break

                print("WRITING TO INFLUXDB")
                await asyncio.gather(write_to_influxdb(influx_points, client=influx_client))

            except Exception as e:
                print(f"Exception occurred: {str(e)}")
                raise HTTPException(status_code=400, detail=f"Error decoding Base64 data: {str(e)}")



            return {"message": "File processed successfully"}

if __name__ == "__main__":
    import uvicorn

    influx_status = influx_client.ping()
    if influx_status:
        print("InfluxDB is reachable")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    else:
        print("InfluxDB is not reachable")

