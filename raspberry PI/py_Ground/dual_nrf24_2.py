import time
import struct
import socket
import threading
from queue import Queue
from pyrf24 import RF24, RF24_PA_LOW, RF24_DRIVER


# Set radio_1 CE, CSN PIN
CSN_PIN_1 = 0 #SPI0 CE0 -> spidev 0.0
if RF24_DRIVER == 'MRAA':
    CE_PIN_1 = 15
elif RF24_DRIVER == 'wiringPi':
    CE_PIN_1 = 3
else:
    CE_PIN_1 = 22

radio_1 = RF24(CE_PIN_1, CSN_PIN_1)

# Set radio_2 CE, CSN PIN
CSN_PIN_2 = 10 #SPI1 CE0 -> spidev1.0
if RF24_DRIVER == 'MRAA':
    CE_PIN_2 = 32
elif RF24_DRIVER == 'wiringPi':
    CE_PIN_2 = 26
else:
    CE_PIN_2 = 12

radio_2 = RF24(CE_PIN_2, CSN_PIN_2)

if not radio_1.begin():
    raise RuntimeError("radio_1 hardware is not responding")

if not radio_2.begin():
    raise RuntimeError("radio_2 hardware is not responding")

address = [b"1Node", b"2Node"]

# radio_1 Hardware Setting
radio_1.setPALevel(RF24_PA_LOW)
radio_1.openReadingPipe(1, address[1])
radio_1.setChannel(0)  
radio_1.payloadSize = 32  # Set the payload size to the maximum for simplicity
radio_1.startListening()

# radio_2 Hardware Setting
radio_2.setPALevel(RF24_PA_LOW)
radio_2.openWritingPipe(address[0])
radio_2.setChannel(100)  
radio_2.payloadSize = 32

# Queue for communication between threads
udp_to_radio_queue = Queue()
radio_to_udp_queue = Queue()

def radio_1_listener():
    while True:
        if radio_1.available():
            received_payload = radio_1.read(radio_1.payloadSize)
            print(f"Received raw data: {received_payload}")
            radio_to_udp_queue.put(received_payload)

def radio_2_sender():
    while True:
        if not udp_to_radio_queue.empty():
            message = udp_to_radio_queue.get()
            payload = message
            radio_2.write(payload)
            print(f"\nSent raw data: {payload}")

def udp_receiver():
    udp_ip = "127.0.0.1"
    udp_port = 1235
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((udp_ip, udp_port))
    
    print(f"Listing for UDP packets on {udp_ip}:{udp_port}")
    while True:
        data, addr = sock.recvfrom(1024)
        print(f"Received UDP message from {udp_ip}:{udp_port}")
        udp_to_radio_queue.put(data)

#def format_byte_string(byte_string):
#    byte_array = [f'0x{byte:02x}' for byte in byte_string]
#    formatted_lines = [' '.join(byte_array[i:i+8]) for i in range(0, len(byte_array), 8)]
#    return '\n'.join(formatted_lines)

def udp_sender():
    udp_ip = "127.0.0.1"
    udp_port = 1234
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        if not radio_to_udp_queue.empty():
            message = radio_to_udp_queue.get()
            #formatted_message = format_byte_string(message)
            sock.sendto(message, (udp_ip, udp_port))
            print(f"Sento message to {udp_ip}:{udp_port}: {message.hex()}")
            
def main():
    # Threads for radio listening and sending
    listener_thread = threading.Thread(target=radio_1_listener)
    sender_thread = threading.Thread(target=radio_2_sender)
    udp_receiver_thread = threading.Thread(target=udp_receiver)
    udp_sender_thread = threading.Thread(target=udp_sender)

    listener_thread.start()
    sender_thread.start()
    udp_receiver_thread.start()
    udp_sender_thread.start()
    
    listener_thread.join()
    sender_thread.join()
    udp_receiver_thread.join()
    udp_sender_thread.join()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting program")
    finally:
        radio_1.powerDown()
        radio_2.powerDown()
