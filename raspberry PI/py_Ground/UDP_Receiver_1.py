import socket
import time

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
