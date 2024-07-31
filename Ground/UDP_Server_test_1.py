import socket

def main():
    recv_host = "127.0.0.1"  # 수신할 IP 주소
    recv_port = 1236         # 수신할 포트 번호

    # UDP 소켓 생성
    recv_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # 소켓 바인딩 (IP 주소와 포트 번호 설정)
    recv_socket.bind((recv_host, recv_port))

    print(f"Listening on {recv_host}:{recv_port}")

    while True:
        # 데이터 수신 (버퍼 크기: 1024 바이트)
        data, addr = recv_socket.recvfrom(1024)
        print(f"Received message: {data.decode('utf-8')} from {addr}")

if __name__ == "__main__":
    main()
