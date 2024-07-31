import serial

# 시리얼 포트를 맞게 설정하세요.
ser = serial.Serial(
    port='/dev/ttyACM1',
    baudrate=115200,
    timeout=1
)

def decode_data(data):
    """Decodes the data by replacing ## with \x00"""
    return data.replace(b'##', b'\x00')

def read_serial_data():
    buffer = bytearray()
    
    while True:
        try:
            if ser.in_waiting > 0:
                # 시리얼 포트에서 데이터 읽기
                data = ser.read(ser.in_waiting)
                buffer.extend(data)
                
                # 데이터 복조하기
                decoded_data = decode_data(buffer)
                print(f"Decoded data: {decoded_data}")
                
                # 버퍼를 비우기
                buffer = bytearray()
        except Exception as e:
            print(f"Error reading from serial: {e}")

if __name__ == "__main__":
    try:
        read_serial_data()
    except KeyboardInterrupt:
        print("Exiting program")
    finally:
        ser.close()