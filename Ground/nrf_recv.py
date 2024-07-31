import serial
import time

def read_from_arduino(serial_port='/dev/ttyACM0', baud_rate=115200):
    """
    아두이노와 시리얼 통신을 설정하고 데이터를 읽어들이는 함수
    :param serial_port: 시리얼 포트 이름 (Windows의 경우 'COM3', 'COM4' 등, Linux/Unix의 경우 '/dev/ttyACM0' 등)
    :param baud_rate: 보드레이트 (아두이노와 동일하게 설정해야 함)
    :param timeout: 읽기 타임아웃 (초 단위)
    :return: 아두이노로부터 읽은 데이터 문자열
    """
    try:
        with serial.Serial(serial_port, baud_rate) as ser:
            while True:
                if ser.in_waiting > 0:
                    data = ser.readline().decode('utf-8').rstrip()
                    print(f"Received data: {data}")
    except serial.SerialException as e:
        print(f"Serial exception: {e}")
    except KeyboardInterrupt:
        print("Program interrupted by user")

if __name__ == "__main__":
    read_from_arduino()
