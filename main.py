# main.py
import uvicorn
from fastapi import FastAPI
import os
import importlib
import asyncio

# Import the CameraStreamProcessor
from services.camera_stream_processor import CameraStreamProcessor
import apis.camera # Import the camera API module to set its processor

# --- Configuration ---
# IMPORTANT: Replace with your ESP32-S3-CAM's actual IP address
# You can find this in the Arduino Serial Monitor output
ESP32_CAM_IP = "YOUR_ESP32_CAM_IP_ADDRESS" # <<<<< CHANGE THIS

# Create a FastAPI app instance
app = FastAPI()

# Global instance of the camera stream processor
camera_processor_instance: CameraStreamProcessor = None

@app.on_event("startup")
async def startup_event():
    global camera_processor_instance
    if ESP32_CAM_IP == "YOUR_ESP32_CAM_IP_ADDRESS":
        print("WARNING: ESP32_CAM_IP is not set in main.py. Camera stream processing will not start.")
        print("Please update ESP32_CAM_IP with your ESP32-S3-CAM's actual IP address.")
    else:
        camera_processor_instance = CameraStreamProcessor(ESP32_CAM_IP)
        # Assign the instance to the apis.camera module
        apis.camera.camera_processor = camera_processor_instance
        # Start the processor in the background
        # It will only start fetching frames when /camera/start is called
        # camera_processor_instance.start() # We will start it via API endpoint

@app.on_event("shutdown")
async def shutdown_event():
    if camera_processor_instance and camera_processor_instance.is_running():
        camera_processor_instance.stop()

# Dynamically import all routes from the apis directory
apis_dir = "apis"
for filename in os.listdir(apis_dir):
    if filename.endswith(".py") and filename != "__init__.py":
        module_name = f"{apis_dir}.{filename[:-3]}"
        # Import the module
        module = importlib.import_module(module_name)
        # Register the router
        app.include_router(module.router)

# Define the root path
@app.get("/")
def read_root():
    # Return a simple welcome message
    return {"message": "歡迎使用 FastAPI 伺服器"}

# Run the server
if __name__ == "__main__":
    # Use uvicorn to run the FastAPI app
    uvicorn.run(app, host="0.0.0.0", port=8000)

