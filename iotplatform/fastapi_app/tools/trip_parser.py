import json

def parse_trips(file_path):
    trips = []
    current_trip = []

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if "CAN Shield started" in line:
                if current_trip:
                    if len(current_trip) > 1:
                        trips.append(current_trip)
                current_trip = [line]
            else:
                if current_trip:
                    current_trip.append(line)

    if current_trip:
        if len(current_trip) > 1:
            trips.append(current_trip)

    return trips

def save_trips(trips, output_dir):
    for i, trip in enumerate(trips):
        if len(trip) <= 1:
            continue
        output_file = f"{output_dir}/trip_{i + 1}.txt"
        with open(output_file, 'w') as file:
            file.write("\n".join(trip))
        print(f"Trip {i + 1} saved to {output_file}")

if __name__ == "__main__":
    input_file = "measurements_log.txt"
    output_directory = "./trips"

    import os
    os.makedirs(output_directory, exist_ok=True)

    trips = parse_trips(input_file)
    save_trips(trips, output_directory)