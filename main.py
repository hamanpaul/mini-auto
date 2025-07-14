# main.py
import uvicorn
from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
import os
import importlib
import asyncio
import sys

# Import the CameraStreamProcessor
from src.py_rear.services.camera_stream_processor import CameraStreamProcessor
from src.py_rear.apis import camera # Import the camera API module to set its processor

from src.py_rear.apis import camera # Import the camera API module to set its processor

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
    from src.py_rear.apis import camera as apis_camera # Import inside function to ensure it's available
    camera_processor_instance = CameraStreamProcessor()
    # Assign the instance to the apis.camera module
    apis_camera.camera_processor = camera_processor_instance
    # Start the processor in the background
    # It will only start fetching frames when /camera/start is called
    # camera_processor_instance.start() # We will start it via API endpoint

@app.on_event("shutdown")
async def shutdown_event():
    if camera_processor_instance and camera_processor_instance.is_running():
        camera_processor_instance.stop()

# Dynamically import all routes from the apis directory
apis_dir = os.path.join(os.path.dirname(__file__), "src", "py_rear", "apis")
for filename in os.listdir(apis_dir):
    if filename.endswith(".py") and filename != "__init__.py":
        module_name = f"src.py_rear.apis.{filename[:-3]}"
        # Import the module
        module = importlib.import_module(module_name)
        # Register the router
        app.include_router(module.router)

# Mount static files
app.mount("/static", StaticFiles(directory=os.path.join(os.path.dirname(__file__), "templates")), name="static")

# Define the root path to serve index.html
@app.get("/", response_class=HTMLResponse)
async def read_root():
    with open(os.path.join(os.path.dirname(__file__), "templates", "index.html"), "r") as f:
        return f.read()

# Run the server
if __name__ == "__main__":
    # Use uvicorn to run the FastAPI app
    uvicorn.run(app, host="0.0.0.0", port=8000)