import socket
import time
import sys
import subprocess
import json

def get_wifi_ip_powershell():
    try:
        command = """
        Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -like '*Wi-Fi*' -or $_.InterfaceDescription -like '*Wireless*' } | Select-Object IPAddress | ConvertTo-Json
        """
        process = subprocess.run(["powershell.exe", "-Command", command], capture_output=True, text=True, check=True)
        output = process.stdout.strip()
        if output:
            # PowerShell ConvertTo-Json might return a single object or an array of objects
            try:
                data = json.loads(output)
            except json.JSONDecodeError:
                # Handle cases where ConvertTo-Json might output a single object without array brackets
                data = json.loads(f"[{output}]")

            if isinstance(data, list) and data:
                for item in data:
                    if 'IPAddress' in item:
                        # Basic validation for IPv4 format
                        if '.' in item['IPAddress'] and len(item['IPAddress'].split('.')) == 4:
                            return item['IPAddress']
            elif isinstance(data, dict) and 'IPAddress' in data:
                if '.' in data['IPAddress'] and len(data['IPAddress'].split('.')) == 4:
                    return data['IPAddress']
        return None
    except subprocess.CalledProcessError as e:
        print(f"PowerShell command failed: {e}")
        print(f"Stderr: {e.stderr}")
        return None
    except Exception as e:
        print(f"An unexpected error occurred while getting Wi-Fi IP: {e}")
        return None

def get_local_ip():
    """
    Gets the local IP address. Tries to find Wi-Fi IP first using netifaces, then falls back to a general method.
    """
    wifi_ip = get_wifi_ip_powershell()
    if wifi_ip:
        print(f"Detected Wi-Fi IP via PowerShell: {wifi_ip}")
        return wifi_ip
    
    # Fallback method if Wi-Fi IP not found or netifaces failed
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
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
    
    # Try to bind to the specific Wi-Fi IP
    wifi_ip = get_wifi_ip_powershell()
    target_broadcast_address = '255.255.255.255' # Default to general broadcast

    if wifi_ip:
        try:
            sock.bind((wifi_ip, 0)) # Bind to the specific Wi-Fi IP, let OS choose port
            # Attempt to derive broadcast address from Wi-Fi IP and common subnet masks
            # This is a simplification; a more robust solution might involve parsing subnet mask from PowerShell
            ip_parts = list(map(int, wifi_ip.split('.')))
            if ip_parts[0] == 192 and ip_parts[1] == 168: # Common /24 subnet
                target_broadcast_address = f"{ip_parts[0]}.{ip_parts[1]}.{ip_parts[2]}.255"
            elif ip_parts[0] == 10: # Common /8 subnet
                target_broadcast_address = f"{ip_parts[0]}.255.255.255"
            elif ip_parts[0] == 172 and 16 <= ip_parts[1] <= 31: # Common /16 subnet
                target_broadcast_address = f"{ip_parts[0]}.{ip_parts[1]}.255.255"

            print(f"Bound to Wi-Fi IP: {wifi_ip}, attempting to use broadcast address: {target_broadcast_address}")
        except Exception as e:
            print(f"Warning: Could not bind to Wi-Fi IP {wifi_ip}: {e}. Falling back to general broadcast.")
            target_broadcast_address = '255.255.255.255'
    else:
        print("Could not determine specific Wi-Fi IP. Using general broadcast.")

    print(f"Local IP detected: {server_ip}")
    print(f"Starting broadcast of '{message}' to port {port} every {interval} second(s).")
    print("Press Ctrl+C to stop.")

    try:
        while True:
            sock.sendto(message.encode('utf-8'), (target_broadcast_address, port))
            print(f"Sent: {message}")
            time.sleep(interval)
    except KeyboardInterrupt:
        print("Broadcast stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    broadcast_ip()
