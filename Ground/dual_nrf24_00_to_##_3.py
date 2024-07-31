import serial
import time
import threading
import socket
from queue import Queue

# Initialize the serial connections for the two Arduinos
ser_1 = serial.Serial(
    port='/dev/ttyACM1',
    baudrate=115200,
    timeout=1
)

ser_2 = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=115200,
    timeout=1
)

# Initialize queues for communication between threads
send_queue = Queue()
receive_queue = Queue()

def encode_data(data):
    """Encodes the data by replacing 00 byte with #"""
    return data.replace(b'\x00', b'#')

def decode_data(data):
    """Decodes the data by replacing # with 00 byte"""
    return data.replace(b'#', b'\x00')

def deexctract_data(data):
    """Decodes the data by replacing @ with 00 byte"""
    return data.replace(b'@', b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')

def serial_receiver():
    buffer = bytearray()
    while True:
        try:
            if ser_2.in_waiting > 0:
                line = ser_2.read(ser_2.in_waiting)  # Read all available data
                buffer.extend(line)
                if b'\n' in buffer:
                    # Split the buffer at the newline character
                    complete_data, buffer = buffer.split(b'\n', 1)
                    complete_data += b'\n'
                    print(f"Received from Arduino (ser_2): {complete_data}")
                    receive_queue.put(complete_data)
        except Exception as e:
            print(f"Error receiving from Arduino (ser_2): {e}")

def serial_sender():
    while True:
        try:
            if not send_queue.empty():
                message = send_queue.get()
                encoded_message = encode_data(message)
                ser_1.write(encoded_message)
                print(f"Sent to Arduino (ser_1): {encoded_message}")
        except Exception as e:
            print(f"Error sending to Arduino (ser_1): {e}")

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

def udp_sender():
    udp_ip = "127.0.0.1"
    udp_port = 1235

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        if not receive_queue.empty():
            message = receive_queue.get()
            decoded_message = decode_data(message)
            deextracted_data = deexctract_data(decoded_message)
            sock.sendto(deextracted_data, (udp_ip, udp_port))
            print(f"Sent UDP message to {udp_ip}:{udp_port}")
            
def main():
    # Create and start threads
    udp_receiver_thread = threading.Thread(target=udp_receiver)
    udp_sender_thread = threading.Thread(target=udp_sender)
    serial_receiver_thread = threading.Thread(target=serial_receiver)
    serial_sender_thread = threading.Thread(target=serial_sender)

    udp_receiver_thread.start()
    udp_sender_thread.start()
    serial_receiver_thread.start()
    serial_sender_thread.start()

    udp_receiver_thread.join()
    udp_sender_thread.join()
    serial_receiver_thread.join()
    serial_sender_thread.join()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting program")
    finally:
        ser_1.close()
        ser_2.close()
