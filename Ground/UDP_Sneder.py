import socket
import time

def udp_sender():
    udp_ip = "127.0.0.1"
    udp_port = 1234
    message = b"Hello, this is a test message."

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        sock.sendto(message, (udp_ip, udp_port))
        print(f"Sent message to {udp_ip}:{udp_port}")
        time.sleep(1)

if __name__ == "__main__":
    udp_sender()