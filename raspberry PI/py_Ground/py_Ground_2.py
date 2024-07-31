import time
import struct
import socket
import threading
from queue import Queue
from pyrf24 import RF24, RF24_PA_LOW, RF24_DRIVER

print(__file__)

CSN_PIN = 0
if RF24_DRIVER == "MRAA":
    CE_PIN = 15
elif RF24_DRIVER == "wiringPi":
    CE_PIN = 3
else:
    CE_PIN = 22
radio = RF24(CE_PIN, CSN_PIN)

if not radio.begin():
    raise RuntimeError("radio hardware is not responding")

address = [b"1Node", b"2Node"]

radio_number = bool(int(input("which radio is this? Enter '0' or '1'. Default to '0': ") or 0))

radio.setPALevel(RF24_PA_LOW)

radio.openWritingPipe(address[radio_number])
radio.openReadingPipe(1, address[not radio_number])

message_queue = Queue()
radio.payloadSize = 32  # Set the payload size to the maximum for simplicity

def master():
    radio.stopListening()
    while True:
        message = message_queue.get()
        if message is None:  # Exit signal
            break
        buffer = message.ljust(32, b'\0')
        start_timer = time.monotonic_ns()
        result = radio.write(buffer)
        end_timer = time.monotonic_ns()
        if not result:
            print("Transmission failed or Timed out")
        else:
            print("Transmission Successful! Time to Transmit:", f"{(end_timer - start_timer)/1000}us. Sent: {message}")

def udp_receiver():
    udp_ip = "127.0.0.1"
    udp_port = 1235

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((udp_ip, udp_port))

    print(f"Listening for UDP packets on {udp_ip}:{udp_port}")
    while True:
        data, addr = sock.recvfrom(1024)
        print(f"Received UDP message from {addr}")
        message_queue.put(data)

if __name__ == "__main__":
    try:
        # Start the UDP receiver thread
        udp_thread = threading.Thread(target=udp_receiver, daemon=True)
        udp_thread.start()

        # Run the master function in the main thread
        master()
    except KeyboardInterrupt:
        print("Keyboard Interrupt detected. Powering down radio.")
        message_queue.put(None)  # Send exit signal to master function
    finally:
        radio.powerDown()
