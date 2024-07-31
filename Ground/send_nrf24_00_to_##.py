import serial
import socket
import threading
from queue import Queue

# Initialize the serial connection for the Arduino
ser = serial.Serial(
    port='/dev/ttyACM0',  # 시리얼 포트를 맞게 설정하세요.
    baudrate=115200,
    timeout=1
)

# Initialize queue for communication between threads
send_queue = Queue()

def encode_data(data):
    """Encodes the data by replacing \x00 byte with ##"""
    return data.replace(b'\x00', b'##')

def udp_receiver():
    udp_ip = "127.0.0.1"
    udp_port = 1234

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((udp_ip, udp_port))

    print(f"Listening on {udp_ip}:{udp_port}")
    while True:
        data, addr = sock.recvfrom(1024)
        print(f"Received UDP message from {addr}: {data}")
        send_queue.put(data)

def serial_sender():
    while True:
        try:
            if not send_queue.empty():
                message = send_queue.get()
                encoded_message = encode_data(message)
                ser.write(encoded_message)
                print(f"Sent to Arduino: {encoded_message}")
        except Exception as e:
            print(f"Error sending to Arduino: {e}")

def main():
    # Create and start threads
    udp_receiver_thread = threading.Thread(target=udp_receiver)
    serial_sender_thread = threading.Thread(target=serial_sender)

    udp_receiver_thread.start()
    serial_sender_thread.start()

    udp_receiver_thread.join()
    serial_sender_thread.join()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting program")
    finally:
        ser.close()