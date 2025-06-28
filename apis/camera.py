from fastapi import APIRouter, HTTPException, Response
from fastapi.responses import StreamingResponse
import io
import cv2
import base64

# This will be set by main.py during startup
camera_processor = None

router = APIRouter()

@router.post("/camera/start")
async def start_camera_stream():
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    if camera_processor.is_running():
        return {"message": "Camera stream processor is already running.", "status": "running"}
    
    camera_processor.start()
    return {"message": "Camera stream processor started.", "status": "starting"}

@router.post("/camera/stop")
async def stop_camera_stream():
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    if not camera_processor.is_running():
        return {"message": "Camera stream processor is not running.", "status": "stopped"}
    
    camera_processor.stop()
    return {"message": "Camera stream processor stopped.", "status": "stopping"}

@router.get("/camera/status")
async def get_camera_status():
    if camera_processor is None:
        return {"status": "not_initialized"}
    return {"status": "running" if camera_processor.is_running() else "stopped"}

@router.get("/camera/latest_frame")
async def get_latest_processed_frame():
    if camera_processor is None or not camera_processor.is_running():
        raise HTTPException(status_code=404, detail="Camera stream not running or no frames available.")
    
    raw_frame_bytes, processed_frame_cv = camera_processor.get_latest_frame()
    
    if processed_frame_cv is None:
        raise HTTPException(status_code=404, detail="No processed frame available.")
    
    # Encode the processed frame (OpenCV image) to JPEG bytes
    ret, buffer = cv2.imencode('.jpg', processed_frame_cv)
    if not ret:
        raise HTTPException(status_code=500, detail="Could not encode image.")
    
    # Return as a base64 encoded string for easy JSON transport
    encoded_image = base64.b64encode(buffer.tobytes()).decode('utf-8')
    return {"image": encoded_image}

@router.get("/camera/stream_mjpeg")
async def stream_mjpeg_from_processor():
    if camera_processor is None or not camera_processor.is_running():
        raise HTTPException(status_code=404, detail="Camera stream not running.")

    async def generate_frames():
        while True:
            if not camera_processor.is_running():
                break
            raw_frame_bytes, _ = camera_processor.get_latest_frame()
            if raw_frame_bytes:
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n' +
                       raw_frame_bytes +
                       b'\r\n')
            await asyncio.sleep(0.05) # Adjust frame rate as needed

    return StreamingResponse(generate_frames(), media_type="multipart/x-mixed-replace; boundary=frame")
