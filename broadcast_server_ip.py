
import socket
import time
import sys

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

def broadcast_ip(port=5005, interval=1):
    server_ip = get_local_ip()
    message = f"MINIAUTO_SERVER_IP:{server_ip}:8000"
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    print(f"Starting broadcast of '{message}' to port {port} every {interval} second(s).")
    print("Press Ctrl+C to stop.")

    try:
        while True:
            sock.sendto(message.encode('utf-8'), ('<broadcast>', port))
            print(f"Sent: {message}")
            time.sleep(interval)
    except KeyboardInterrupt:
        print("Broadcast stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    broadcast_ip()
