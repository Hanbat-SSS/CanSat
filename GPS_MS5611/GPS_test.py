import serial
import pynmea2
import time
import logging
from datetime import datetime

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
file_path = "/home/test2/GPS_data.txt"
def convert_to_decimal(degree, minutes):
    return degree + minutes / 60

def parse_gps(data):
    data_str = data.decode('ascii', errors='ignore').strip()
    if 'GGA' in data_str:
        try:
            msg = pynmea2.parse(data_str)

            # 위도와 경도 값 초기화
            lat_degree = 0
            lat_minutes = 0.0
            lon_degree = 0
            lon_minutes = 0.0

            # 위도 값이 비어 있지 않은지 확인
            if msg.lat and len(msg.lat) >= 4:  # 최소 4자리는 있어야 유효 (도 2자리 + 분 2자리 이상)
                lat_degree = int(msg.lat[:2])
                lat_minutes = float(msg.lat[2:])
            else:
                logging.warning(f"Invalid latitude value: {msg.lat}")

            # 경도 값이 비어 있지 않은지 확인
            if msg.lon and len(msg.lon) >= 5:  # 최소 5자리는 있어야 유효 (도 3자리 + 분 2자리 이상)
                lon_degree = int(msg.lon[:3])
                lon_minutes = float(msg.lon[3:])
            else:
                logging.warning(f"Invalid longitude value: {msg.lon}")

            # 위도/경도 값을 소수점 형식으로 변환
            lat_decimal = convert_to_decimal(lat_degree, lat_minutes)
            lon_decimal = convert_to_decimal(lon_degree, lon_minutes)

            # 방향 처리
            if msg.lat_dir == 'S':
                lat_decimal = -lat_decimal
            if msg.lon_dir == 'W':
                lon_decimal = -lon_decimal
            local = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            # 출력
            logging.info(
                "Timestamp: %s -- Lat: %.6f -- Lon: %.6f -- Altitude: %s %s",
                msg.timestamp, lat_decimal, lon_decimal, msg.altitude, msg.altitude_units
            )
            with open(file_path, "a") as file:
                file.write(f"Timestamp: {local} -- Lat: {lat_decimal} -- Lon: {lon_decimal} \n")
        except pynmea2.ParseError as e:
            logging.warning(f"Parse error: {e}. Data: {data_str}")
        except ValueError as e:
            logging.error(f"Value error: {e}. Data: {data_str}")

def main():
    while True:
        try:
            with serial.Serial("/dev/ttyS0", 9600, timeout=1) as serial_port:
                logging.info("Listening for GPS data...")
                
                while True:
                    data = serial_port.readline()
                    if data:
                        parse_gps(data)
                    else:
                        logging.debug("No data received. Retrying...")
                    
        except serial.SerialException as e:
            logging.error(f"Serial exception: {e}. Retrying in 5 seconds...")
            time.sleep(5)

if __name__ == "__main__":
    main()
