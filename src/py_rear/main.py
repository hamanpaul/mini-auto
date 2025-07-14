# main.py
import uvicorn
from fastapi import FastAPI
import os
import importlib
import asyncio
import sys
import socket # Import socket for UDP broadcast
import netifaces # To get local IP address

# Event to signal the UDP broadcast to stop
stop_broadcast_event = asyncio.Event()

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

# Import the CameraStreamProcessor
from src.py_rear.services.camera_stream_processor import CameraStreamProcessor
import src.py_rear.apis.camera # Import the camera API module to set its processor
import src.py_rear.apis.discovery # Import the discovery API module

# --- Configuration ---
# IMPORTANT: Replace with your ESP32-S3-CAM's actual IP address
# You can find this in the Arduino Serial Monitor output
ESP32_CAM_IP = "YOUR_ESP32_CAM_IP_ADDRESS" # <<<<< CHANGE THIS

# UDP Broadcast Configuration
UDP_BROADCAST_PORT = 5005
BROADCAST_INTERVAL = 5 # seconds

# Create a FastAPI app instance
app = FastAPI()

# Global instance of the camera stream processor
camera_processor_instance: CameraStreamProcessor = None

# Function to get the local IP address
def get_local_ip():
    try:
        # Get all network interfaces
        for iface in netifaces.interfaces():
            # Get addresses for the interface
            addresses = netifaces.ifaddresses(iface)
            # Check for IPv4 addresses
            if netifaces.AF_INET in addresses:
                for link in addresses[netifaces.AF_INET]:
                    # Exclude loopback and Docker interfaces
                    if 'addr' in link and link['addr'] != '127.0.0.1':
                        return link['addr']
    except Exception as e:
        print(f"Error getting local IP: {e}")
    return None

# Asynchronous function to send UDP broadcast
async def send_udp_broadcast():
    local_ip = get_local_ip()
    if not local_ip:
        print("Could not determine local IP address for UDP broadcast.")
        return

    message = f"MINIAUTO_SERVER_IP:{local_ip}:8000" # Include server IP and port
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(0.1) # Non-blocking send

    print(f"Starting UDP broadcast of server IP: {local_ip}:{8000} on port {UDP_BROADCAST_PORT}")

    while not stop_broadcast_event.is_set():
        try:
            sock.sendto(message.encode(), ('<broadcast>', UDP_BROADCAST_PORT))
            # print(f"Sent UDP broadcast: {message}")
        except Exception as e:
            print(f"Error sending UDP broadcast: {e}")
        await asyncio.sleep(BROADCAST_INTERVAL)
    print("UDP broadcast stopped.")

@app.on_event("startup")
async def startup_event():
    global camera_processor_instance
    camera_processor_instance = CameraStreamProcessor()
    # Assign the instance to the apis.camera module
    src.py_rear.apis.camera.camera_processor = camera_processor_instance
    # Start the processor in the background
    # It will only start fetching frames when /camera/start is called
    # camera_processor_instance.start() # We will start it via API endpoint

    # Start UDP broadcast in the background
    asyncio.create_task(send_udp_broadcast())

@app.on_event("shutdown")
async def shutdown_event():
    if camera_processor_instance and camera_processor_instance.is_running():
        camera_processor_instance.stop()

# Dynamically import all routes from the apis directory
apis_dir = os.path.join(os.path.dirname(__file__), "apis")
for filename in os.listdir(apis_dir):
    if filename.endswith(".py") and filename != "__init__.py":
        module_name = f"src.py_rear.apis.{filename[:-3]}"
        # Import the module
        module = importlib.import_module(module_name)
        # Register the router
        app.include_router(module.router)

# Register the discovery router explicitly
app.include_router(src.py_rear.apis.discovery.router)

@app.post("/api/arduino_connected")
async def arduino_connected_signal():
    stop_broadcast_event.set()
    print("Received Arduino connected signal. Stopping UDP broadcast.")
    return {"message": "UDP broadcast will be stopped."}

# Define the root path
@app.get("/")
def read_root():
    # Return a simple welcome message
    return {"message": "歡迎使用 FastAPI 伺服器"}

# Run the server
if __name__ == "__main__":
    # Use uvicorn to run the FastAPI app
    uvicorn.run(app, host="0.0.0.0", port=8000)

