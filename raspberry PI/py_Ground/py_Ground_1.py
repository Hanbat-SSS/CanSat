import time
import struct
import socket
import threading
from pyrf24 import RF24, RF24_PA_LOW, RF24_DRIVER

print(__file__)

CSN_PIN = 0
if RF24_DRIVER == "MRAA":
    CE_PIN = 15
elif   RF24_DRIVER == "wiringPi":
    CE_PIN = 3
else:
    CE_PIN = 22
radio = RF24(CE_PIN, CSN_PIN)

if not radio.begin():
    raise RuntimeError("radio hardware is not responding")
    
address = [b"1Node", b"2Node"]

radio_number = bool(int(input("which radio is this? Enter 'o' or '1'. sefault to '0'") or 0))

radio.setPALevel(RF24_PA_LOW)

radio.openWritingPipe(address[radio_number])
radio.openReadingPipe(1, address[not radio_number])

message = ""
radio.payloadSize = len(message)

def master():
    radio.stopListening()
    failures = 0
    while True:
        buffer = bytes(message)
        start_timer  = time.monotonic_ns()
        result = radio.write(buffer)
        end_timer = time.monotonic_ns()
        if not result:
            print("Transmission failed ot Timed out")
        else:
            print("Transmission Successful! Time to Transmit:",f"{(end_timer - start_timer)/1000}us. sent: {message}")
            time.sleep(2)
    
def udp_receiver():
	udp_ip = "127.0.0.1"
	udp_port = 1235
	
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind((udp_ip, udp_port))
	
	print("{udp_ip}:{udp_port}")
	while True:
		data, addr = sock.recvfrom(1024)
		try:
			message = data.decode('utf-8')
			print(f"{message}")
		except UnicodeDecodeError:
			print(f"{data}")
			
if __name__ == "__main__":
	udp_receiver()        
            
            
if __name__ == "__main__":
    try:
        master()
    except KeyboardInterrupt:
        print("Keyboard Interrupt detected. powering down radio.")
        radio.powerDown()
else:
    print("   Run master() on the transmitted")


