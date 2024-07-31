import socket
import time
import threading
import serial

ser = serial.Serial(
    port = '/dev/ttyACM0',
    baudrate = 115200,
    )

def udp_receiver():
    udp_ip = "127.0.0.1"
    udp_port = 1234

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((udp_ip, udp_port))

    print(f"Listening on {udp_ip}:{udp_port}")
    while True:
        data, addr = sock.recvfrom(1024)
        try:
            message = data.decode('utf-8')
            print(f"Received message from {addr}: {message}")
        except UnicodeDecodeError:
            print(f"Received non-UTF-8 message from {addr}: {data}")

        # 받은 데이터를 송신을 위해 전역 큐에 추가
        send_queue.append(data)

def udp_sender():
    udp_ip = "127.0.0.1"
    udp_port = 1235

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        if send_queue:
            message = send_queue.pop(0)
            sock.sendto(message, (udp_ip, udp_port))
            print(f"Sent message to {udp_ip}:{udp_port}")

def serial_receiver():
    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline()
                if line:
                    print(f"{line}")
                    send_queue.append(line)
        except Exception as e:
            print(f"Error receiving from Arduino: {e}")
        

if __name__ == "__main__":
    try :
        # 전역 큐 생성
        send_queue = []

        # 수신 및 송신 스레드 시작
        receiver_thread = threading.Thread(target=udp_receiver)
        sender_thread = threading.Thread(target=udp_sender)
        serial_thread = threading.Thread(target=serial_receiver)

        receiver_thread.start()
        sender_thread.start()
        serial_thread.start()

        receiver_thread.join()
        sender_thread.join()
        serial_thraed.join()
    except KeyboardInterrupt:
        print("Exiting Program")
    finally:
        ser.close()
    #Arduino commmunication start
