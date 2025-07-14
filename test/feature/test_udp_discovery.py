
import socket
import time

UDP_BROADCAST_PORT = 5005
BUFFER_SIZE = 1024

def test_udp_discovery():
    """
    Listens for the UDP broadcast from the server and verifies its content.
    """
    # Create a UDP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Allow reusing the address, which is useful for quick restarts
    client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Bind the socket to listen on all interfaces for the broadcast port
    client_socket.bind(("", UDP_BROADCAST_PORT))
    
    print(f"Listening for UDP broadcast on port {UDP_BROADCAST_PORT}...")
    
    # Set a timeout so the test doesn't run forever
    client_socket.settimeout(10) # 10-second timeout
    
    try:
        # Receive data
        data, addr = client_socket.recvfrom(BUFFER_SIZE)
        
        message = data.decode()
        print(f"Received broadcast from {addr}: {message}")
        
        # --- Verification ---
        assert message.startswith("MINIAUTO_SERVER_IP:"), "Message should start with MINIAUTO_SERVER_IP:"
        
        parts = message.split(':')
        assert len(parts) == 3, "Message format should be MINIAUTO_SERVER_IP:IP:PORT"
        
        ip_address = parts[1]
        port = parts[2]
        
        # Validate IP address format (simple check)
        ip_parts = ip_address.split('.')
        assert len(ip_parts) == 4 and all(p.isdigit() for p in ip_parts), "Invalid IP address format"
        
        # Validate port format
        assert port.isdigit(), "Port should be a number"
        
        print("UDP Discovery Test PASSED!")
        
    except socket.timeout:
        print("UDP Discovery Test FAILED: Did not receive broadcast within 10 seconds.")
        assert False, "Test timed out"
        
    finally:
        client_socket.close()
        print("Socket closed.")

if __name__ == "__main__":
    test_udp_discovery()
