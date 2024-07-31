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
message_queue = Queue()

def radio_1_listener():
    buffer = bytearray()
    while True:
        if radio_1.available():
            received_payload = radio_1.read(radio_1.payloadSize)
            buffer.extend(received_payload)
            
            if len(buffer) > 0 and buffer[-1] == 0:
                break
            
            print(f"Received raw data: {received_payload}")
            try:
                message = received_payload.decode('utf-8').strip('\x00')
                print(f"Received message: {message}")
            except UnicodeError:
                print("Received non-UTF-8 message")

def radio_2_sender():
    while True:
        if not message_queue.empty():
            message = message_queue.get()
            payload = message.ljust(32, '\x00').encode('utf-8')
            radio_2.write(payload)
            print(f"\nSent message: {message}")

def user_input():
    while True:
        message = input("Enter a message to Send: ")
        message_queue.put(message)

def main():
    # Threads for radio listening and sending
    listener_thread = threading.Thread(target=radio_1_listener)
    sender_thread = threading.Thread(target=radio_2_sender)
    input_thread = threading.Thread(target=user_input)

    listener_thread.start()
    sender_thread.start()
    input_thread.start()

    listener_thread.join()
    sender_thread.join()
    input_thread.join()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting program")
    finally:
        radio_1.powerDown()
        radio_2.powerDown()
